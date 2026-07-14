/**
 * cam678_emulator.cpp
 * Receives I2C packets sent by cam678_example.py and prints them.
 * Same structure as i2c_logging_emulator.cpp — already proven to compile.
 *
 * Compile:
 *   g++ -std=c++17 -O2 \
 *       -I src/ \
 *       -I src/hololink/emulation/src \
 *       -I src/hololink/emulation/src/linux \
 *       -I /opt/nvidia/holoscan/include/3rdparty \
 *       cam678_emulator.cpp \
 *       build/src/hololink/emulation/libemulationcoe.a \
 *       build/src/hololink/emulation/libemulationroce.a \
 *       build/src/hololink/operators/emulator/liblinux_data_plane.a \
 *       build/src/hololink/common/libhololink.a \
 *       build/src/hololink/core/libhololink_core.a \
 *       build/src/hololink/emulation/sensors/libemulation_sensors.a \
 *       build/src/hololink/emulation/libemulation.a \
 *       -L /usr/local/cuda/lib64 -lcudart -lz -lpthread \
 *       -o cam678_emulator
 *
 * Run:
 *   ./cam678_emulator
 */

#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <chrono>
#include <thread>
#include <vector>

#include "hololink/emulation/hsb_emulator.hpp"
#include "hololink/emulation/i2c_interface.hpp"
#include "hololink/emulation/linux_data_plane.hpp"
#include "hololink/emulation/net.hpp"

using namespace hololink::emulation;

static constexpr uint8_t  CAM678_I2C_ADDRESS = 0x10;
static constexpr uint32_t I2C_CTRL_ADDR      = 0x03000200;
static constexpr uint8_t  CAM_I2C_BUS        = 1;
static constexpr char     EMULATOR_IP[]      = "192.168.0.2";
static constexpr uint8_t  DATA_PLANE_ID      = 0;
static constexpr uint8_t  SENSOR_ID          = 0;

static volatile bool g_running = true;
static void sig_handler(int) { g_running = false; }

// ── Cam678I2CPeripheral ───────────────────────────────────────────────────────

class Cam678I2CPeripheral : public I2CPeripheral {
public:
    explicit Cam678I2CPeripheral(uint8_t dev_addr)
        : dev_addr_(dev_addr), count_(0)
    {
        memset(regs_, 0x00, sizeof(regs_));
        // Pre-load chip ID so get_version() returns 0x0219
        regs_[0x0000] = 0x02;
        regs_[0x0001] = 0x19;
        printf("[cam678_emulator] Ready  dev=0x%02X\n", dev_addr_);
    }

    void attach_to_i2c(I2CController& ctrl, uint8_t bus) override
    {
        printf("[cam678_emulator] Attaching  bus=%u  dev=0x%02X\n", bus, dev_addr_);
        ctrl.attach_i2c_peripheral(bus, dev_addr_, this);
    }

    // 2.5.0 vector-based signature
    I2CStatus i2c_transaction(
        uint8_t                     peripheral_address,
        const std::vector<uint8_t>& write_bytes,
        std::vector<uint8_t>&       read_bytes) override
    {
        if (peripheral_address != dev_addr_)
            return I2CStatus::I2C_STATUS_INVALID_PERIPHERAL_ADDRESS;
        if (write_bytes.size() < 2)
            return I2CStatus::I2C_STATUS_INVALID_REGISTER_ADDRESS;

        uint16_t reg = (static_cast<uint16_t>(write_bytes[0]) << 8)
                     |  static_cast<uint16_t>(write_bytes[1]);
        ++count_;

        if (write_bytes.size() > 2) {
            // ── WRITE ─────────────────────────────────────────────────────────
            uint16_t data_len = write_bytes.size() - 2;
            printf("[CAM678 WRITE #%u]  reg=0x%04X  len=%u  bytes:",
                   count_, reg, data_len);
            for (size_t i = 2; i < write_bytes.size(); ++i) {
                printf(" 0x%02X", write_bytes[i]);
                if (reg + (i - 2) < sizeof(regs_))
                    regs_[reg + (i - 2)] = write_bytes[i];
            }
            printf("\n");
            annotate_write(reg, write_bytes.data() + 2, data_len);
        } else {
            // ── READ ──────────────────────────────────────────────────────────
            size_t n = read_bytes.size();
            printf("[CAM678 READ  #%u]  reg=0x%04X  len=%zu  returned:",
                   count_, reg, n);
            for (size_t i = 0; i < n; ++i) {
                read_bytes[i] = (reg + i < sizeof(regs_)) ? regs_[reg + i] : 0x00;
                printf(" 0x%02X", read_bytes[i]);
            }
            printf("\n");
            annotate_read(reg, n);
        }
        return I2CStatus::I2C_STATUS_SUCCESS;
    }

    void start() override { printf("[cam678_emulator] start()\n"); }
    void stop()  override {
        printf("[cam678_emulator] stop()  total_transactions=%u\n", count_);
    }

private:
    void annotate_write(uint16_t reg, const uint8_t* data, uint16_t /*len*/)
    {
        switch (reg) {
        // ── Streaming control ─────────────────────────────────────────────────
        case 0x0100:
            printf("           -> MODE_SELECT: %s\n",
                   data[0] ? "STREAM ON  ← cam678 start()" : "STANDBY  ← cam678 stop() / configure()");
            break;
        // ── Global config (access codes) ──────────────────────────────────────
        case 0x30EB: printf("           -> access code byte\n"); break;
        case 0x300A: printf("           -> access code 0xFF\n"); break;
        // ── CSI config ───────────────────────────────────────────────────────
        case 0x0114: printf("           -> CSI_LANE_MODE: %d lanes  ← configure()\n", (data[0] & 3) + 1); break;
        // ── Frame timing ─────────────────────────────────────────────────────
        case 0x0160: printf("           -> FRAME_LENGTH_LINES MSB  ← configure()\n"); break;
        case 0x0161: printf("           -> FRAME_LENGTH_LINES LSB\n"); break;
        // ── Output size ───────────────────────────────────────────────────────
        case 0x016C: printf("           -> X_OUTPUT_SIZE MSB  ← configure()\n"); break;
        case 0x016D: printf("           -> X_OUTPUT_SIZE LSB\n"); break;
        case 0x016E: printf("           -> Y_OUTPUT_SIZE MSB  ← configure()\n"); break;
        case 0x016F: printf("           -> Y_OUTPUT_SIZE LSB\n"); break;
        // ── Data format ───────────────────────────────────────────────────────
        case 0x018C: printf("           -> CSI_DATA_FORMAT MSB (0x0A=RAW10)  ← configure()\n"); break;
        case 0x018D: printf("           -> CSI_DATA_FORMAT LSB\n"); break;
        // ── Gain ─────────────────────────────────────────────────────────────
        case 0x0157: printf("           -> ANALOG_GAIN  ← set_gain()\n"); break;
        case 0x0158: printf("           -> DIG_GAIN_GLOBAL MSB  ← set_gain()\n"); break;
        case 0x0159: printf("           -> DIG_GAIN_GLOBAL LSB\n"); break;
        // ── Exposure ─────────────────────────────────────────────────────────
        case 0x015A: printf("           -> COARSE_INT_TIME MSB  ← set_exposure()\n"); break;
        case 0x015B: printf("           -> COARSE_INT_TIME LSB\n"); break;
        default: break;
        }
    }

    void annotate_read(uint16_t reg, size_t /*n*/)
    {
        switch (reg) {
        case 0x0000: printf("           -> Chip ID read  ← get_version()\n"); break;
        case 0x0001: printf("           -> Chip ID LSB   ← get_version()\n"); break;
        default: break;
        }
    }

    uint8_t  dev_addr_;
    uint32_t count_;
    uint8_t  regs_[0x10000];
};

// ── main ─────────────────────────────────────────────────────────────────────

int main()
{
    signal(SIGINT,  sig_handler);
    signal(SIGTERM, sig_handler);

    printf("============================================\n");
    printf("  cam678 I2C Logging Emulator\n");
    printf("  Emulator IP : %s\n", EMULATOR_IP);
    printf("  cam678 addr : 0x%02X\n", CAM678_I2C_ADDRESS);
    printf("  I2C bus     : %u\n", CAM_I2C_BUS);
    printf("============================================\n\n");

    HSBEmulator   hsb;
    LinuxDataPlane dp(hsb,
                      IPAddress_from_string(EMULATOR_IP),
                      DATA_PLANE_ID,
                      SENSOR_ID);

    Cam678I2CPeripheral periph(CAM678_I2C_ADDRESS);
    periph.attach_to_i2c(hsb.get_i2c(I2C_CTRL_ADDR), CAM_I2C_BUS);

    hsb.start();

    printf("[cam678_emulator] Running — waiting for cam678_example.py\n");
    printf("[cam678_emulator] Ctrl-C to stop.\n\n");

    while (g_running)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    printf("\n[cam678_emulator] Stopping...\n");
    hsb.stop();
    return 0;
}
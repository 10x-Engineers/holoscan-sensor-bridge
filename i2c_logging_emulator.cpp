/**
 * i2c_logging_emulator.cpp  —  Terminal 1, OUTSIDE Docker
 *
 * Runs the HSB Emulator with a LoggingI2CPeripheral.
 * Every I2C write and read from the host is printed with full register
 * address, byte values, and annotations for known IMX219 registers.
 *
 * Build (from repo root):
 *   g++ -std=c++17 -O2 \
 *       -I src/hololink/emulation \
 *       -I src/hololink/emulation/src \
 *       i2c_logging_emulator.cpp \
 *       build/src/linux/libemulation_host.a \
 *       build/src/linux/libemulationroce.a \
 *       build/src/linux/libemulationcoe.a \
 *       build/src/common/libemulation_common.a \
 *       -lpthread -o i2c_logging_emulator
 *
 * Run:
 *   sudo ./i2c_logging_emulator
 */

#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <chrono>
#include <thread>

#include "hololink/emulation/hsb_emulator.hpp"
#include "hololink/emulation/i2c_interface.hpp"
#include "hololink/emulation/linux_data_plane.hpp"
#include "hololink/emulation/net.hpp"

using namespace hololink::emulation;

// ── Constants (match hololink Python values) ──────────────────────────────────
static constexpr uint8_t  IMX219_I2C_ADDRESS = 0x10;
static constexpr uint32_t I2C_CTRL_ADDR      = 0x03000200;  // hololink.I2C_CTRL
static constexpr uint8_t  CAM_I2C_BUS        = 1;           // hololink.CAM_I2C_BUS
static constexpr char     EMULATOR_IP[]      = "192.168.0.2";
static constexpr uint8_t  DATA_PLANE_ID      = 0;
static constexpr uint8_t  SENSOR_ID          = 0;

static volatile bool g_running = true;
static void sig_handler(int) { g_running = false; }

// ── LoggingI2CPeripheral ──────────────────────────────────────────────────────
class LoggingI2CPeripheral : public I2CPeripheral {
public:
    explicit LoggingI2CPeripheral(uint8_t dev_addr)
        : dev_addr_(dev_addr), count_(0)
    {
        memset(regs_, 0x00, sizeof(regs_));
        regs_[0x0000] = 0x02;
        regs_[0x0001] = 0x19;
        printf("[emulator] LoggingI2CPeripheral ready  dev=0x%02X\n", dev_addr_);
    }

    void attach_to_i2c(I2CController& ctrl, uint8_t bus) override
    {
        printf("[emulator] Attaching to I2C controller  bus=%u  dev=0x%02X\n",
               bus, dev_addr_);
        ctrl.attach_i2c_peripheral(bus, dev_addr_, this);
    }

    // ── 2.5.0 correct signature ───────────────────────────────────────────────
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
            // WRITE transaction
            uint16_t data_len = write_bytes.size() - 2;
            printf("[I2C WRITE #%u]  dev=0x%02X  reg=0x%04X  len=%u  bytes:",
                   count_, peripheral_address, reg, data_len);
            for (size_t i = 2; i < write_bytes.size(); ++i) {
                printf(" 0x%02X", write_bytes[i]);
                if (reg + (i-2) < sizeof(regs_))
                    regs_[reg + (i-2)] = write_bytes[i];
            }
            printf("\n");
            annotate(reg, write_bytes.data() + 2, data_len);
        } else {
            // READ transaction — fill read_bytes from register map
            // read_bytes.size() tells us how many bytes the host wants
            size_t n = read_bytes.size();
            printf("[I2C READ  #%u]  dev=0x%02X  reg=0x%04X  len=%zu  returned:",
                   count_, peripheral_address, reg, n);
            for (size_t i = 0; i < n; ++i) {
                read_bytes[i] = (reg + i < sizeof(regs_)) ? regs_[reg + i] : 0x00;
                printf(" 0x%02X", read_bytes[i]);
            }
            printf("\n");
        }
        return I2CStatus::I2C_STATUS_SUCCESS;
    }

    void start() override { printf("[emulator] I2CPeripheral::start()\n"); }
    void stop()  override { printf("[emulator] I2CPeripheral::stop()  "
                                   "total_transactions=%u\n", count_); }

private:
    void annotate(uint16_t reg, const uint8_t* data, uint16_t len)
    {
        if (!len) return;
        switch (reg) {
        case 0x0100: printf("           -> MODE_SELECT: %s\n",
                            data[0] ? "STREAM ON" : "STANDBY"); break;
        case 0x0114: printf("           -> CSI_LANE_MODE: %d lanes\n", (data[0]&3)+1); break;
        case 0x0160: printf("           -> FRAME_LENGTH_LINES MSB\n"); break;
        case 0x0161: printf("           -> FRAME_LENGTH_LINES LSB\n"); break;
        case 0x016C: printf("           -> X_OUTPUT_SIZE MSB\n"); break;
        case 0x016D: printf("           -> X_OUTPUT_SIZE LSB\n"); break;
        case 0x016E: printf("           -> Y_OUTPUT_SIZE MSB\n"); break;
        case 0x016F: printf("           -> Y_OUTPUT_SIZE LSB\n"); break;
        case 0x018C: printf("           -> CSI_DATA_FORMAT MSB (0x0A=RAW10)\n"); break;
        default: break;
        }
    }

    uint8_t  dev_addr_;
    uint32_t count_;
    uint8_t  regs_[0x10000];
};;

// ── main ─────────────────────────────────────────────────────────────────────
int main()
{
    signal(SIGINT,  sig_handler);
    signal(SIGTERM, sig_handler);

    printf("============================================\n");
    printf("  HSB I2C Logging Emulator\n");
    printf("  Emulator IP : %s\n", EMULATOR_IP);
    printf("  IMX219 addr : 0x%02X\n", IMX219_I2C_ADDRESS);
    printf("  I2C bus     : %u  (CAM_I2C_BUS)\n", CAM_I2C_BUS);
    printf("============================================\n\n");

    HSBEmulator  hsb;
    LinuxDataPlane dp(hsb,
                      IPAddress_from_string(EMULATOR_IP),
                      DATA_PLANE_ID,
                      SENSOR_ID);

    LoggingI2CPeripheral periph(IMX219_I2C_ADDRESS);
    periph.attach_to_i2c(hsb.get_i2c(I2C_CTRL_ADDR), CAM_I2C_BUS);

    hsb.start();

    printf("[emulator] Running — BOOTP broadcasting on %s\n", EMULATOR_IP);
    printf("[emulator] Start i2c_host in Docker Terminal 2.\n");
    printf("[emulator] Ctrl-C to stop.\n\n");

    while (g_running)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    printf("\n[emulator] Stopping...\n");
    hsb.stop();
    return 0;
}

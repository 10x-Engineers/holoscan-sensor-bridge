#include "native_cam678_sensor.hpp"
#include <cstdio>
#include <vector>

namespace hololink::sensors {

NativeCam678Sensor::NativeCam678Sensor(std::shared_ptr<Hololink> hololink,
                                        uint32_t i2c_bus)
    : hololink_(hololink)
    , i2c_(hololink->get_i2c(i2c_bus))
{
    printf("[cam678] Hello from Cam678 constructor  "
           "driver=%s  version=%u  i2c_addr=0x%02X\n",
           DRIVER_NAME, VERSION, I2C_ADDRESS);
}

NativeCam678Sensor::~NativeCam678Sensor()
{
    printf("[cam678] Cam678 destructor called\n");
}

void NativeCam678Sensor::configure(CameraMode mode)
{
    printf("[cam678] Hello from configure  mode=%d\n", static_cast<int>(mode));
    set_register(0x0100, 0x00);
    set_register(0x30EB, 0x05);
    set_register(0x30EB, 0x0C);
    set_register(0x300A, 0xFF);
    set_register(0x0114, 0x01);
    set_register(0x0160, 0x04);
    set_register(0x0161, 0x80);
    set_register(0x016C, 0x05);
    set_register(0x016D, 0x00);
    set_register(0x016E, 0x02);
    set_register(0x016F, 0xD0);
    set_register(0x018C, 0x0A);
    set_register(0x018D, 0x0A);
    printf("[cam678] configure done\n");
}

void NativeCam678Sensor::start()
{
    printf("[cam678] Hello from start — sending stream ON\n");
    set_register(0x0100, 0x01);
}

void NativeCam678Sensor::stop()
{
    printf("[cam678] Hello from stop — sending stream OFF\n");
    set_register(0x0100, 0x00);
}

uint32_t NativeCam678Sensor::get_version() const
{
    printf("[cam678] Hello from get_version\n");
    uint8_t hi = get_register(0x0000);
    uint8_t lo = get_register(0x0001);
    uint32_t version = (static_cast<uint32_t>(hi) << 8) | lo;
    printf("[cam678] get_version -> 0x%04X\n", version);
    return version;
}

void NativeCam678Sensor::set_gain(int32_t value)
{
    printf("[cam678] Hello from set_gain  value=%d\n", value);
    set_register(0x0157, static_cast<uint8_t>(value & 0xFF));
    set_register(0x0158, static_cast<uint8_t>((value >> 8) & 0xFF));
    set_register(0x0159, static_cast<uint8_t>(value & 0xFF));
    printf("[cam678] set_gain done\n");
}

void NativeCam678Sensor::set_exposure(int32_t value)
{
    printf("[cam678] Hello from set_exposure  value=%d\n", value);
    set_register(0x015A, static_cast<uint8_t>((value >> 8) & 0xFF));
    set_register(0x015B, static_cast<uint8_t>(value & 0xFF));
    printf("[cam678] set_exposure done\n");
}

uint8_t NativeCam678Sensor::get_register(uint16_t reg) const
{
    std::vector<uint8_t> write_buf = {
        static_cast<uint8_t>(reg >> 8),
        static_cast<uint8_t>(reg & 0xFF)
    };
    auto reply = i2c_->i2c_transaction(I2C_ADDRESS, write_buf, 1);
    return reply.empty() ? 0x00 : reply[0];
}

void NativeCam678Sensor::set_register(uint16_t reg, uint8_t value)
{
    std::vector<uint8_t> write_buf = {
        static_cast<uint8_t>(reg >> 8),
        static_cast<uint8_t>(reg & 0xFF),
        value
    };
    i2c_->i2c_transaction(I2C_ADDRESS, write_buf, 0);
}

} // namespace hololink::sensors
#include "native_cam380_sensor.hpp"
#include <cstdio>
#include <vector>

namespace hololink::sensors {

NativeCam380Sensor::NativeCam380Sensor(std::shared_ptr<Hololink> hololink,
                                        uint32_t i2c_bus)
    : hololink_(hololink)
    , i2c_(hololink->get_i2c(i2c_bus))
{
    printf("[cam380] Hello from constructor  driver=%s  version=%u  i2c_addr=0x%02X\n",
           DRIVER_NAME, VERSION, I2C_ADDRESS);
}

NativeCam380Sensor::~NativeCam380Sensor()
{
    printf("[cam380] Destructor called\n");
}

void NativeCam380Sensor::configure(CameraMode mode)
{
    printf("[cam380] Hello from configure  mode=%d\n", static_cast<int>(mode));
    set_register(0x0100, 0x00);
    set_register(0x30EB, 0x05); set_register(0x30EB, 0x0C);
    set_register(0x300A, 0xFF); set_register(0x300B, 0xFF);
    set_register(0x30EB, 0x05); set_register(0x30EB, 0x09);
    set_register(0x0114, 0x01);
    set_register(0x0128, 0x00);
    set_register(0x012A, 0x18); set_register(0x012B, 0x00);
    set_register(0x0160, 0x04); set_register(0x0161, 0x80);
    set_register(0x0162, 0x0D); set_register(0x0163, 0x78);
    set_register(0x016C, 0x07); set_register(0x016D, 0x80);
    set_register(0x016E, 0x04); set_register(0x016F, 0x38);
    set_register(0x0170, 0x01); set_register(0x0171, 0x01);
    set_register(0x0174, 0x00); set_register(0x0175, 0x00);
    set_register(0x018C, 0x0A); set_register(0x018D, 0x0A);
    printf("[cam380] configure done\n");
}

void NativeCam380Sensor::start()
{
    printf("[cam380] Hello from start — stream ON\n");
    set_register(0x0100, 0x01);
}

void NativeCam380Sensor::stop()
{
    printf("[cam380] Hello from stop — stream OFF\n");
    set_register(0x0100, 0x00);
}

uint32_t NativeCam380Sensor::get_version() const
{
    printf("[cam380] Hello from get_version\n");
    uint8_t hi = get_register(0x0000);
    uint8_t lo = get_register(0x0001);
    uint32_t v = (static_cast<uint32_t>(hi) << 8) | lo;
    printf("[cam380] get_version -> 0x%04X\n", v);
    return v;
}

void NativeCam380Sensor::set_analog_gain(int32_t value)
{
    printf("[cam380] Hello from set_analog_gain  value=%d\n", value);
    set_register(0x0157, static_cast<uint8_t>(value & 0xFF));
    printf("[cam380] set_analog_gain done\n");
}

void NativeCam380Sensor::set_digital_gain(int32_t value)
{
    printf("[cam380] Hello from set_digital_gain  value=%d\n", value);
    set_register(0x0158, static_cast<uint8_t>((value >> 8) & 0xFF));
    set_register(0x0159, static_cast<uint8_t>(value & 0xFF));
    printf("[cam380] set_digital_gain done\n");
}

void NativeCam380Sensor::set_exposure(int32_t value)
{
    printf("[cam380] Hello from set_exposure  value=%d\n", value);
    set_register(0x015A, static_cast<uint8_t>((value >> 8) & 0xFF));
    set_register(0x015B, static_cast<uint8_t>(value & 0xFF));
    printf("[cam380] set_exposure done\n");
}

void NativeCam380Sensor::print_status() const
{
    printf("[cam380] Hello from print_status\n");
    uint8_t mode    = get_register(0x0100);
    uint8_t gain    = get_register(0x0157);
    uint8_t exp_msb = get_register(0x015A);
    uint8_t exp_lsb = get_register(0x015B);
    printf("[cam380] mode=0x%02X  analog_gain=0x%02X  exposure=0x%04X\n",
           mode, gain, (exp_msb << 8) | exp_lsb);
}

uint8_t NativeCam380Sensor::get_register(uint16_t reg) const
{
    std::vector<uint8_t> wb = {
        static_cast<uint8_t>(reg >> 8),
        static_cast<uint8_t>(reg & 0xFF)
    };
    auto reply = i2c_->i2c_transaction(I2C_ADDRESS, wb, 1);
    return reply.empty() ? 0x00 : reply[0];
}

void NativeCam380Sensor::set_register(uint16_t reg, uint8_t value)
{
    std::vector<uint8_t> wb = {
        static_cast<uint8_t>(reg >> 8),
        static_cast<uint8_t>(reg & 0xFF),
        value
    };
    i2c_->i2c_transaction(I2C_ADDRESS, wb, 0);
}

} // namespace hololink::sensors
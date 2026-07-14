#pragma once

#include <hololink/core/hololink.hpp>
#include <hololink/sensors/camera/camera_sensor.hpp>
#include <memory>
#include <cstdint>

namespace hololink::sensors {

class NativeCam380Sensor : public CameraSensor {
public:
    static constexpr const char* DRIVER_NAME = "CAM380-NATIVE";
    static constexpr uint32_t    VERSION     = 1;
    static constexpr uint32_t    I2C_ADDRESS = 0x10;

    NativeCam380Sensor(std::shared_ptr<Hololink> hololink,
                       uint32_t i2c_bus = CAM_I2C_BUS);
    ~NativeCam380Sensor() override;

    void configure(CameraMode mode) override;
    void start() override;
    void stop() override;

    uint32_t get_version() const;
    void     set_analog_gain(int32_t value);
    void     set_digital_gain(int32_t value);
    void     set_exposure(int32_t value);
    void     print_status() const;

private:
    uint8_t get_register(uint16_t reg) const;
    void    set_register(uint16_t reg, uint8_t value);

    std::shared_ptr<Hololink>      hololink_;
    std::shared_ptr<Hololink::I2c> i2c_;
};

} // namespace hololink::sensors
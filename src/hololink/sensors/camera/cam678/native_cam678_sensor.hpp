#ifndef SENSORS_CAMERA_CAM678_NATIVE_CAM678_SENSOR_HPP
#define SENSORS_CAMERA_CAM678_NATIVE_CAM678_SENSOR_HPP

#include <hololink/core/hololink.hpp>
#include <hololink/sensors/camera/camera_sensor.hpp>
#include <memory>

namespace hololink::sensors {

class NativeCam678Sensor : public CameraSensor {
public:
    static constexpr const char* DRIVER_NAME = "CAM678-NATIVE";
    static constexpr uint32_t    VERSION     = 1;
    static constexpr uint32_t    I2C_ADDRESS = 0x10;

    // Takes Hololink directly — no DataChannel, no packetizer_program.hpp
    NativeCam678Sensor(std::shared_ptr<Hololink> hololink,
                       uint32_t i2c_bus = CAM_I2C_BUS);
    ~NativeCam678Sensor() override;

    void configure(CameraMode mode) override;
    void start() override;
    void stop() override;

    uint32_t get_version() const;
    void set_gain(int32_t value);
    void set_exposure(int32_t value);

protected:
    uint8_t get_register(uint16_t reg) const;
    void    set_register(uint16_t reg, uint8_t value);

private:
    std::shared_ptr<Hololink>      hololink_;
    std::shared_ptr<Hololink::I2c> i2c_;
};

} // namespace hololink::sensors
#endif
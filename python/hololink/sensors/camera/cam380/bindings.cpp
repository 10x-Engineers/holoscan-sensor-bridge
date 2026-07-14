#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <hololink/sensors/camera/cam380/native_cam380_sensor.hpp>

namespace py = pybind11;

namespace hololink::sensors {

PYBIND11_MODULE(_native_cam380_camera_sensor, m)
{
    py::class_<NativeCam380Sensor, CameraSensor,
               std::shared_ptr<NativeCam380Sensor>>(m, "NativeCam380Sensor")
        .def(py::init<std::shared_ptr<Hololink>, uint32_t>(),
            py::arg("hololink"),
            py::arg("i2c_bus") = CAM_I2C_BUS)
        .def("configure",       py::overload_cast<CameraMode>(&NativeCam380Sensor::configure))
        .def("start",           &NativeCam380Sensor::start)
        .def("stop",            &NativeCam380Sensor::stop)
        .def("get_version",     &NativeCam380Sensor::get_version)
        .def("set_analog_gain", &NativeCam380Sensor::set_analog_gain, py::arg("value") = 0)
        .def("set_digital_gain",&NativeCam380Sensor::set_digital_gain,py::arg("value") = 0)
        .def("set_exposure",    &NativeCam380Sensor::set_exposure,    py::arg("value") = 0x0C)
        .def("print_status",    &NativeCam380Sensor::print_status)
        .def_property_readonly_static("DRIVER_NAME",
            [](py::object) { return NativeCam380Sensor::DRIVER_NAME; })
        .def_property_readonly_static("VERSION",
            [](py::object) { return NativeCam380Sensor::VERSION; })
        .def_property_readonly_static("I2C_ADDRESS",
            [](py::object) { return NativeCam380Sensor::I2C_ADDRESS; });
}

} // namespace hololink::sensors
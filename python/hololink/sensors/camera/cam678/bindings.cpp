#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <hololink/sensors/camera/cam678/native_cam678_sensor.hpp>

namespace py = pybind11;

namespace hololink::sensors {

PYBIND11_MODULE(_native_cam678_camera_sensor, m)
{
    py::class_<NativeCam678Sensor, CameraSensor,
               std::shared_ptr<NativeCam678Sensor>>(m, "NativeCam678Sensor")
        .def(py::init<std::shared_ptr<Hololink>, uint32_t>(),
            py::arg("hololink"),
            py::arg("i2c_bus") = CAM_I2C_BUS)
        .def("configure",    py::overload_cast<CameraMode>(&NativeCam678Sensor::configure))
        .def("start",        &NativeCam678Sensor::start)
        .def("stop",         &NativeCam678Sensor::stop)
        .def("get_version",  &NativeCam678Sensor::get_version)
        .def("set_gain",     &NativeCam678Sensor::set_gain,     py::arg("value") = 0)
        .def("set_exposure", &NativeCam678Sensor::set_exposure, py::arg("value") = 0x0C)
        .def_property_readonly_static("DRIVER_NAME",
            [](py::object) { return NativeCam678Sensor::DRIVER_NAME; })
        .def_property_readonly_static("VERSION",
            [](py::object) { return NativeCam678Sensor::VERSION; })
        .def_property_readonly_static("I2C_ADDRESS",
            [](py::object) { return NativeCam678Sensor::I2C_ADDRESS; });
}

} // namespace hololink::sensors
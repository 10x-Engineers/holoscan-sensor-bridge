include(pybind11_add_hololink_module)
pybind11_add_hololink_module(
    CPP_CMAKE_TARGET native_cam380_camera_sensor
    CLASS_NAME "NativeCam380Sensor"
    SOURCES bindings.cpp
)
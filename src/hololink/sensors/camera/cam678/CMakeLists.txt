# cam678 POC sensor — mirrors imx274 structure exactly
add_library(native_cam678_camera_sensor STATIC
    native_cam678_sensor.cpp
)
set_property(TARGET native_cam678_camera_sensor PROPERTY POSITION_INDEPENDENT_CODE ON)
add_library(hololink::sensors::native_cam678_camera_sensor ALIAS native_cam678_camera_sensor)
target_include_directories(native_cam678_camera_sensor
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../..>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(native_cam678_camera_sensor
    PUBLIC
        hololink_camera_sensor
)
set_target_properties(native_cam678_camera_sensor
    PROPERTIES EXPORT_NAME sensors::native_cam678_camera_sensor)
install(TARGETS native_cam678_camera_sensor
        EXPORT HololinkTargets
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT hololink-sensors)
install(FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/native_cam678_sensor.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hololink/sensors/camera/cam678
        COMPONENT hololink-sensors)
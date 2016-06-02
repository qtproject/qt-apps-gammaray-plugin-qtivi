# probe plugin
gammaray_add_plugin(gammaray_qtivi JSON gammaray_qtivi.json SOURCES qtivisupport.cpp)
target_link_libraries(gammaray_qtivi gammaray_core Qt5::IviCore Qt5::IviVehicleFunctions)

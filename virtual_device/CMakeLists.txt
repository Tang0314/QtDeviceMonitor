cmake_minimum_required(VERSION 3.21)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.7 REQUIRED COMPONENTS
    Core Widgets Network
)

set(VD_SOURCES
    main.cpp
    VirtualDeviceWindow.cpp
    VirtualDeviceWindow.h
    ../src/mock/MockDataFormulas.h
    ../src/data/DeviceData.h
)

qt_add_executable(VirtualDevice ${VD_SOURCES})

target_include_directories(VirtualDevice PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(VirtualDevice PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
)

if(WIN32)
    set_target_properties(VirtualDevice PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

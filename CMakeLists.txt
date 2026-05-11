cmake_minimum_required(VERSION 3.21)
project(QtDeviceMonitor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.7 REQUIRED COMPONENTS
    Core Widgets Network SerialPort Sql Charts
)

set(SOURCES
    src/main.cpp
    src/ui/mainwindow.cpp
    src/ui/mainwindow.h
    src/ui/ChartWidget.cpp
    src/ui/ChartWidget.h
    src/ui/SettingsDialog.cpp
    src/ui/SettingsDialog.h
    src/ui/HistoryDialog.cpp
    src/ui/HistoryDialog.h
    src/data/DeviceData.h
    src/data/DatabaseManager.cpp
    src/data/DatabaseManager.h
    src/mock/MockDataGenerator.cpp
    src/mock/MockDataGenerator.h
    src/mock/VirtualTcpDevice.cpp
    src/mock/VirtualTcpDevice.h
    src/comm/TcpComm.cpp
    src/comm/TcpComm.h
    src/alarm/AlarmChecker.cpp
    src/alarm/AlarmChecker.h
    src/config/ConfigManager.cpp
    src/config/ConfigManager.h
)

qt_add_executable(QtDeviceMonitor ${SOURCES})

target_include_directories(QtDeviceMonitor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

target_link_libraries(QtDeviceMonitor PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::SerialPort
    Qt6::Sql
    Qt6::Charts
)

if(WIN32)
    set_target_properties(QtDeviceMonitor PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

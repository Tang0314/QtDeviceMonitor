cmake_minimum_required(VERSION 3.21)
project(QtDeviceMonitor VERSION 1.3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.7 REQUIRED COMPONENTS
    Core Widgets Network SerialPort Sql Charts Test
)

set(APP_SOURCES
    src/core/DeviceMonitorController.cpp
    src/core/DeviceMonitorController.h
    src/ui/ChartWidget.cpp
    src/ui/ChartWidget.h
    src/ui/SettingsDialog.cpp
    src/ui/SettingsDialog.h
    src/ui/HistoryDialog.cpp
    src/ui/HistoryDialog.h
    src/ui/SerialConfigDialog.cpp
    src/ui/SerialConfigDialog.h
    src/ui/AlarmHistoryDialog.cpp
    src/ui/AlarmHistoryDialog.h
    src/data/DeviceData.h
    src/data/DataParser.cpp
    src/data/DataParser.h
    src/data/DatabaseManager.cpp
    src/data/DatabaseManager.h
    src/data/DatabaseWorker.cpp
    src/data/DatabaseWorker.h
    src/mock/MockDataFormulas.h
    src/mock/MockDataGenerator.cpp
    src/mock/MockDataGenerator.h
    src/mock/VirtualTcpDevice.cpp
    src/mock/VirtualTcpDevice.h
    src/comm/TcpComm.cpp
    src/comm/TcpComm.h
    src/comm/SerialComm.cpp
    src/comm/SerialComm.h
    src/utils/LogManager.cpp
    src/utils/LogManager.h
    src/alarm/AlarmChecker.cpp
    src/alarm/AlarmChecker.h
    src/config/ConfigManager.cpp
    src/config/ConfigManager.h
)

set(MAIN_SOURCES
    src/main.cpp
    src/ui/mainwindow.cpp
    src/ui/mainwindow.h
)

qt_add_executable(QtDeviceMonitor ${MAIN_SOURCES} ${APP_SOURCES})

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

add_subdirectory(virtual_device)

enable_testing()
add_subdirectory(tests)

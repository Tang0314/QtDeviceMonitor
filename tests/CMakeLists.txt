qt_add_executable(DataParserTest
    tst_dataparser.cpp
    ../src/data/DataParser.cpp
    ../src/data/DataParser.h
    ../src/data/DeviceData.h
)

target_include_directories(DataParserTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(DataParserTest PRIVATE
    Qt6::Core
    Qt6::Test
)

add_test(NAME DataParserTest COMMAND DataParserTest)

qt_add_executable(AlarmCheckerTest
    tst_alarmchecker.cpp
    ../src/alarm/AlarmChecker.cpp
    ../src/alarm/AlarmChecker.h
    ../src/data/DeviceData.h
)

target_include_directories(AlarmCheckerTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(AlarmCheckerTest PRIVATE
    Qt6::Core
    Qt6::Test
)

add_test(NAME AlarmCheckerTest COMMAND AlarmCheckerTest)

qt_add_executable(DatabaseManagerTest
    tst_databasemanager.cpp
    ../src/data/DatabaseManager.cpp
    ../src/data/DatabaseManager.h
    ../src/data/DatabaseWorker.cpp
    ../src/data/DatabaseWorker.h
    ../src/data/DeviceData.h
    ../src/alarm/AlarmChecker.cpp
    ../src/alarm/AlarmChecker.h
)

target_include_directories(DatabaseManagerTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(DatabaseManagerTest PRIVATE
    Qt6::Core
    Qt6::Sql
    Qt6::Test
)

add_test(NAME DatabaseManagerTest COMMAND DatabaseManagerTest)

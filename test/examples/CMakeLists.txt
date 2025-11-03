# Examples for LogAndTrace module

# Find required packages
find_package(Threads REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../source/inc
    ${CMAKE_SOURCE_DIR}/modules/Core/source/inc
)

# Multi-sink comprehensive test example
add_executable(example_multi_sink example_multi_sink.cpp)
target_link_libraries(example_multi_sink 
    PRIVATE 
    ${PLATFORM_SYSTEM_TARGET}_core
    ${PLATFORM_SYSTEM_TARGET}_log
    Threads::Threads
    dlt
    Boost::regex
)

# Direct DLT API test
add_executable(test_dlt_direct test_dlt_direct.cpp)
target_link_libraries(test_dlt_direct 
    PRIVATE 
    dlt
)

# Install examples and config files
install(TARGETS example_multi_sink test_dlt_direct DESTINATION bin/examples)
install(FILES 
    config_console_file.json
    config_dlt.json
    config_syslog.json
    config_all_sinks.json
    DESTINATION bin/examples
)

message(STATUS "Added example: example_multi_sink")
message(STATUS "Added example: test_dlt_direct")

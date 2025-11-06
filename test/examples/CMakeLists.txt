# Examples for LogAndTrace module

# Find required packages
find_package(Threads REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../source/inc
    ${CMAKE_SOURCE_DIR}/modules/Core/source/inc
)

# Link libraries
set(EXAMPLE_LIBS
    ${PLATFORM_SYSTEM_TARGET}_core
    ${PLATFORM_SYSTEM_TARGET}_log
    Threads::Threads
)

# Basic usage example
add_executable(example_basic_usage example_basic_usage.cpp)
target_link_libraries(example_basic_usage PRIVATE ${EXAMPLE_LIBS})

# Multi-threaded logging example
add_executable(example_multi_thread example_multi_thread.cpp)
target_link_libraries(example_multi_thread PRIVATE ${EXAMPLE_LIBS})

# File rotation example
add_executable(example_file_rotation example_file_rotation.cpp)
target_link_libraries(example_file_rotation PRIVATE ${EXAMPLE_LIBS})

# Install examples and config files
install(TARGETS 
    example_basic_usage 
    example_multi_thread 
    example_file_rotation
    DESTINATION bin/examples
)

install(FILES 
    config_console_file.json
    config_dlt.json
    config_syslog.json
    config_all_sinks.json
    DESTINATION bin/examples
)

message(STATUS "Added example: example_basic_usage")
message(STATUS "Added example: example_multi_thread")
message(STATUS "Added example: example_file_rotation")

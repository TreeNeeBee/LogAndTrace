cmake_minimum_required ( VERSION "3.10.2" )
include ( ../../BuildTemplate/Config.cmake.in )

project ( Log )

set ( MODULE_NAME "Log" )
set ( MODULE_VERNO 1.0.0 )

set ( Boost_USE_STATIC_LIBS OFF )
set ( Boost_USE_MULTITHREADED ON )
set ( MODULE_EXTERNAL_INCLUDE_DIR /usr/local/include )
set ( MODULE_EXTERNAL_LIB_DIR /usr/local/lib )

# Build LogAndTrace against local Modules/Core outputs (headers + lib)
# Create symlink for core headers to match include structure
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include)
file(CREATE_LINK ${CMAKE_SOURCE_DIR}/modules/Core/source/inc 
     ${CMAKE_CURRENT_BINARY_DIR}/include/core SYMBOLIC)

set ( CORE_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/include )
set ( CORE_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR} )
list ( APPEND MODULE_EXTERNAL_INCLUDE_DIR ${CORE_INCLUDE_DIR} )
list ( APPEND MODULE_EXTERNAL_LIB_DIR ${CORE_LIB_DIR} )
set ( MODULE_EXTERNAL_LIB ${PLATFORM_SYSTEM_TARGET}_core dlt Threads::Threads Boost::filesystem Boost::regex )

set ( MODULE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR} )
set ( MODULE_SOURCE_CXX_DIR ${MODULE_ROOT_DIR}/source )
set ( ENABLE_BUILD_SHARED_LIBRARY ON CACHE BOOL "Build log shared library" FORCE )
set ( ENABLE_BUILD_WITH_PLATFORM_PREX ON )
include ( ../../BuildTemplate/SharedLibrary.cmake.in )

# Daemon disabled temporarily - include path issues
# set ( MODULE_DAEMON_DIR ${MODULE_ROOT_DIR}/test/examples )
# set ( ENABLE_BUILD_DAEMON ON CACHE BOOL "Build log examples" FORCE )
# set ( MODULE_EXTERNAL_DAEMON_LIB ${PLATFORM_SYSTEM_TARGET}_core ${PLATFORM_SYSTEM_TARGET}_log Threads::Threads dlt Boost::filesystem Boost::regex )
# include ( ../../BuildTemplate/Daemon.cmake.in )

# Unit tests
set ( MODULE_TEST_DIR ${MODULE_ROOT_DIR}/test )
set ( ENABLE_BUILD_TEST ON CACHE BOOL "Build log tests" FORCE )
set ( ENABLE_BUILD_UNITTEST ON CACHE BOOL "Build log unit tests" FORCE )
# 使用 GTest::GTest 而不是 GTest::Main，因为我们有自定义的 main 函数
set ( MODULE_EXTERNAL_TEST_LIB ${PLATFORM_SYSTEM_TARGET}_core ${PLATFORM_SYSTEM_TARGET}_log Threads::Threads dlt Boost::filesystem Boost::regex GTest::GTest )

include ( ../../BuildTemplate/Test.cmake.in )

# Register tests with CTest
add_test(NAME log_tests COMMAND log_test)

# Benchmarks (enable for this module only)
set ( ENABLE_MODULE_BENCHMARK ON )

if ( ENABLE_MODULE_BENCHMARK )
    set ( BENCHMARK_DIR ${MODULE_TEST_DIR}/benchmark )
    
    # Only compile the working benchmarks (exclude old API-based ones)
    set ( BENCHMARK_SOURCES 
        ${BENCHMARK_DIR}/benchmark_simple.cpp
        ${BENCHMARK_DIR}/benchmark_stress_test.cpp
        ${BENCHMARK_DIR}/benchmark_multiprocess.cpp
    )
    
    set ( BENCHMARK_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR} ${LOCAL_LIB_INCLUDE_DIRS} )
    set ( BENCHMARK_LIB ${PLATFORM_SYSTEM_TARGET}_core ${PLATFORM_SYSTEM_TARGET}_log Threads::Threads dlt Boost::filesystem Boost::regex )
    
    include_directories ( ${SDK_INCLUDE_DIR} ${BENCHMARK_INCLUDE_DIRS} ${MODULE_EXTERNAL_INCLUDE_DIR} )
    link_directories ( ${SDK_LIB_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${MODULE_EXTERNAL_LIB_DIR} )
    
    foreach ( BENCHMARK_SRC ${BENCHMARK_SOURCES} )
        get_filename_component ( BENCHMARK_NAME ${BENCHMARK_SRC} NAME_WE )
        
        add_executable ( ${BENCHMARK_NAME} ${BENCHMARK_SRC} )
        target_link_libraries ( ${BENCHMARK_NAME} PRIVATE ${BENCHMARK_LIB} )
        
        message ( STATUS "Added benchmark: ${BENCHMARK_NAME}" )
    endforeach ()
endif ()

# Build examples
set ( ENABLE_MODULE_EXAMPLES ON )

if ( ENABLE_MODULE_EXAMPLES )
    set ( EXAMPLES_DIR ${MODULE_TEST_DIR}/examples )
    
    set ( EXAMPLE_SOURCES 
        ${EXAMPLES_DIR}/example_multi_sink.cpp
        ${EXAMPLES_DIR}/test_dlt_direct.cpp
        ${EXAMPLES_DIR}/test_config_loading.cpp
        ${EXAMPLES_DIR}/test_dlt_long_message.cpp
    )
    
    set ( EXAMPLE_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR} ${LOCAL_LIB_INCLUDE_DIRS} )
    set ( EXAMPLE_LIB ${PLATFORM_SYSTEM_TARGET}_core ${PLATFORM_SYSTEM_TARGET}_log Threads::Threads dlt Boost::filesystem Boost::regex )
    
    include_directories ( ${SDK_INCLUDE_DIR} ${EXAMPLE_INCLUDE_DIRS} ${MODULE_EXTERNAL_INCLUDE_DIR} )
    link_directories ( ${SDK_LIB_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${MODULE_EXTERNAL_LIB_DIR} )
    
    foreach ( EXAMPLE_SRC ${EXAMPLE_SOURCES} )
        get_filename_component ( EXAMPLE_NAME ${EXAMPLE_SRC} NAME_WE )
        
        add_executable ( ${EXAMPLE_NAME} ${EXAMPLE_SRC} )
        
        if ( ${EXAMPLE_NAME} STREQUAL "example_multi_sink" )
            target_link_libraries ( ${EXAMPLE_NAME} PRIVATE ${EXAMPLE_LIB} )
        elseif ( ${EXAMPLE_NAME} STREQUAL "test_dlt_direct" )
            target_link_libraries ( ${EXAMPLE_NAME} PRIVATE dlt )
        elseif ( ${EXAMPLE_NAME} STREQUAL "test_config_loading" )
            target_link_libraries ( ${EXAMPLE_NAME} PRIVATE ${EXAMPLE_LIB} )
        elseif ( ${EXAMPLE_NAME} STREQUAL "test_dlt_long_message" )
            target_link_libraries ( ${EXAMPLE_NAME} PRIVATE ${EXAMPLE_LIB} )
        endif ()
        
        message ( STATUS "Added example: ${EXAMPLE_NAME}" )
    endforeach ()
    
    # Install config files
    install ( FILES
        ${EXAMPLES_DIR}/config_console_file.json
        ${EXAMPLES_DIR}/config_dlt.json
        ${EXAMPLES_DIR}/config_syslog.json
        ${EXAMPLES_DIR}/config_all_sinks.json
        DESTINATION bin/examples
    )
endif ()

# ==============================================================================
# Installation and Export Configuration
# ==============================================================================

# Install library with export support
install(TARGETS ${PLATFORM_SYSTEM_TARGET}_log
    EXPORT LogAndTraceTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include/lap/log
)

# Install headers
install(DIRECTORY ${MODULE_SOURCE_CXX_DIR}/inc/
    DESTINATION include/lap/log
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

# Install documentation
install(FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/README.md
    DESTINATION share/doc/lap-log
)

# ==============================================================================
# CMake Package Configuration (支持 find_package)
# ==============================================================================
include(CMakePackageConfigHelpers)

# 导出目标
install(EXPORT LogAndTraceTargets
    FILE LogAndTraceTargets.cmake
    NAMESPACE LightAP::
    DESTINATION lib/cmake/LogAndTrace
    COMPONENT Development
)

# 配置包配置文件
set(CMAKE_INSTALL_INCLUDEDIR "include/lap/log")
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/LogAndTraceConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/LogAndTraceConfig.cmake
    INSTALL_DESTINATION lib/cmake/LogAndTrace
    PATH_VARS CMAKE_INSTALL_INCLUDEDIR
)

# 生成版本文件
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/LogAndTraceConfigVersion.cmake
    VERSION ${MODULE_VERNO}
    COMPATIBILITY SameMajorVersion
)

# 安装配置文件
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/LogAndTraceConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/LogAndTraceConfigVersion.cmake
    DESTINATION lib/cmake/LogAndTrace
    COMPONENT Development
)

message(STATUS "[LogAndTrace] CMake package configuration will be installed to lib/cmake/LogAndTrace")

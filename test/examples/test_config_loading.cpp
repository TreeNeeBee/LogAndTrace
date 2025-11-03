/**
 * @file test_config_loading.cpp
 * @brief Test program to verify sinks configuration loading
 */

#include "CLogManager.hpp"
#include "CLogger.hpp"
#include <iostream>

using namespace lap::log;

int main()
{
    lap::core::MemManager::getInstance();  // Initialize memory manager first
    std::cout << "Testing configuration loading with sinks array...\n";
    
    // Initialize with test config
    auto& logMgr = LogManager::getInstance();
    bool result = logMgr.initialize(lap::core::InstanceSpecifier("test_sinks_config.json"));
    
    if (!result) {
        std::cerr << "Failed to initialize LogManager\n";
        return 1;
    }
    
    std::cout << "LogManager initialized successfully\n";
    
    // Get default logger and test logging
    auto& logger = logMgr.logger();
    
    std::cout << "\nTesting log output...\n";
    logger.LogInfo() << "This is an INFO message - should appear in file sink";
    logger.LogDebug() << "This is a DEBUG message - should appear in console only";
    logger.LogWarn() << "This is a WARN message - should appear in both sinks";
    logger.LogError() << "This is an ERROR message - should appear in both sinks";
    
    std::cout << "\nCheck /tmp/test_file_sink.log for file sink output\n";
    std::cout << "Configuration test completed successfully!\n";
    
    logMgr.uninitialize();
    
    // Use _Exit to avoid static destructor order issues
    // This skips calling destructors of static objects (like LogManager singleton)
    std::_Exit(0);
}

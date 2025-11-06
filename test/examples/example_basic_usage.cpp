/**
 * @file        example_basic_usage.cpp
 * @brief       Basic usage example for LightAP logging system
 * @date        2025-11-06
 * @details     Demonstrates initialization, logger creation, and basic logging
 */

#include "CLogManager.hpp"
#include "CLogger.hpp"
#include <lap/core/CMemory.hpp>
#include <lap/core/CConfig.hpp>
#include <lap/core/CPath.hpp>

using namespace lap::log;
using namespace lap::core;

int main(int argc, char* argv[]) {
    // Initialize memory manager first
    MemManager::getInstance();
    
    // Initialize ConfigManager with config file
    auto& cfgMgr = ConfigManager::getInstance();
    
    if (argc > 1) {
        // Use provided config file
        String configPath(argv[1]);
        cfgMgr.initialize(configPath, false);
    } else {
        // Use default config in current directory
        auto appFolder = Path::getApplicationFolder();
        auto configPath = Path::append(appFolder, "config_console_file.json");
        cfgMgr.initialize(String(configPath.data()), false);
    }
    
    // Initialize LogManager (loads config from ConfigManager)
    auto& logMgr = LogManager::getInstance();
    if (!logMgr.initialize()) {
        return 1;
    }
    
    // Get default logger
    auto& logger = logMgr.logger();
    
    // Basic logging - all levels (stream-style)
    logger.LogFatal() << "This is a FATAL message";
    logger.LogError() << "This is an ERROR message";
    logger.LogWarn() << "This is a WARNING message";
    logger.LogInfo() << "This is an INFO message";
    logger.LogDebug() << "This is a DEBUG message";
    logger.LogVerbose() << "This is a VERBOSE message";
    
    // Stream-style logging with formatted data
    logger.LogInfo() << "Stream style: counter = " << 42;
    logger.LogError() << "Error code: 0x" << LogHex32{0xDEADBEEF};
    
    // With location info
    logger.LogWarn().WithLocation(__FILE__, __LINE__) << "Warning with location";
    
    // Register a custom logger with specific context
    auto& customLogger = logMgr.registerLogger("CUSTOM", "Custom Context", LogLevel::kDebug);
    customLogger.LogInfo() << "Message from custom logger";
    customLogger.LogDebug() << "Debug message from custom logger";
    
    return 0;
}

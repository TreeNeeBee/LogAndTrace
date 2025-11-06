/**
 * @file        example_file_rotation.cpp
 * @brief       File sink with rotation example
 * @date        2025-11-06
 * @details     Demonstrates file logging with automatic rotation
 */

#include "CLogManager.hpp"
#include "CLogger.hpp"
#include <core/CMemory.hpp>
#include <core/CConfig.hpp>
#include <core/CPath.hpp>

using namespace lap::log;
using namespace lap::core;

int main() {
    // Initialize memory manager
    MemManager::getInstance();
    
    // Initialize ConfigManager
    // Note: Configure file rotation in config JSON:
    // "fileRotation": {
    //     "maxSize": 1048576,    // 1MB
    //     "maxFiles": 5
    // }
    auto& cfgMgr = ConfigManager::getInstance();
    auto appFolder = Path::getApplicationFolder();
    auto configPath = Path::append(appFolder, "config_console_file.json");
    cfgMgr.initialize(String(configPath.data()), false);
    
    // Initialize LogManager
    auto& logMgr = LogManager::getInstance();
    if (!logMgr.initialize()) {
        return 1;
    }
    
    auto& logger = logMgr.logger();
    
    logger.LogInfo() << "=== File Rotation Example ===";
    logger.LogInfo() << "Writing large number of log messages to trigger rotation...";
    
    // Generate enough logs to trigger rotation (if configured)
    for (int i = 0; i < 10000; ++i) {
        logger.LogInfo() << "Log message #" << i << " - This is a sample message to demonstrate file rotation";
        
        if (i % 1000 == 0) {
            logger.LogInfo() << "Progress: " << i << " messages written";
        }
    }
    
    logger.LogInfo() << "=== Rotation Test Complete ===";
    logger.LogInfo() << "Check the log directory for rotated files (e.g., app.log.1, app.log.2, etc.)";
    
    return 0;
}

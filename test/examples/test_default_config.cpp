/**
 * @file        test_default_config.cpp
 * @author      ddkv587 ( ddkv587@gmail.com )
 * @brief       Test default configuration without config file
 * @date        2025-10-29
 */

#include <iostream>
#include "CLogManager.hpp"

using namespace lap::log;

int main() {
    core::MemManager::getInstance();  // Initialize memory manager first
    std::cout << "=== Testing Default Configuration (No Config File) ===" << std::endl;
    
    // Initialize without config file
    // Should use default: Console sink with Info level
    auto& logMgr = LogManager::getInstance();
    logMgr.initialize();
    
    // Get default logger
    auto& logger = logMgr.logger();
    
    std::cout << "\nWriting test logs with default config..." << std::endl;
    
    // Test all log levels
    logger.fatal("FATAL: This should appear (default level is Info)");
    logger.error("ERROR: This should appear");
    logger.warn("WARN: This should appear");
    logger.info("INFO: This should appear");
    logger.debug("DEBUG: This should NOT appear (below Info level)");
    logger.verbose("VERBOSE: This should NOT appear (below Info level)");
    
    std::cout << "\n=== Test Complete ===" << std::endl;
    std::cout << "Expected output: FATAL, ERROR, WARN, INFO" << std::endl;
    std::cout << "Expected sink: Console only (no file)" << std::endl;
    
    return 0;
}

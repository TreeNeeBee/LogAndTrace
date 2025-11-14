/**
 * @file        example_base64_encode.cpp
 * @brief       Example showing base64 encoding feature for log messages
 * @date        2025-11-14
 */

#include "CLogManager.hpp"
#include "CLogger.hpp"
#include <lap/core/CMemory.hpp>
#include <lap/core/CConfig.hpp>
#include <unistd.h>

using namespace lap::log;
using namespace lap::core;

int main()
{
    // Initialize memory and config managers
    MemoryManager::getInstance();
    auto& cfgMgr = ConfigManager::getInstance();
    
    // Try to load config (ignore errors if file doesn't exist)
    String configPath = "config_base64_test.json";
    cfgMgr.initialize(configPath, false);
    
    // Get LogManager instance (auto-initializes)
    auto& logMgr = LogManager::getInstance();
    UNUSED(logMgr);
    
    // Create a logger context
    auto& logger = CreateLogger("ENC", "Base64 Encoding Test", LogLevel::kDebug);
    
    // Test 1: Normal logging (without encoding)
    logger.LogInfo() << "Test 1: Normal log message without encoding";
    
    // Test 2: Use WithEncode() to enable base64 encoding on LogStream
    logger.LogInfo().WithEncode() << "Test 2: This message will be base64 encoded";
    
    // Test 3: Chain WithLevel and WithEncode
    logger.WithLevel(LogLevel::kWarn).WithEncode() << "Test 3: Warning with encoding - Sensitive data: password=secret123";
    logger.WithLevel(LogLevel::kError).WithEncode() << "Test 4: Error with encoding - Special chars: @#$%^&*()";
    
    // Test 5: Normal logging again (each stream is independent)
    logger.LogInfo() << "Test 5: Back to normal logging without encoding";
    
    // Test 6: Complex message with encoding
    logger.WithLevel(LogLevel::kDebug).WithEncode() << "Test 6: User: admin, IP: 192.168.1.100, Action: login";
    
    // Test 7: Explicitly disable encoding (WithEncode(false))
    logger.LogInfo().WithEncode(false) << "Test 7: This will NOT be encoded even though we called WithEncode(false)";
    
    // Test 8: WithLocation combined with WithEncode
    logger.LogInfo().WithLocation(__FILE__, __LINE__).WithEncode() << "Test 8: Location + Encoding";
    
    // Test 9: Multiple WithEncode calls in sequence
    logger.LogDebug().WithEncode() << "Test 9a: First encoded message";
    logger.LogDebug().WithEncode() << "Test 9b: Second encoded message";
    logger.LogDebug() << "Test 9c: Normal message after encoded ones";
    
    sleep(1);  // Give time for async operations
    
    return 0;
}

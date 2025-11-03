/**
 * @file syslog_example.cpp
 * @brief Example demonstrating syslog sink usage
 * 
 * This example shows how to use the SyslogSink to send logs to the system syslog.
 * You can view the logs with:
 *   - journalctl -f  (systemd-based systems)
 *   - tail -f /var/log/syslog  (traditional syslog)
 */

#include "CLogManager.hpp"
#include "CLogger.hpp"
#include "CLogStream.hpp"
#include "CCommon.hpp"
#include <thread>
#include <chrono>

using namespace lap::log;

int main()
{
    core::MemManager::getInstance();  // Initialize memory manager first
    // Initialize LogManager with syslog enabled
    // Note: Syslog mode needs to be enabled in config
    auto& logManager = LogManager::getInstance();
    
    // For demonstration, we'll manually configure syslog mode
    // In production, this would come from your config.json
    logManager.setLogMode(
        static_cast<LogMode>(
            static_cast<core::UInt8>(LogMode::kConsole) | 
            static_cast<core::UInt8>(LogMode::kSyslog)
        )
    );
    logManager.setLogLevel(LogLevel::kDebug);
    
    // Register a logger context
    auto logger = logManager.registerContext("SYSLOG", "Syslog Example", LogLevel::kDebug);
    
    if (!logger) {
        std::cerr << "Failed to register logger context\n";
        return 1;
    }
    
    std::cout << "=== Syslog Example ===\n";
    std::cout << "Sending logs to both console and syslog...\n";
    std::cout << "View syslog with: journalctl -f | grep LightAP\n";
    std::cout << "Or: tail -f /var/log/syslog | grep LightAP\n\n";
    
    // Send various log levels
    LOG_FATAL(*logger) << "This is a FATAL message to syslog";
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    LOG_ERROR(*logger) << "This is an ERROR message to syslog";
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    LOG_WARN(*logger) << "This is a WARNING message to syslog";
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    LOG_INFO(*logger) << "This is an INFO message to syslog";
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    LOG_DEBUG(*logger) << "This is a DEBUG message to syslog";
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Demonstrate structured logging
    LOG_INFO(*logger) << "User login successful: username=alice, ip=192.168.1.100";
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    LOG_WARN(*logger) << "High memory usage detected: " << 85 << "% used";
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    LOG_ERROR(*logger) << "Connection failed: timeout after " << 30 << " seconds";
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::cout << "\n=== Logs sent to syslog ===\n";
    std::cout << "Check your syslog for messages tagged with [LightAP] and [SYSLOG]\n";
    
    return 0;
}

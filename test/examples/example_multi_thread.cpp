/**
 * @file        example_multi_thread.cpp
 * @brief       Multi-threaded logging example
 * @date        2025-11-06
 * @details     Demonstrates thread-safe concurrent logging
 */

#include "CLogManager.hpp"
#include "CLogger.hpp"
#include <lap/core/CMemory.hpp>
#include <lap/core/CConfig.hpp>
#include <lap/core/CPath.hpp>
#include <thread>
#include <vector>
#include <atomic>

using namespace lap::log;
using namespace lap::core;

int main() {
    // Initialize memory manager
    MemManager::getInstance();
    
    // Initialize ConfigManager with config
    auto& cfgMgr = ConfigManager::getInstance();
    auto appFolder = Path::getApplicationFolder();
    auto configPath = Path::append(appFolder, "config_console_file.json");
    cfgMgr.initialize(String(configPath.data()), false);
    
    // Initialize LogManager
    auto& logMgr = LogManager::getInstance();
    if (!logMgr.initialize()) {
        return 1;
    }
    
    // Create logger for multi-threading test
    auto& logger = logMgr.registerLogger("MTHD", "Multi-Thread", LogLevel::kInfo);
    
    logger.LogInfo() << "Starting multi-threaded logging test...";
    
    const int NUM_THREADS = 10;
    const int LOGS_PER_THREAD = 100;
    
    std::atomic<int> ready{0};
    std::atomic<bool> start{false};
    std::vector<std::thread> threads;
    
    // Create worker threads
    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([&, t]() {
            ready++;
            
            // Wait for all threads to be ready
            while (!start.load()) {
                std::this_thread::yield();
            }
            
            // Log messages
            for (int i = 0; i < LOGS_PER_THREAD; ++i) {
                logger.LogInfo() << "Thread " << t << " message #" << i;
            }
        });
    }
    
    // Wait for all threads to be ready
    while (ready.load() < NUM_THREADS) {
        std::this_thread::yield();
    }
    
    // Start all threads simultaneously
    start = true;
    
    // Wait for completion
    for (auto& t : threads) {
        t.join();
    }
    
    logger.LogInfo() << "Multi-threaded test completed successfully";
    logger.LogInfo() << "Total messages logged: " << (NUM_THREADS * LOGS_PER_THREAD);
    
    return 0;
}

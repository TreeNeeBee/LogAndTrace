/**
 * @file        example_multi_sink.cpp
 * @brief       Comprehensive example demonstrating all sinks with multi-threading and stress testing
 * @date        2025-10-29
 * @details     Tests Console, File, Syslog, and DLT sinks under various conditions
 */

#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <CLog.hpp>
#include <core/CMemory.hpp>
#include <core/CInstanceSpecifier.hpp>

using namespace lap::log;
using namespace lap::core;

// Global counters for statistics
std::atomic<uint64_t> g_totalLogs{0};
std::atomic<uint64_t> g_errors{0};

/**
 * @brief Basic logging test - demonstrates all log levels
 */
void testBasicLogging() {
    std::cout << "\n=== Test 1: Basic Logging (All Levels) ===" << std::endl;
    
    auto& logger = CreateLogger("BASIC", "Basic Test");
    
    logger.LogFatal() << "This is a FATAL message";
    logger.LogError() << "This is an ERROR message";
    logger.LogWarn() << "This is a WARNING message";
    logger.LogInfo() << "This is an INFO message";
    logger.LogDebug() << "This is a DEBUG message";
    logger.LogVerbose() << "This is a VERBOSE message";
    
    g_totalLogs += 6;
    std::cout << "✓ Basic logging test completed (6 messages)" << std::endl;
}

/**
 * @brief Multi-threaded logging test
 */
void testMultiThreaded() {
    std::cout << "\n=== Test 2: Multi-Threaded Logging ===" << std::endl;
    
    const int NUM_THREADS = 10;
    const int LOGS_PER_THREAD = 1000;
    
    auto& logger = CreateLogger("MT", "Multi-Thread Test");
    
    std::atomic<int> ready{0};
    std::atomic<bool> start{false};
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([&, t]() {
            ready++;
            while (!start.load()) {
                std::this_thread::yield();
            }
            
            for (int i = 0; i < LOGS_PER_THREAD; ++i) {
                logger.LogInfo() << "Thread " << t << " message #" << i;
                g_totalLogs++;
            }
        });
    }
    
    // Wait for all threads ready
    while (ready.load() < NUM_THREADS) {
        std::this_thread::yield();
    }
    
    start = true;
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    
    int total_logs = NUM_THREADS * LOGS_PER_THREAD;
    double throughput = (total_logs * 1000.0) / duration;
    
    std::cout << "✓ Multi-threaded test completed" << std::endl;
    std::cout << "  Threads:    " << NUM_THREADS << std::endl;
    std::cout << "  Total logs: " << total_logs << std::endl;
    std::cout << "  Duration:   " << duration << " ms" << std::endl;
    std::cout << "  Throughput: " << static_cast<int>(throughput) << " logs/sec" << std::endl;
}

/**
 * @brief Stress test with sustained high-rate logging
 */
void testStress() {
    std::cout << "\n=== Test 3: Stress Test (Sustained Load) ===" << std::endl;
    
    const int NUM_THREADS = 8;
    const int DURATION_SEC = 5;
    
    auto& logger = CreateLogger("STRESS", "Stress Test");
    
    std::atomic<bool> stop{false};
    std::atomic<uint64_t> log_count{0};
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([&, t]() {
            uint64_t local_count = 0;
            while (!stop.load()) {
                logger.LogInfo() << "Stress thread " << t << " log #" << local_count;
                local_count++;
            }
            log_count += local_count;
        });
    }
    
    // Run for specified duration
    std::this_thread::sleep_for(std::chrono::seconds(DURATION_SEC));
    stop = true;
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto actual_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    
    uint64_t total = log_count.load();
    g_totalLogs += total;
    double throughput = (total * 1000.0) / actual_duration;
    
    std::cout << "✓ Stress test completed" << std::endl;
    std::cout << "  Duration:   " << actual_duration << " ms" << std::endl;
    std::cout << "  Total logs: " << total << std::endl;
    std::cout << "  Throughput: " << static_cast<int>(throughput) << " logs/sec" << std::endl;
}

/**
 * @brief Memory usage validation
 */
void testMemory() {
    std::cout << "\n=== Test 4: Memory Usage Validation ===" << std::endl;
    
    auto stats_before = Memory::getMemoryStats();
    
    std::cout << "Before test:" << std::endl;
    std::cout << "  Alloc count: " << stats_before.currentAllocCount << std::endl;
    std::cout << "  Alloc size:  " << stats_before.currentAllocSize << " bytes" << std::endl;
    std::cout << "  Pool memory: " << stats_before.totalPoolMemory << " bytes" << std::endl;
    
    auto& logger = CreateLogger("MEM", "Memory Test");
    
    const int NUM_LOGS = 100000;
    for (int i = 0; i < NUM_LOGS; ++i) {
        logger.LogInfo() << "Memory test message #" << i;
    }
    g_totalLogs += NUM_LOGS;
    
    auto stats_after = Memory::getMemoryStats();
    
    std::cout << "\nAfter " << NUM_LOGS << " logs:" << std::endl;
    std::cout << "  Alloc count: " << stats_after.currentAllocCount << std::endl;
    std::cout << "  Alloc size:  " << stats_after.currentAllocSize << " bytes" << std::endl;
    std::cout << "  Pool memory: " << stats_after.totalPoolMemory << " bytes" << std::endl;
    std::cout << "  Growth:      " << (stats_after.currentAllocSize - stats_before.currentAllocSize) << " bytes" << std::endl;
    
    if (stats_after.currentAllocSize == stats_before.currentAllocSize) {
        std::cout << "✓ Zero memory growth confirmed!" << std::endl;
    } else {
        std::cout << "⚠ Memory growth detected: " << (stats_after.currentAllocSize - stats_before.currentAllocSize) << " bytes" << std::endl;
        g_errors++;
    }
}

/**
 * @brief Mixed workload test
 */
void testMixedWorkload() {
    std::cout << "\n=== Test 5: Mixed Workload (Multiple Loggers) ===" << std::endl;
    
    auto& logger1 = CreateLogger("MIX1", "Mixed Logger 1");
    auto& logger2 = CreateLogger("MIX2", "Mixed Logger 2");
    auto& logger3 = CreateLogger("MIX3", "Mixed Logger 3");
    
    const int ITERATIONS = 1000;
    
    std::vector<std::thread> threads;
    
    // Thread 1: High frequency info
    threads.emplace_back([&]() {
        for (int i = 0; i < ITERATIONS; ++i) {
            logger1.LogInfo() << "Info message #" << i;
            g_totalLogs++;
        }
    });
    
    // Thread 2: Mix of levels
    threads.emplace_back([&]() {
        for (int i = 0; i < ITERATIONS; ++i) {
            if (i % 4 == 0) logger2.LogError() << "Error #" << i;
            else if (i % 4 == 1) logger2.LogWarn() << "Warning #" << i;
            else if (i % 4 == 2) logger2.LogInfo() << "Info #" << i;
            else logger2.LogDebug() << "Debug #" << i;
            g_totalLogs++;
        }
    });
    
    // Thread 3: Verbose logging
    threads.emplace_back([&]() {
        for (int i = 0; i < ITERATIONS; ++i) {
            logger3.LogVerbose() << "Verbose details #" << i;
            g_totalLogs++;
        }
    });
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "✓ Mixed workload test completed (" << (ITERATIONS * 3) << " messages)" << std::endl;
}

/**
 * @brief Display test summary
 */
void printSummary() {
    std::cout << "\n=============================================" << std::endl;
    std::cout << "  Test Summary" << std::endl;
    std::cout << "=============================================" << std::endl;
    std::cout << "Total logs generated: " << g_totalLogs.load() << std::endl;
    std::cout << "Errors detected:      " << g_errors.load() << std::endl;
    
    if (g_errors.load() == 0) {
        std::cout << "\n✅ All tests PASSED!" << std::endl;
    } else {
        std::cout << "\n❌ Some tests FAILED!" << std::endl;
    }
    std::cout << "=============================================" << std::endl;
}

int main(int argc, char* argv[]) {
    // Initialize memory manager
    MemManager::getInstance();
    
    std::cout << "=============================================" << std::endl;
    std::cout << "  LightAP Multi-Sink Test Example" << std::endl;
    std::cout << "=============================================" << std::endl;
    
    // Determine config file
    std::string config_file = "config.json";
    std::string config_source;
    
    if (argc > 1) {
        config_source = argv[1];
        std::cout << "Using config file: " << config_source << std::endl;
        
        // Copy the specified config to config.json in current directory
        std::ifstream src(config_source, std::ios::binary);
        if (!src.is_open()) {
            std::cerr << "Failed to open config file: " << config_source << std::endl;
            return 1;
        }
        
        std::ofstream dst("config.json", std::ios::binary);
        if (!dst.is_open()) {
            std::cerr << "Failed to create config.json in current directory" << std::endl;
            return 1;
        }
        
        dst << src.rdbuf();
        src.close();
        dst.close();
        
        std::cout << "✓ Config copied to config.json" << std::endl;
    } else {
        std::cout << "No config file specified, using default initialization" << std::endl;
        // Use default initialization without config
        if (!LogManager::getInstance().initialize()) {
            std::cerr << "Failed to initialize LogManager with defaults" << std::endl;
            return 1;
        }
        std::cout << "✓ LogManager initialized with defaults" << std::endl;
        
        goto run_tests;
    }
    
    // Initialize logging with config
    {
        InstanceSpecifier spec(config_file);
        if (!LogManager::getInstance().initialize(spec)) {
            std::cerr << "Failed to initialize LogManager with config: " << config_file << std::endl;
            return 1;
        }
    }
    
    std::cout << "✓ LogManager initialized successfully" << std::endl;

run_tests:
    // Run all tests
    try {
        testBasicLogging();
        testMultiThreaded();
        testStress();
        testMemory();
        testMixedWorkload();
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Exception caught: " << e.what() << std::endl;
        g_errors++;
    }
    
    // Display summary
    printSummary();
    
    return (g_errors.load() == 0) ? 0 : 1;
}

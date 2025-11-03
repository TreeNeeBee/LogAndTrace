/**
 * @file        benchmark_throughput.cpp
 * @author      ddkv587 ( ddkv587@gmail.com )
 * @brief       Throughput benchmark for log system
 * @date        2025-10-28
 * 
 * @details     Tests:
 *              - Single-threaded write throughput
 *              - Multi-threaded write throughput
 *              - Different sink types comparison
 *              - Different log message sizes
 */

#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <atomic>
#include <iomanip>
#include "CLogManager.hpp"
#include "CLogger.hpp"
#include "CSinkManager.hpp"
#include "CConsoleSink.hpp"
#include "CFileSink.hpp"
#include "CSyslogSink.hpp"

using namespace lap::log;
using namespace lap::core;
using namespace std::chrono;

// Global counters
std::atomic<uint64_t> g_totalLogs{0};

/**
 * @brief Helper function to create a log entry
 */
static LogEntry* createLogEntry(
    LogLevelType level,
    StringView contextId,
    StringView message
) {
    Size entrySize = LogEntry::calculateSize(contextId.size(), message.size());
    void* mem = ::operator new(entrySize);
    LogEntry* entry = new (mem) LogEntry();
    
    auto now = system_clock::now();
    entry->timestamp = duration_cast<microseconds>(now.time_since_epoch()).count();
    entry->threadId = static_cast<UInt32>(std::hash<std::thread::id>{}(std::this_thread::get_id()));
    entry->level = level;
    entry->contextIdLen = static_cast<UInt16>(contextId.size());
    entry->messageLen = static_cast<UInt16>(message.size());
    
    char* data = reinterpret_cast<char*>(entry + 1);
    std::memcpy(data, contextId.data(), contextId.size());
    std::memcpy(data + contextId.size(), message.data(), message.size());
    
    return entry;
}

void printHeader(const String& title) {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "  " << title << std::endl;
    std::cout << std::string(70, '=') << std::endl;
}

void printResult(const String& test, uint64_t count, double durationMs, uint64_t throughput) {
    std::cout << std::left << std::setw(40) << test
              << std::right << std::setw(8) << count << " logs"
              << std::setw(10) << std::fixed << std::setprecision(2) << durationMs << " ms"
              << std::setw(12) << throughput << " logs/sec"
              << std::endl;
}

/**
 * @brief Benchmark single-threaded throughput with file sink
 */
void benchmarkFileSinkThroughput() {
    printHeader("File Sink Throughput (Single Thread)");
    
    const char* testFile = "/tmp/lap_benchmark_throughput.log";
    ::unlink(testFile);
    
    SinkManager manager;
    auto sink = std::make_unique<FileSink>(testFile, 100 * 1024 * 1024, 1, LogLevel::kVerbose);
    manager.addSink(std::move(sink));
    
    // Test different message sizes
    const std::vector<std::pair<int, String>> testCases = {
        {10000, "Short"},
        {10000, "Medium length message with some details and context information"},
        {10000, "Long message with lots of content to simulate real-world logging scenarios "
                "including timestamps, context, detailed error information, stack traces, "
                "and other diagnostic data that might be logged in production systems"}
    };
    
    for (const auto& testCase : testCases) {
        int count = testCase.first;
        const String& msg = testCase.second;
        
        auto start = high_resolution_clock::now();
        
        for (int i = 0; i < count; ++i) {
            auto* entry = createLogEntry(static_cast<lap::log::LogLevelType>(0x04), "BENCH", msg.c_str());
            manager.write(*entry);
            ::operator delete(entry);
        }
        
        manager.flushAll();
        
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end - start);
        uint64_t throughput = (count * 1000ULL) / duration.count();
        
        String testName = "Message size: " + std::to_string(msg.length()) + " bytes";
        printResult(testName, count, duration.count(), throughput);
    }
    
    ::unlink(testFile);
}

/**
 * @brief Benchmark multi-threaded throughput
 */
void benchmarkMultiThreadedThroughput() {
    printHeader("Multi-Threaded Throughput");
    
    const char* testFile = "/tmp/lap_benchmark_mt.log";
    ::unlink(testFile);
    
    const std::vector<int> threadCounts = {1, 2, 4, 8};
    const int LOGS_PER_THREAD = 5000;
    
    for (int numThreads : threadCounts) {
        SinkManager manager;
        auto sink = std::make_unique<FileSink>(testFile, 100 * 1024 * 1024, 1, LogLevel::kVerbose);
        manager.addSink(std::move(sink));
        
        std::vector<std::thread> threads;
        
        auto start = high_resolution_clock::now();
        
        for (int t = 0; t < numThreads; ++t) {
            threads.emplace_back([&manager, t]() {
                for (int i = 0; i < LOGS_PER_THREAD; ++i) {
                    String msg = "Thread " + std::to_string(t) + " message #" + std::to_string(i);
                    auto* entry = createLogEntry(static_cast<lap::log::LogLevelType>(0x04), "MT", msg.c_str());
                    manager.write(*entry);
                    ::operator delete(entry);
                }
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
        
        manager.flushAll();
        
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end - start);
        uint64_t totalLogs = numThreads * LOGS_PER_THREAD;
        uint64_t throughput = (totalLogs * 1000ULL) / duration.count();
        
        String testName = std::to_string(numThreads) + " thread(s)";
        printResult(testName, totalLogs, duration.count(), throughput);
        
        ::unlink(testFile);
    }
}

/**
 * @brief Compare different sink types
 */
void benchmarkSinkTypeComparison() {
    printHeader("Sink Type Comparison (10K logs each)");
    
    const int NUM_LOGS = 10000;
    const String message = "Standard log message for comparison benchmark";
    
    // File Sink
    {
        const char* testFile = "/tmp/lap_benchmark_file.log";
        ::unlink(testFile);
        
        SinkManager manager;
        auto sink = std::make_unique<FileSink>(testFile, 100 * 1024 * 1024, 1, LogLevel::kVerbose);
        manager.addSink(std::move(sink));
        
        auto start = high_resolution_clock::now();
        
        for (int i = 0; i < NUM_LOGS; ++i) {
            auto* entry = createLogEntry(static_cast<lap::log::LogLevelType>(0x04), "FILE", message.c_str());
            manager.write(*entry);
            ::operator delete(entry);
        }
        
        manager.flushAll();
        
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end - start);
        uint64_t throughput = (NUM_LOGS * 1000ULL) / duration.count();
        
        printResult("File Sink", NUM_LOGS, duration.count(), throughput);
        ::unlink(testFile);
    }
    
    // Console Sink (disabled output to avoid terminal spam)
    {
        SinkManager manager;
        auto sink = std::make_unique<ConsoleSink>(false, LogLevel::kVerbose);
        manager.addSink(std::move(sink));
        
        auto start = high_resolution_clock::now();
        
        for (int i = 0; i < NUM_LOGS; ++i) {
            auto* entry = createLogEntry(static_cast<lap::log::LogLevelType>(0x04), "CON", message.c_str());
            manager.write(*entry);
            ::operator delete(entry);
        }
        
        manager.flushAll();
        
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end - start);
        uint64_t throughput = (NUM_LOGS * 1000ULL) / duration.count();
        
        printResult("Console Sink (disabled)", NUM_LOGS, duration.count(), throughput);
    }
    
    // Syslog Sink
    {
        SinkManager manager;
        auto sink = std::make_unique<SyslogSink>("LAPBench", LOG_USER, LogLevel::kVerbose);
        manager.addSink(std::move(sink));
        
        auto start = high_resolution_clock::now();
        
        for (int i = 0; i < NUM_LOGS; ++i) {
            auto* entry = createLogEntry(static_cast<lap::log::LogLevelType>(0x04), "SYS", message.c_str());
            manager.write(*entry);
            ::operator delete(entry);
        }
        
        manager.flushAll();
        
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end - start);
        uint64_t throughput = (NUM_LOGS * 1000ULL) / duration.count();
        
        printResult("Syslog Sink", NUM_LOGS, duration.count(), throughput);
    }
}

/**
 * @brief Sustained throughput test
 */
void benchmarkSustainedThroughput() {
    printHeader("Sustained Throughput (1 minute test)");
    
    const char* testFile = "/tmp/lap_benchmark_sustained.log";
    ::unlink(testFile);
    
    SinkManager manager;
    auto sink = std::make_unique<FileSink>(testFile, 500 * 1024 * 1024, 3, LogLevel::kVerbose);
    manager.addSink(std::move(sink));
    
    const String message = "Sustained throughput test message with moderate length";
    const int DURATION_SECONDS = 60;
    
    std::cout << "Running for " << DURATION_SECONDS << " seconds..." << std::endl;
    
    auto start = high_resolution_clock::now();
    auto deadline = start + seconds(DURATION_SECONDS);
    uint64_t count = 0;
    
    while (high_resolution_clock::now() < deadline) {
        auto* entry = createLogEntry(static_cast<lap::log::LogLevelType>(0x04), "SUST", message.c_str());
        manager.write(*entry);
        ::operator delete(entry);
        ++count;
        
        // Report progress every 10 seconds
        if (count % 100000 == 0) {
            auto elapsed = duration_cast<seconds>(high_resolution_clock::now() - start).count();
            if (elapsed > 0) {
                std::cout << "  " << elapsed << "s: " << count << " logs (" 
                          << (count / elapsed) << " logs/sec)" << std::endl;
            }
        }
    }
    
    manager.flushAll();
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    uint64_t throughput = (count * 1000ULL) / duration.count();
    
    printResult("Sustained (1 min)", count, duration.count(), throughput);
    
    ::unlink(testFile);
}

int main() {
    lap::core::MemManager::getInstance();  // Initialize memory manager first
    std::cout << "\n╔════════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║         LightAP Log System - Throughput Benchmark             ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════════════╝" << std::endl;
    
    try {
        benchmarkFileSinkThroughput();
        benchmarkMultiThreadedThroughput();
        benchmarkSinkTypeComparison();
        benchmarkSustainedThroughput();
        
        std::cout << "\n" << std::string(70, '=') << std::endl;
        std::cout << "  Benchmark completed successfully!" << std::endl;
        std::cout << std::string(70, '=') << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Benchmark failed: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

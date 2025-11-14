/**
 * @file        benchmark_memory.cpp
 * @author      ddkv587 ( ddkv587@gmail.com )
 * @brief       Memory usage benchmark for log system
 * @date        2025-10-28
 * 
 * @details     Tests:
 *              - Memory per log entry
 *              - Memory usage growth
 *              - Memory leak detection
 *              - Peak memory usage
 */

#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <iomanip>
#include <fstream>
#include <sstream>
#include "CSinkManager.hpp"
#include "CFileSink.hpp"
#include "CConsoleSink.hpp"
#include <lap/core/CInitialization.hpp>

using namespace lap::log;
using namespace lap::core;
using namespace std::chrono;

/**
 * @brief Get current process memory usage in KB
 */
size_t getCurrentMemoryUsage() {
    std::ifstream status("/proc/self/status");
    std::string line;
    
    while (std::getline(status, line)) {
        if (line.find("VmRSS:") == 0) {
            std::istringstream iss(line);
            std::string tag;
            size_t size;
            iss >> tag >> size;
            return size; // Already in KB
        }
    }
    
    return 0;
}

/**
 * @brief Helper function to create a log entry
 */
static LogEntry* createLogEntry(
    DltLogLevelType level,
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

/**
 * @brief Calculate LogEntry size for different message lengths
 */
void benchmarkLogEntrySize() {
    printHeader("LogEntry Memory Overhead");
    
    std::cout << std::left;
    std::cout << std::setw(30) << "Message Length" 
              << std::setw(20) << "Total Size (bytes)"
              << std::setw(20) << "Overhead (bytes)" << std::endl;
    std::cout << std::string(70, '-') << std::endl;
    
    const std::vector<size_t> messageLengths = {10, 50, 100, 200, 500, 1000};
    const StringView contextId = "BENCH";
    
    for (size_t msgLen : messageLengths) {
        String message(msgLen, 'X');
        size_t totalSize = LogEntry::calculateSize(contextId.size(), message.size());
        size_t overhead = totalSize - msgLen - contextId.size();
        
        std::cout << std::setw(30) << (std::to_string(msgLen) + " bytes")
                  << std::setw(20) << totalSize
                  << std::setw(20) << overhead << std::endl;
    }
}

/**
 * @brief Memory usage growth during continuous logging
 */
void benchmarkMemoryGrowth() {
    printHeader("Memory Usage Growth");
    
    const char* testFile = "/tmp/lap_benchmark_memory.log";
    ::unlink(testFile);
    
    SinkManager manager;
    auto sink = std::make_unique<FileSink>(testFile, 100 * 1024 * 1024, 1, LogLevel::kVerbose);
    manager.addSink(std::move(sink));
    
    const String message = "Memory growth test message";
    const std::vector<int> checkpoints = {1000, 5000, 10000};
    
    size_t baseline = getCurrentMemoryUsage();
    auto coreBaseline = Memory::getMemoryStats();
    std::cout << "Baseline memory (proc): " << baseline << " KB" << std::endl;
    std::cout << "Baseline memory (core): " << (coreBaseline.totalPoolMemory / 1024) << " KB"
              << " (pools: " << coreBaseline.poolCount 
              << ", allocated: " << (coreBaseline.currentAllocSize / 1024) << " KB)" << std::endl;
    std::cout << "\n" << std::left;
    std::cout << std::setw(20) << "Logs Written"
              << std::setw(20) << "Memory (KB)"
              << std::setw(20) << "Delta (KB)"
              << std::setw(20) << "Per Log (bytes)" << std::endl;
    std::cout << std::string(70, '-') << std::endl;
    
    int count = 0;
    size_t lastMemory = baseline;
    
    for (int checkpoint : checkpoints) {
        while (count < checkpoint) {
            auto* entry = createLogEntry(static_cast<lap::log::LogLevelType>(0x04), "MEM", message.c_str());
            manager.write(*entry);
            ::operator delete(entry);
            ++count;
        }
        
        manager.flushAll();
        
        size_t currentMemory = getCurrentMemoryUsage();
        auto coreStats = Memory::getMemoryStats();
        size_t delta = currentMemory - lastMemory;
        double perLog = (delta * 1024.0) / (checkpoint - (count - checkpoint));
        
        std::cout << std::setw(20) << count
                  << std::setw(20) << currentMemory
                  << std::setw(20) << delta
                  << std::setw(20) << std::fixed << std::setprecision(2) << perLog;
        std::cout << "    Core: " << (coreStats.currentAllocSize / 1024) << " KB"
                  << " (count: " << coreStats.currentAllocCount << ")" << std::endl;
        
        lastMemory = currentMemory;
    }
    
    ::unlink(testFile);
}

/**
 * @brief Memory leak detection
 */
void benchmarkMemoryLeak() {
    printHeader("Memory Leak Detection");
    
    const char* testFile = "/tmp/lap_benchmark_leak.log";
    const int NUM_ITERATIONS = 10;
    const int LOGS_PER_ITERATION = 10000;
    const String message = "Memory leak test message";
    
    std::cout << "Running " << NUM_ITERATIONS << " iterations of " 
              << LOGS_PER_ITERATION << " logs each..." << std::endl;
    std::cout << "\n" << std::left;
    std::cout << std::setw(15) << "Iteration"
              << std::setw(20) << "Memory (KB)"
              << std::setw(20) << "Delta (KB)" << std::endl;
    std::cout << std::string(55, '-') << std::endl;
    
    size_t baseline = getCurrentMemoryUsage();
    auto coreBaseline = Memory::getMemoryStats();
    std::cout << std::setw(15) << "Baseline"
              << std::setw(20) << baseline
              << std::setw(20) << "-"
              << "    Core: " << (coreBaseline.currentAllocSize / 1024) << " KB"
              << " (count: " << coreBaseline.currentAllocCount << ")" << std::endl;
    
    size_t lastMemory = baseline;
    
    for (int iter = 1; iter <= NUM_ITERATIONS; ++iter) {
        ::unlink(testFile);
        
        {
            SinkManager manager;
            auto sink = std::make_unique<FileSink>(testFile, 50 * 1024 * 1024, 1, LogLevel::kVerbose);
            manager.addSink(std::move(sink));
            
            for (int i = 0; i < LOGS_PER_ITERATION; ++i) {
                auto* entry = createLogEntry(static_cast<lap::log::LogLevelType>(0x04), "LEAK", message.c_str());
                manager.write(*entry);
                ::operator delete(entry);
            }
            
            manager.flushAll();
        }
        // SinkManager and FileSink destroyed here
        
        // Force cleanup
        std::this_thread::sleep_for(milliseconds(100));
        
        size_t currentMemory = getCurrentMemoryUsage();
        auto coreStats = Memory::getMemoryStats();
        int delta = static_cast<int>(currentMemory) - static_cast<int>(lastMemory);
        
        std::cout << std::setw(15) << iter
                  << std::setw(20) << currentMemory
                  << std::setw(20) << delta
                  << "    Core: " << (coreStats.currentAllocSize / 1024) << " KB"
                  << " (count: " << coreStats.currentAllocCount << ")" << std::endl;
        
        lastMemory = currentMemory;
    }
    
    ::unlink(testFile);
    
    size_t final = getCurrentMemoryUsage();
    auto coreFinal = Memory::getMemoryStats();
    int totalDelta = static_cast<int>(final) - static_cast<int>(baseline);
    
    std::cout << "\nTotal memory delta (proc): " << totalDelta << " KB" << std::endl;
    std::cout << "Core final alloc: " << (coreFinal.currentAllocSize / 1024) << " KB"
              << " (count: " << coreFinal.currentAllocCount << ")" << std::endl;
    if (totalDelta < 1000) {
        std::cout << "✓ No significant memory leak detected" << std::endl;
    } else {
        std::cout << "⚠ Possible memory leak: " << totalDelta << " KB growth" << std::endl;
    }
}

/**
 * @brief Peak memory usage during burst logging
 */
void benchmarkPeakMemory() {
    printHeader("Peak Memory Usage");
    
    const char* testFile = "/tmp/lap_benchmark_peak.log";
    ::unlink(testFile);
    
    SinkManager manager;
    auto sink = std::make_unique<FileSink>(testFile, 100 * 1024 * 1024, 1, LogLevel::kVerbose);
    manager.addSink(std::move(sink));
    
    const String message = "Peak memory test with moderate message length";
    const std::vector<int> burstSizes = {1000, 5000, 10000};
    
    std::cout << std::left;
    std::cout << std::setw(20) << "Burst Size"
              << std::setw(20) << "Before (KB)"
              << std::setw(20) << "Peak (KB)"
              << std::setw(20) << "After (KB)" << std::endl;
    std::cout << std::string(70, '-') << std::endl;
    
    for (int burstSize : burstSizes) {
        size_t beforeMemory = getCurrentMemoryUsage();
        size_t peakMemory = beforeMemory;
        
        // Generate burst
        for (int i = 0; i < burstSize; ++i) {
            auto* entry = createLogEntry(static_cast<lap::log::LogLevelType>(0x04), "PEAK", message.c_str());
            manager.write(*entry);
            ::operator delete(entry);
            
            if (i % 1000 == 0) {
                size_t currentMemory = getCurrentMemoryUsage();
                if (currentMemory > peakMemory) {
                    peakMemory = currentMemory;
                }
            }
        }
        
        manager.flushAll();
        
        size_t afterMemory = getCurrentMemoryUsage();
        auto coreAfterStats = Memory::getMemoryStats();
        
        std::cout << std::setw(20) << burstSize
                  << std::setw(20) << beforeMemory
                  << std::setw(20) << peakMemory
                  << std::setw(20) << afterMemory
                  << "    Core: " << (coreAfterStats.currentAllocSize / 1024) << " KB" << std::endl;
    }
    
    ::unlink(testFile);
}

/**
 * @brief Core memory tracking stats
 */
void benchmarkCoreMemoryTracking() {
    printHeader("Core Memory Tracking");
    
    std::cout << "Note: Memory tracking stats feature not yet available in Core module" << std::endl;
    std::cout << "TODO: Implement Memory::getMemoryStats() API" << std::endl;
    
    // const char* testFile = "/tmp/lap_benchmark_tracking.log";
    // ::unlink(testFile);
    // 
    // // Get initial stats
    // auto initialStats = Memory::getMemoryStats();
    // std::cout << "Initial Core Memory Stats:" << std::endl;
    // std::cout << "  Total allocations: " << initialStats.totalAllocations << std::endl;
    // 
    // ... (rest commented out until getMemoryStats() is implemented)
}

int main() {
    // Initialize Core module
    auto initResult = Initialize();
    if (!initResult.HasValue()) {
        return 1;
    }
    
    std::cout << "\n╔════════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║          LightAP Log System - Memory Benchmark                ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════════════╝" << std::endl;
    
    try {
        benchmarkLogEntrySize();
        benchmarkMemoryGrowth();
        benchmarkMemoryLeak();
        benchmarkPeakMemory();
        benchmarkCoreMemoryTracking();
        
        std::cout << "\n" << std::string(70, '=') << std::endl;
        std::cout << "  Memory benchmark completed!" << std::endl;
        std::cout << std::string(70, '=') << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Benchmark failed: " << e.what() << std::endl;
        Deinitialize();
        return 1;
    }
    
    // Deinitialize Core module
    Deinitialize();
    
    return 0;
}

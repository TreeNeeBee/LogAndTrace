/**
 * @file        benchmark_latency.cpp
 * @author      ddkv587 ( ddkv587@gmail.com )
 * @brief       Latency benchmark for log system
 * @date        2025-10-28
 * 
 * @details     Tests:
 *              - Single log write latency
 *              - Percentile latency (P50, P90, P99, P99.9)
 *              - Latency under load
 *              - Worst-case latency
 */

#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <numeric>
#include <cmath>
#include "CSinkManager.hpp"
#include "CFileSink.hpp"
#include "CConsoleSink.hpp"
#include "CSyslogSink.hpp"
#include <lap/core/CInitialization.hpp>

using namespace lap::log;
using namespace lap::core;
using namespace std::chrono;

/**
 * @brief Helper function to create a log entry
 */
static LogEntry* createLogEntry(
    lap::log::LogLevelType level,
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

void printLatencyStats(const String& name, const std::vector<double>& latencies) {
    if (latencies.empty()) return;
    
    std::vector<double> sorted = latencies;
    std::sort(sorted.begin(), sorted.end());
    
    double sum = std::accumulate(sorted.begin(), sorted.end(), 0.0);
    double mean = sum / sorted.size();
    
    double variance = 0.0;
    for (double lat : sorted) {
        variance += (lat - mean) * (lat - mean);
    }
    variance /= sorted.size();
    double stddev = std::sqrt(variance);
    
    size_t size = sorted.size();
    double p50 = sorted[size * 50 / 100];
    double p90 = sorted[size * 90 / 100];
    double p99 = sorted[size * 99 / 100];
    double p999 = sorted[size * 999 / 1000];
    double min = sorted.front();
    double max = sorted.back();
    
    std::cout << "\n" << name << ":" << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "  Count:  " << size << " samples" << std::endl;
    std::cout << "  Mean:   " << mean << " μs" << std::endl;
    std::cout << "  Stddev: " << stddev << " μs" << std::endl;
    std::cout << "  Min:    " << min << " μs" << std::endl;
    std::cout << "  P50:    " << p50 << " μs" << std::endl;
    std::cout << "  P90:    " << p90 << " μs" << std::endl;
    std::cout << "  P99:    " << p99 << " μs" << std::endl;
    std::cout << "  P99.9:  " << p999 << " μs" << std::endl;
    std::cout << "  Max:    " << max << " μs" << std::endl;
}

/**
 * @brief Measure single log write latency
 */
void benchmarkSingleLogLatency() {
    printHeader("Single Log Write Latency");
    
    const char* testFile = "/tmp/lap_benchmark_latency.log";
    ::unlink(testFile);
    
    SinkManager manager;
    auto sink = std::make_unique<FileSink>(testFile, 100 * 1024 * 1024, 1, LogLevel::kVerbose);
    manager.addSink(std::move(sink));
    
    const int NUM_SAMPLES = 10000;
    const String message = "Latency test message";
    std::vector<double> latencies;
    latencies.reserve(NUM_SAMPLES);
    
    // Warmup
    for (int i = 0; i < 100; ++i) {
        auto* entry = createLogEntry(static_cast<lap::log::LogLevelType>(0x04), "WARM", message.c_str());
        manager.write(*entry);
        ::operator delete(entry);
    }
    manager.flushAll();
    
    // Measure
    for (int i = 0; i < NUM_SAMPLES; ++i) {
        auto* entry = createLogEntry(static_cast<lap::log::LogLevelType>(0x04), "LAT", message.c_str());
        
        auto start = high_resolution_clock::now();
        manager.write(*entry);
        auto end = high_resolution_clock::now();
        
        ::operator delete(entry);
        
        double latencyUs = duration_cast<nanoseconds>(end - start).count() / 1000.0;
        latencies.push_back(latencyUs);
    }
    
    manager.flushAll();
    
    printLatencyStats("File Sink Write Latency", latencies);
    
    ::unlink(testFile);
}

/**
 * @brief Measure latency with flush
 */
void benchmarkLatencyWithFlush() {
    printHeader("Latency with Flush");
    
    const char* testFile = "/tmp/lap_benchmark_flush_latency.log";
    ::unlink(testFile);
    
    SinkManager manager;
    auto sink = std::make_unique<FileSink>(testFile, 100 * 1024 * 1024, 1, LogLevel::kVerbose);
    manager.addSink(std::move(sink));
    
    const int NUM_SAMPLES = 1000;
    const String message = "Flush latency test message";
    std::vector<double> latencies;
    latencies.reserve(NUM_SAMPLES);
    
    for (int i = 0; i < NUM_SAMPLES; ++i) {
        auto* entry = createLogEntry(static_cast<lap::log::LogLevelType>(0x04), "FLUSH", message.c_str());
        
        auto start = high_resolution_clock::now();
        manager.write(*entry);
        manager.flushAll();
        auto end = high_resolution_clock::now();
        
        ::operator delete(entry);
        
        double latencyUs = duration_cast<nanoseconds>(end - start).count() / 1000.0;
        latencies.push_back(latencyUs);
    }
    
    printLatencyStats("Write + Flush Latency", latencies);
    
    ::unlink(testFile);
}

/**
 * @brief Compare latency across different sink types
 */
void benchmarkSinkLatencyComparison() {
    printHeader("Sink Type Latency Comparison");
    
    const int NUM_SAMPLES = 5000;
    const String message = "Sink comparison message";
    
    // File Sink
    {
        const char* testFile = "/tmp/lap_benchmark_file_lat.log";
        ::unlink(testFile);
        
        SinkManager manager;
        auto sink = std::make_unique<FileSink>(testFile, 100 * 1024 * 1024, 1, LogLevel::kVerbose);
        manager.addSink(std::move(sink));
        
        std::vector<double> latencies;
        latencies.reserve(NUM_SAMPLES);
        
        for (int i = 0; i < NUM_SAMPLES; ++i) {
            auto* entry = createLogEntry(static_cast<lap::log::LogLevelType>(0x04), "FILE", message.c_str());
            
            auto start = high_resolution_clock::now();
            manager.write(*entry);
            auto end = high_resolution_clock::now();
            
            ::operator delete(entry);
            
            double latencyUs = duration_cast<nanoseconds>(end - start).count() / 1000.0;
            latencies.push_back(latencyUs);
        }
        
        manager.flushAll();
        printLatencyStats("File Sink", latencies);
        
        ::unlink(testFile);
    }
    
    // Console Sink (disabled)
    {
        SinkManager manager;
        auto sink = std::make_unique<ConsoleSink>(false, LogLevel::kVerbose);
        manager.addSink(std::move(sink));
        
        std::vector<double> latencies;
        latencies.reserve(NUM_SAMPLES);
        
        for (int i = 0; i < NUM_SAMPLES; ++i) {
            auto* entry = createLogEntry(static_cast<lap::log::LogLevelType>(0x04), "CON", message.c_str());
            
            auto start = high_resolution_clock::now();
            manager.write(*entry);
            auto end = high_resolution_clock::now();
            
            ::operator delete(entry);
            
            double latencyUs = duration_cast<nanoseconds>(end - start).count() / 1000.0;
            latencies.push_back(latencyUs);
        }
        
        printLatencyStats("Console Sink (disabled)", latencies);
    }
    
    // Syslog Sink
    {
        SinkManager manager;
        auto sink = std::make_unique<SyslogSink>("LAPLatency", LOG_USER, LogLevel::kVerbose);
        manager.addSink(std::move(sink));
        
        std::vector<double> latencies;
        latencies.reserve(NUM_SAMPLES);
        
        for (int i = 0; i < NUM_SAMPLES; ++i) {
            auto* entry = createLogEntry(static_cast<lap::log::LogLevelType>(0x04), "SYS", message.c_str());
            
            auto start = high_resolution_clock::now();
            manager.write(*entry);
            auto end = high_resolution_clock::now();
            
            ::operator delete(entry);
            
            double latencyUs = duration_cast<nanoseconds>(end - start).count() / 1000.0;
            latencies.push_back(latencyUs);
        }
        
        printLatencyStats("Syslog Sink", latencies);
    }
}

/**
 * @brief Latency under heavy load
 */
void benchmarkLatencyUnderLoad() {
    printHeader("Latency Under Load");
    
    const char* testFile = "/tmp/lap_benchmark_load_latency.log";
    ::unlink(testFile);
    
    SinkManager manager;
    auto sink = std::make_unique<FileSink>(testFile, 100 * 1024 * 1024, 1, LogLevel::kVerbose);
    manager.addSink(std::move(sink));
    
    const int NUM_SAMPLES = 10000;
    const String message = "Load test message with moderate length to simulate real usage";
    std::vector<double> latencies;
    latencies.reserve(NUM_SAMPLES);
    
    std::cout << "Generating load with " << NUM_SAMPLES << " logs..." << std::endl;
    
    for (int i = 0; i < NUM_SAMPLES; ++i) {
        auto* entry = createLogEntry(static_cast<lap::log::LogLevelType>(0x04), "LOAD", message.c_str());
        
        auto start = high_resolution_clock::now();
        manager.write(*entry);
        auto end = high_resolution_clock::now();
        
        ::operator delete(entry);
        
        double latencyUs = duration_cast<nanoseconds>(end - start).count() / 1000.0;
        latencies.push_back(latencyUs);
        
        // No delay - continuous load
    }
    
    manager.flushAll();
    
    printLatencyStats("Continuous Load", latencies);
    
    ::unlink(testFile);
}

int main() {
    // Initialize Core module
    auto initResult = Initialize();
    if (!initResult.HasValue()) {
        return 1;
    }
    
    std::cout << "\n╔════════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║           LightAP Log System - Latency Benchmark              ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════════════╝" << std::endl;
    
    try {
        benchmarkSingleLogLatency();
        benchmarkLatencyWithFlush();
        benchmarkSinkLatencyComparison();
        benchmarkLatencyUnderLoad();
        
        std::cout << "\n" << std::string(70, '=') << std::endl;
        std::cout << "  Latency benchmark completed!" << std::endl;
        std::cout << "  Note: Latencies in microseconds (μs)" << std::endl;
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

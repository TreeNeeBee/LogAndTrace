/**
 * @file        benchmark_simple.cpp
 * @brief       Simplified benchmark for logging performance
 * @date        2025-10-29
 */

#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>
#include <thread>
#include <atomic>
#include <CLog.hpp>
#include <core/CMemory.hpp>

using namespace lap::log;
using namespace lap::core;
using namespace std::chrono;

// Statistics calculation
void printStats(const std::vector<double>& data, const char* name) {
    if (data.empty()) return;
    
    auto sorted = data;
    std::sort(sorted.begin(), sorted.end());
    
    double sum = 0;
    for (auto v : sorted) sum += v;
    double mean = sum / sorted.size();
    
    double p50 = sorted[sorted.size() * 50 / 100];
    double p95 = sorted[sorted.size() * 95 / 100];
    double p99 = sorted[sorted.size() * 99 / 100];
    double min = sorted.front();
    double max = sorted.back();
    
    std::cout << "\n" << name << " Statistics:" << std::endl;
    std::cout << "  Mean:   " << mean << " µs" << std::endl;
    std::cout << "  P50:    " << p50 << " µs" << std::endl;
    std::cout << "  P95:    " << p95 << " µs" << std::endl;
    std::cout << "  P99:    " << p99 << " µs" << std::endl;
    std::cout << "  Min:    " << min << " µs" << std::endl;
    std::cout << "  Max:    " << max << " µs" << std::endl;
}

/**
 * @brief Benchmark single-threaded throughput
 */
void benchmarkThroughput() {
    std::cout << "\n=== Benchmark: Single-Thread Throughput ===" << std::endl;
    
    const int NUM_LOGS = 100000;
    auto& logger = CreateLogger("THRU", "Throughput Test");
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < NUM_LOGS; ++i) {
        logger.LogInfo() << "Log message #" << i;
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();
    
    double throughput = (NUM_LOGS * 1000.0) / duration;
    
    std::cout << "  Total logs:   " << NUM_LOGS << std::endl;
    std::cout << "  Duration:     " << duration << " ms" << std::endl;
    std::cout << "  Throughput:   " << static_cast<int>(throughput) << " logs/sec" << std::endl;
}

/**
 * @brief Benchmark multi-threaded throughput
 */
void benchmarkMultiThreadThroughput() {
    std::cout << "\n=== Benchmark: Multi-Thread Throughput ===" << std::endl;
    
    const int NUM_THREADS = 10;
    const int LOGS_PER_THREAD = 10000;
    auto& logger = CreateLogger("MTHR", "Multi-Thread Test");
    
    std::atomic<int> ready{0};
    std::atomic<bool> start{false};
    
    auto start_time = high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([&, t]() {
            ready++;
            while (!start.load()) {
                std::this_thread::yield();
            }
            
            for (int i = 0; i < LOGS_PER_THREAD; ++i) {
                logger.LogInfo() << "Thread " << t << " log #" << i;
            }
        });
    }
    
    // Wait for all threads to be ready
    while (ready.load() < NUM_THREADS) {
        std::this_thread::yield();
    }
    
    start = true;
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end_time = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end_time - start_time).count();
    
    int total_logs = NUM_THREADS * LOGS_PER_THREAD;
    double throughput = (total_logs * 1000.0) / duration;
    
    std::cout << "  Threads:      " << NUM_THREADS << std::endl;
    std::cout << "  Total logs:   " << total_logs << std::endl;
    std::cout << "  Duration:     " << duration << " ms" << std::endl;
    std::cout << "  Throughput:   " << static_cast<int>(throughput) << " logs/sec" << std::endl;
}

/**
 * @brief Benchmark latency distribution
 */
void benchmarkLatency() {
    std::cout << "\n=== Benchmark: Latency Distribution ===" << std::endl;
    
    const int NUM_SAMPLES = 10000;
    auto& logger = CreateLogger("LAT", "Latency Test");
    std::vector<double> latencies;
    latencies.reserve(NUM_SAMPLES);
    
    for (int i = 0; i < NUM_SAMPLES; ++i) {
        auto start = high_resolution_clock::now();
        logger.LogInfo() << "Latency test message #" << i;
        auto end = high_resolution_clock::now();
        
        double latency_us = duration_cast<nanoseconds>(end - start).count() / 1000.0;
        latencies.push_back(latency_us);
    }
    
    printStats(latencies, "Latency");
}

/**
 * @brief Benchmark memory usage
 */
void benchmarkMemory() {
    std::cout << "\n=== Benchmark: Memory Usage ===" << std::endl;
    
    auto stats_before = Memory::getMemoryStats();
    
    std::cout << "\nBefore benchmark:" << std::endl;
    std::cout << "  Alloc count:    " << stats_before.currentAllocCount << std::endl;
    std::cout << "  Alloc size:     " << stats_before.currentAllocSize << " bytes" << std::endl;
    std::cout << "  Pool memory:    " << stats_before.totalPoolMemory << " bytes" << std::endl;
    std::cout << "  Pool count:     " << stats_before.poolCount << std::endl;
    
    const int NUM_LOGS = 50000;
    auto& logger = CreateLogger("MEM", "Memory Test");
    
    for (int i = 0; i < NUM_LOGS; ++i) {
        logger.LogInfo() << "Memory test #" << i;
    }
    
    auto stats_after = Memory::getMemoryStats();
    
    std::cout << "\nAfter " << NUM_LOGS << " logs:" << std::endl;
    std::cout << "  Alloc count:    " << stats_after.currentAllocCount << std::endl;
    std::cout << "  Alloc size:     " << stats_after.currentAllocSize << " bytes" << std::endl;
    std::cout << "  Pool memory:    " << stats_after.totalPoolMemory << " bytes" << std::endl;
    std::cout << "  Pool count:     " << stats_after.poolCount << std::endl;
    std::cout << "  Memory growth:  " << (stats_after.currentAllocSize - stats_before.currentAllocSize) << " bytes" << std::endl;
}

int main(int argc, char* argv[]) {
    // Initialize memory manager first
    MemManager::getInstance();
    
    // Initialize logging
    LogManager::getInstance().initialize();
    
    std::cout << "==============================================\n";
    std::cout << "  LightAP Logging Performance Benchmark\n";
    std::cout << "==============================================" << std::endl;
    
    if (argc > 1) {
        std::string benchmark = argv[1];
        if (benchmark == "throughput") {
            benchmarkThroughput();
        } else if (benchmark == "multi") {
            benchmarkMultiThreadThroughput();
        } else if (benchmark == "latency") {
            benchmarkLatency();
        } else if (benchmark == "memory") {
            benchmarkMemory();
        } else {
            std::cout << "Unknown benchmark: " << benchmark << std::endl;
            std::cout << "Available: throughput, multi, latency, memory, all" << std::endl;
            return 1;
        }
    } else {
        // Run all benchmarks
        benchmarkThroughput();
        benchmarkMultiThreadThroughput();
        benchmarkLatency();
        benchmarkMemory();
    }
    
    std::cout << "\n==============================================\n";
    std::cout << "  Benchmark completed successfully!\n";
    std::cout << "==============================================" << std::endl;
    
    return 0;
}

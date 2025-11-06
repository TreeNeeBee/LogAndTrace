#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <atomic>
#include <iomanip>
#include "CLogManager.hpp"
#include "CLogger.hpp"
#include <lap/core/CMemory.hpp>

using namespace lap::log;
using namespace lap::core;

// 内存统计辅助函数
void printMemoryStats(const std::string& phase) {
    auto stats = Memory::getMemoryStats();
    std::cout << "\n[" << phase << "] Memory Statistics:\n"
              << "  Current allocations: " << stats.currentAllocCount << " blocks\n"
              << "  Current memory: " << stats.currentAllocSize << " bytes ("
              << (stats.currentAllocSize / 1024.0) << " KB)\n"
              << "  Total pool memory: " << stats.totalPoolMemory << " bytes ("
              << (stats.totalPoolMemory / 1024.0) << " KB)\n"
              << "  Pool count: " << stats.poolCount << "\n"
              << "  Thread count: " << stats.threadCount << "\n"
              << std::endl;
}

// 测试场景1：单线程大批量日志 (10K)
void benchmark_single_thread_10k() {
    std::cout << "\n========== Benchmark: Single Thread 10K Logs ==========\n";
    
    printMemoryStats("Before");
    
    LogManager::getInstance().initialize();
    auto& logger = LogManager::getInstance().registerLogger("BENCH", "Benchmark", LogLevel::kInfo);
    
    printMemoryStats("After Logger Init");
    
    const int numLogs = 10000;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < numLogs; ++i) {
        logger.LogInfo() << "Single thread benchmark log #" << i;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    printMemoryStats("After Logging");
    
    double logsPerSecond = (numLogs * 1000.0) / duration.count();
    std::cout << "Results: " << numLogs << " logs in " << duration.count() << "ms\n"
              << "Throughput: " << std::fixed << std::setprecision(0) << logsPerSecond << " logs/sec\n"
              << "Average: " << std::fixed << std::setprecision(3) 
              << (duration.count() * 1000.0 / numLogs) << " μs/log\n";
    
    LogManager::getInstance().uninitialize();
    printMemoryStats("After Cleanup");
}

// 测试场景2：单线程超大批量 (100K)
void benchmark_single_thread_100k() {
    std::cout << "\n========== Benchmark: Single Thread 100K Logs ==========\n";
    
    printMemoryStats("Before");
    
    LogManager::getInstance().initialize();
    auto& logger = LogManager::getInstance().registerLogger("BENCH", "Benchmark", LogLevel::kInfo);
    
    printMemoryStats("After Logger Init");
    
    const int numLogs = 100000;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < numLogs; ++i) {
        logger.LogInfo() << "Single thread 100K benchmark log #" << i;
        
        // 每10K打印一次内存状态
        if (i > 0 && i % 10000 == 0) {
            std::cout << "Progress: " << i << " logs..." << std::endl;
            printMemoryStats("During Logging");
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    printMemoryStats("After Logging");
    
    double logsPerSecond = (numLogs * 1000.0) / duration.count();
    std::cout << "Results: " << numLogs << " logs in " << duration.count() << "ms\n"
              << "Throughput: " << std::fixed << std::setprecision(0) << logsPerSecond << " logs/sec\n"
              << "Average: " << std::fixed << std::setprecision(3) 
              << (duration.count() * 1000.0 / numLogs) << " μs/log\n";
    
    LogManager::getInstance().uninitialize();
    printMemoryStats("After Cleanup");
}

// 测试场景3：多线程并发 (10 threads × 10K = 100K total)
void benchmark_multi_thread_100k() {
    std::cout << "\n========== Benchmark: Multi-Thread 100K Logs (10×10K) ==========\n";
    
    printMemoryStats("Before");
    
    LogManager::getInstance().initialize();
    auto& logger = LogManager::getInstance().registerLogger("BENCH", "Benchmark", LogLevel::kInfo);
    
    printMemoryStats("After Logger Init");
    
    const int numThreads = 10;
    const int logsPerThread = 10000;
    const int totalLogs = numThreads * logsPerThread;
    
    std::atomic<int> completedThreads{0};
    std::vector<std::thread> threads;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([&logger, t, logsPerThread, &completedThreads]() {
            for (int i = 0; i < logsPerThread; ++i) {
                logger.LogInfo() << "Thread " << t << " log #" << i;
            }
            completedThreads++;
            std::cout << "Thread " << t << " completed" << std::endl;
        });
    }
    
    // 监控线程完成情况
    while (completedThreads < numThreads) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        std::cout << "Progress: " << completedThreads << "/" << numThreads 
                  << " threads completed" << std::endl;
        printMemoryStats("During Logging");
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    printMemoryStats("After Logging");
    
    double logsPerSecond = (totalLogs * 1000.0) / duration.count();
    std::cout << "Results: " << totalLogs << " logs from " << numThreads 
              << " threads in " << duration.count() << "ms\n"
              << "Throughput: " << std::fixed << std::setprecision(0) << logsPerSecond << " logs/sec\n"
              << "Average: " << std::fixed << std::setprecision(3) 
              << (duration.count() * 1000.0 / totalLogs) << " μs/log\n";
    
    LogManager::getInstance().uninitialize();
    printMemoryStats("After Cleanup");
}

// 测试场景4：高并发多线程 (50 threads × 2K = 100K total)
void benchmark_high_concurrency() {
    std::cout << "\n========== Benchmark: High Concurrency (50×2K) ==========\n";
    
    printMemoryStats("Before");
    
    LogManager::getInstance().initialize();
    auto& logger = LogManager::getInstance().registerLogger("BENCH", "Benchmark", LogLevel::kInfo);
    
    printMemoryStats("After Logger Init");
    
    const int numThreads = 50;
    const int logsPerThread = 2000;
    const int totalLogs = numThreads * logsPerThread;
    
    std::atomic<int> completedThreads{0};
    std::vector<std::thread> threads;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([&logger, t, logsPerThread, &completedThreads]() {
            for (int i = 0; i < logsPerThread; ++i) {
                logger.LogInfo() << "High concurrency thread " << t << " log " << i;
            }
            completedThreads++;
        });
    }
    
    // 监控进度
    int lastReported = 0;
    while (completedThreads < numThreads) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        int current = completedThreads.load();
        if (current > lastReported) {
            std::cout << "Progress: " << current << "/" << numThreads 
                      << " threads completed" << std::endl;
            lastReported = current;
        }
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    printMemoryStats("After Logging");
    
    double logsPerSecond = (totalLogs * 1000.0) / duration.count();
    std::cout << "Results: " << totalLogs << " logs from " << numThreads 
              << " threads in " << duration.count() << "ms\n"
              << "Throughput: " << std::fixed << std::setprecision(0) << logsPerSecond << " logs/sec\n"
              << "Average: " << std::fixed << std::setprecision(3) 
              << (duration.count() * 1000.0 / totalLogs) << " μs/log\n";
    
    LogManager::getInstance().uninitialize();
    printMemoryStats("After Cleanup");
}

// 测试场景5：持续压力测试
void benchmark_sustained_load() {
    std::cout << "\n========== Benchmark: Sustained Load (10 seconds) ==========\n";
    
    printMemoryStats("Before");
    
    LogManager::getInstance().initialize();
    auto& logger = LogManager::getInstance().registerLogger("BENCH", "Benchmark", LogLevel::kInfo);
    
    printMemoryStats("After Logger Init");
    
    const int numThreads = 10;
    const int durationSeconds = 10;
    std::atomic<bool> stopFlag{false};
    std::atomic<long long> totalLogs{0};
    
    std::vector<std::thread> threads;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([&logger, t, &stopFlag, &totalLogs]() {
            int count = 0;
            while (!stopFlag.load()) {
                logger.LogInfo() << "Sustained load thread " << t << " log " << count++;
                totalLogs++;
            }
        });
    }
    
    // 每秒报告一次状态
    for (int sec = 1; sec <= durationSeconds; ++sec) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        long long currentLogs = totalLogs.load();
        std::cout << "After " << sec << "s: " << currentLogs << " logs" << std::endl;
        if (sec % 2 == 0) {
            printMemoryStats("During Load");
        }
    }
    
    stopFlag = true;
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    printMemoryStats("After Logging");
    
    long long finalLogs = totalLogs.load();
    double logsPerSecond = (finalLogs * 1000.0) / duration.count();
    
    std::cout << "Results: " << finalLogs << " logs in " << duration.count() << "ms\n"
              << "Throughput: " << std::fixed << std::setprecision(0) << logsPerSecond << " logs/sec\n"
              << "Average: " << std::fixed << std::setprecision(3) 
              << (duration.count() * 1000000.0 / finalLogs) << " ns/log\n";
    
    LogManager::getInstance().uninitialize();
    printMemoryStats("After Cleanup");
}

int main(int argc, char** argv) {
    std::cout << "========================================\n"
              << "Log System Stress Test & Memory Monitor\n"
              << "========================================\n";
    
    // 首先初始化 MemManager 确保正确的析构顺序
    MemManager::getInstance();
    
    printMemoryStats("Initial");
    
    // 选择要运行的测试
    if (argc > 1) {
        std::string test = argv[1];
        if (test == "10k") {
            benchmark_single_thread_10k();
        } else if (test == "100k") {
            benchmark_single_thread_100k();
        } else if (test == "multi") {
            benchmark_multi_thread_100k();
        } else if (test == "concurrent") {
            benchmark_high_concurrency();
        } else if (test == "sustained") {
            benchmark_sustained_load();
        } else if (test == "all") {
            benchmark_single_thread_10k();
            benchmark_single_thread_100k();
            benchmark_multi_thread_100k();
            benchmark_high_concurrency();
            benchmark_sustained_load();
        } else {
            std::cerr << "Usage: " << argv[0] << " [10k|100k|multi|concurrent|sustained|all]\n";
            return 1;
        }
    } else {
        // 默认运行所有测试
        benchmark_single_thread_10k();
        benchmark_single_thread_100k();
        benchmark_multi_thread_100k();
        benchmark_high_concurrency();
        benchmark_sustained_load();
    }
    
    printMemoryStats("Final");
    
    std::cout << "\n========================================\n"
              << "All benchmarks completed!\n"
              << "========================================\n";
    
    return 0;
}

#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include "CLogManager.hpp"
#include "CLogger.hpp"

using namespace lap::log;
using namespace lap::core;

class MultiThreadTest : public ::testing::Test {
protected:
    void SetUp() override {
        LogManager::getInstance().initialize();
        logger_ = &LogManager::getInstance().registerLogger("MTST", "MultiThreadTest", LogLevel::kInfo);
    }

    void TearDown() override {
        LogManager::getInstance().uninitialize();
    }

    Logger* logger_;
};

// 测试：多线程并发写入日志
TEST_F(MultiThreadTest, ConcurrentLogging) {
    const int numThreads = 10;
    const int logsPerThread = 1000;
    std::atomic<int> completedThreads{0};
    
    std::vector<std::thread> threads;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([this, t, logsPerThread, &completedThreads]() {
            for (int i = 0; i < logsPerThread; ++i) {
                logger_->LogInfo() << "Thread " << t << " log " << i;
            }
            completedThreads++;
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_EQ(completedThreads, numThreads);
    
    int totalLogs = numThreads * logsPerThread;
    double logsPerSecond = (totalLogs * 1000.0) / duration.count();
    
    std::cout << "Multi-thread test: " << totalLogs << " logs in " 
              << duration.count() << "ms (" 
              << static_cast<int>(logsPerSecond) << " logs/sec, "
              << numThreads << " threads)" << std::endl;
}

// 测试：大量线程短时间高并发
TEST_F(MultiThreadTest, HighConcurrency) {
    const int numThreads = 50;
    const int logsPerThread = 100;
    
    std::vector<std::thread> threads;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([this, t, logsPerThread]() {
            for (int i = 0; i < logsPerThread; ++i) {
                logger_->LogInfo() << "High concurrency test from thread " << t;
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    int totalLogs = numThreads * logsPerThread;
    std::cout << "High concurrency: " << totalLogs << " logs from " 
              << numThreads << " threads in " << duration.count() << "ms" << std::endl;
}

// 测试：不同日志级别的并发
TEST_F(MultiThreadTest, MixedLevels) {
    const int numThreads = 8;
    const int logsPerThread = 500;
    
    std::vector<std::thread> threads;
    
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([this, t, logsPerThread]() {
            for (int i = 0; i < logsPerThread; ++i) {
                switch (i % 5) {
                    case 0: logger_->LogVerbose() << "Thread " << t << " verbose " << i; break;
                    case 1: logger_->LogDebug() << "Thread " << t << " debug " << i; break;
                    case 2: logger_->LogInfo() << "Thread " << t << " info " << i; break;
                    case 3: logger_->LogWarn() << "Thread " << t << " warn " << i; break;
                    case 4: logger_->LogError() << "Thread " << t << " error " << i; break;
                }
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    SUCCEED() << "Mixed level logging completed successfully";
}

// 测试：持续压力测试
TEST_F(MultiThreadTest, SustainedLoad) {
    const int numThreads = 10;
    const int durationSeconds = 3;
    std::atomic<bool> stopFlag{false};
    std::atomic<int> totalLogs{0};
    
    std::vector<std::thread> threads;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([this, t, &stopFlag, &totalLogs]() {
            int count = 0;
            while (!stopFlag.load()) {
                logger_->LogInfo() << "Sustained load thread " << t << " log " << count++;
                totalLogs++;
            }
        });
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(durationSeconds));
    stopFlag = true;
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    double logsPerSecond = (totalLogs.load() * 1000.0) / duration.count();
    
    std::cout << "Sustained load: " << totalLogs.load() << " logs in " 
              << duration.count() << "ms (" 
              << static_cast<int>(logsPerSecond) << " logs/sec)" << std::endl;
}

// 测试：多个Logger并发
TEST_F(MultiThreadTest, MultipleLoggers) {
    const int numThreads = 8;
    const int logsPerThread = 500;
    
    // 创建多个Logger
    auto& logger2 = LogManager::getInstance().registerLogger("MT2", "Logger2", LogLevel::kInfo);
    auto& logger3 = LogManager::getInstance().registerLogger("MT3", "Logger3", LogLevel::kDebug);
    
    std::vector<std::thread> threads;
    
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([this, &logger2, &logger3, t, logsPerThread]() {
            for (int i = 0; i < logsPerThread; ++i) {
                switch (i % 3) {
                    case 0: logger_->LogInfo() << "Logger1 from thread " << t; break;
                    case 1: logger2.LogInfo() << "Logger2 from thread " << t; break;
                    case 2: logger3.LogDebug() << "Logger3 from thread " << t; break;
                }
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    SUCCEED() << "Multiple loggers test completed";
}

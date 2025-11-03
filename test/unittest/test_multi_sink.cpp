/**
 * @file        test_multi_sink.cpp
 * @author      ddkv587 ( ddkv587@gmail.com )
 * @brief       Multi-sink architecture demo and test
 * @date        2025-10-28
 */

#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include "CConsoleSink.hpp"
#include "CFileSink.hpp"
#include "CSinkManager.hpp"

using namespace lap::log;
using namespace lap::core;

TEST(MultiSink, ConsoleSinkBasic) {
    ConsoleSink sink(true, LogLevel::kDebug);
    
    EXPECT_TRUE(sink.isEnabled());
    EXPECT_EQ(sink.getName(), "Console");
    EXPECT_TRUE(sink.shouldLog(LogLevel::kInfo));
    EXPECT_TRUE(sink.shouldLog(LogLevel::kDebug));
    EXPECT_FALSE(sink.shouldLog(LogLevel::kVerbose));
    
    // Write directly with sink's write method
    auto now = ::std::chrono::system_clock::now();
    UInt64 timestamp = ::std::chrono::duration_cast<::std::chrono::microseconds>(
        now.time_since_epoch()).count();
    UInt32 threadId = static_cast<UInt32>(::std::hash<::std::thread::id>{}(::std::this_thread::get_id()));
    
    sink.write(timestamp, threadId, static_cast<lap::log::LogLevelType>(0x04), "TEST", "Hello from console sink!");
    sink.flush();
}

TEST(MultiSink, FileSinkBasic) {
    const char* testFile = "/tmp/lap_test.log";
    
    // Remove old test file
    ::unlink(testFile);
    
    FileSink sink(testFile, 1024, 3, LogLevel::kVerbose);
    
    EXPECT_TRUE(sink.isEnabled());
    EXPECT_EQ(sink.getName(), "File");
    
    // Get common parameters
    auto now = ::std::chrono::system_clock::now();
    UInt64 timestamp = ::std::chrono::duration_cast<::std::chrono::microseconds>(
        now.time_since_epoch()).count();
    UInt32 threadId = static_cast<UInt32>(::std::hash<::std::thread::id>{}(::std::this_thread::get_id()));
    
    // Write multiple entries
    for (int i = 0; i < 10; ++i) {
        String msg = "Test log message #" + std::to_string(i);
        sink.write(timestamp, threadId, static_cast<lap::log::LogLevelType>(0x04), "FTEST", msg.c_str());
    }
    
    sink.flush();
    
    // Check file exists
    FILE* f = fopen(testFile, "r");
    ASSERT_NE(f, nullptr);
    
    // Read and verify content
    char buffer[256];
    int lineCount = 0;
    while (fgets(buffer, sizeof(buffer), f) != nullptr) {
        ++lineCount;
    }
    
    fclose(f);
    EXPECT_EQ(lineCount, 10);
    
    // Cleanup
    ::unlink(testFile);
}

TEST(MultiSink, FileSinkRotation) {
    const char* testFile = "/tmp/lap_test_rotate.log";
    
    // Remove old test files
    ::unlink(testFile);
    ::unlink((String(testFile) + ".1").c_str());
    ::unlink((String(testFile) + ".2").c_str());
    
    // Create sink with small max size to trigger rotation
    FileSink sink(testFile, 512, 2, LogLevel::kVerbose);
    
    // Get common parameters
    auto now = ::std::chrono::system_clock::now();
    UInt64 timestamp = ::std::chrono::duration_cast<::std::chrono::microseconds>(
        now.time_since_epoch()).count();
    UInt32 threadId = static_cast<UInt32>(::std::hash<::std::thread::id>{}(::std::this_thread::get_id()));
    
    // Write many entries to trigger rotation
    for (int i = 0; i < 50; ++i) {
        String msg = "Long test log message with padding to increase size #" + std::to_string(i);
        sink.write(timestamp, threadId, static_cast<lap::log::LogLevelType>(0x04), "ROTATE", msg.c_str());
    }
    
    sink.flush();
    
    // Check that rotation happened
    FILE* f = fopen(testFile, "r");
    ASSERT_NE(f, nullptr);
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fclose(f);
    
    // Size should be less than 1KB after rotation
    EXPECT_LT(size, 1024);
    
    // Check backup file exists
    FILE* f1 = fopen((String(testFile) + ".1").c_str(), "r");
    EXPECT_NE(f1, nullptr);
    if (f1) fclose(f1);
    
    // Cleanup
    ::unlink(testFile);
    ::unlink((String(testFile) + ".1").c_str());
    ::unlink((String(testFile) + ".2").c_str());
}

TEST(MultiSink, SinkManagerMultipleDestinations) {
    const char* testFile = "/tmp/lap_manager_test.log";
    ::unlink(testFile);
    
    // Create individual sinks and write directly since we can't use Logger in this test
    ConsoleSink consoleSink(true, LogLevel::kInfo);
    FileSink fileSink(testFile, 10 * 1024, 5, LogLevel::kDebug);
    
    // Get common parameters
    auto now = ::std::chrono::system_clock::now();
    UInt64 timestamp = ::std::chrono::duration_cast<::std::chrono::microseconds>(
        now.time_since_epoch()).count();
    UInt32 threadId = static_cast<UInt32>(::std::hash<::std::thread::id>{}(::std::this_thread::get_id()));
    
    // Write logs to both sinks
    for (int i = 0; i < 5; ++i) {
        String msg = "Multi-sink log message #" + std::to_string(i);
        consoleSink.write(timestamp, threadId, static_cast<lap::log::LogLevelType>(0x04), "MULTI", msg.c_str());
        fileSink.write(timestamp, threadId, static_cast<lap::log::LogLevelType>(0x04), "MULTI", msg.c_str());
    }
    
    consoleSink.flush();
    fileSink.flush();
    
    // Verify file has logs
    FILE* f = fopen(testFile, "r");
    ASSERT_NE(f, nullptr);
    
    char buffer[256];
    int lineCount = 0;
    while (fgets(buffer, sizeof(buffer), f) != nullptr) {
        ++lineCount;
    }
    fclose(f);
    
    EXPECT_EQ(lineCount, 5);
    
    // Cleanup
    ::unlink(testFile);
}

TEST(MultiSink, SinkManagerLevelFiltering) {
    SinkManager manager;
    
    // Add console sink with INFO level
    auto consoleSink = std::make_unique<ConsoleSink>(true, LogLevel::kInfo);
    manager.addSink(std::move(consoleSink));
    
    // shouldLog should return true for INFO and higher
    EXPECT_TRUE(manager.shouldLog(LogLevel::kFatal));
    EXPECT_TRUE(manager.shouldLog(LogLevel::kError));
    EXPECT_TRUE(manager.shouldLog(LogLevel::kWarn));
    EXPECT_TRUE(manager.shouldLog(LogLevel::kInfo));
    EXPECT_FALSE(manager.shouldLog(LogLevel::kDebug));
    EXPECT_FALSE(manager.shouldLog(LogLevel::kVerbose));
}

TEST(MultiSink, SinkManagerRemoval) {
    SinkManager manager;
    
    auto consoleSink = std::make_unique<ConsoleSink>();
    manager.addSink(std::move(consoleSink));
    
    auto fileSink = std::make_unique<FileSink>("/tmp/test.log");
    manager.addSink(std::move(fileSink));
    
    EXPECT_EQ(manager.getSinkCount(), 2u);
    
    // Remove console sink
    EXPECT_TRUE(manager.removeSink("Console"));
    EXPECT_EQ(manager.getSinkCount(), 1u);
    
    // Remove file sink
    EXPECT_TRUE(manager.removeSink("File"));
    EXPECT_EQ(manager.getSinkCount(), 0u);
    
    // Try to remove non-existent sink
    EXPECT_FALSE(manager.removeSink("NonExistent"));
}

TEST(MultiSink, PerformanceBenchmark) {
    const char* testFile = "/tmp/lap_perf_test.log";
    ::unlink(testFile);
    
    // Add file sink only for performance test
    FileSink sink(testFile, 100 * 1024 * 1024, 1, LogLevel::kVerbose);
    
    const int NUM_LOGS = 10000;
    
    // Get common parameters
    auto now = ::std::chrono::system_clock::now();
    UInt64 timestamp = ::std::chrono::duration_cast<::std::chrono::microseconds>(
        now.time_since_epoch()).count();
    UInt32 threadId = static_cast<UInt32>(::std::hash<::std::thread::id>{}(::std::this_thread::get_id()));
    
    auto start = ::std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < NUM_LOGS; ++i) {
        String msg = "Performance test message #" + std::to_string(i);
        sink.write(timestamp, threadId, static_cast<lap::log::LogLevelType>(0x04), "PERF", msg.c_str());
    }
    
    sink.flush();
    
    auto end = ::std::chrono::high_resolution_clock::now();
    auto duration = ::std::chrono::duration_cast<::std::chrono::milliseconds>(end - start);
    
    double throughput = (NUM_LOGS * 1000.0) / duration.count();
    
    std::cout << "Performance: " << NUM_LOGS << " logs in " << duration.count() << "ms"
              << " (" << static_cast<int>(throughput) << " logs/sec)" << std::endl;
    
    // Should achieve at least 10K logs/sec on modern hardware
    EXPECT_GT(throughput, 10000.0);
    
    // Cleanup
    ::unlink(testFile);
}

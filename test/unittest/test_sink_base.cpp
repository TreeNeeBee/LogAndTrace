/**
 * @file        test_sink_base.cpp
 * @author      ddkv587 ( ddkv587@gmail.com )
 * @brief       ISink interface and base functionality tests
 * @date        2025-10-28
 */

#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include "ISink.hpp"
#include "CConsoleSink.hpp"
#include "CFileSink.hpp"
#include "CSyslogSink.hpp"

using namespace lap::log;
using namespace lap::core;

TEST(SinkBase, PolymorphicBehavior) {
    // Test that all sinks can be used through ISink interface
    std::vector<std::unique_ptr<ISink>> sinks;
    
    sinks.push_back(std::make_unique<ConsoleSink>(false, LogLevel::kInfo));
    sinks.push_back(std::make_unique<FileSink>("/tmp/test_polymorphic.log", 1024, 3, LogLevel::kInfo));
    sinks.push_back(std::make_unique<SyslogSink>("TestPoly", LOG_USER, LogLevel::kInfo));
    
    EXPECT_EQ(sinks.size(), 3u);
    
    // All should have different names
    EXPECT_EQ(sinks[0]->getName(), "Console");
    EXPECT_EQ(sinks[1]->getName(), "File");
    EXPECT_EQ(sinks[2]->getName(), "Syslog");
    
    // All should be enabled by default
    for (const auto& sink : sinks) {
        EXPECT_TRUE(sink->isEnabled());
    }
    
    // All should support level filtering
    for (const auto& sink : sinks) {
        EXPECT_TRUE(sink->shouldLog(LogLevel::kInfo));
        EXPECT_FALSE(sink->shouldLog(LogLevel::kDebug));
    }
    
    // Write through polymorphic interface with direct parameters
    auto now = ::std::chrono::system_clock::now();
    UInt64 timestamp = ::std::chrono::duration_cast<::std::chrono::microseconds>(
        now.time_since_epoch()).count();
    UInt32 threadId = static_cast<UInt32>(::std::hash<::std::thread::id>{}(::std::this_thread::get_id()));
    
    for (auto& sink : sinks) {
        sink->write(timestamp, threadId, static_cast<lap::log::LogLevelType>(0x04), "POLY", "Polymorphic test message");
    }
    
    // Flush all
    for (auto& sink : sinks) {
        sink->flush();
    }
    
    ::unlink("/tmp/test_polymorphic.log");
}

TEST(SinkBase, LevelHierarchy) {
    ConsoleSink sink(false, LogLevel::kWarn);
    
    // Off level should block all
    sink.setLevel(LogLevel::kOff);
    EXPECT_FALSE(sink.shouldLog(LogLevel::kFatal));
    EXPECT_FALSE(sink.shouldLog(LogLevel::kError));
    EXPECT_FALSE(sink.shouldLog(LogLevel::kWarn));
    EXPECT_FALSE(sink.shouldLog(LogLevel::kInfo));
    
    // Fatal should only allow Fatal
    sink.setLevel(LogLevel::kFatal);
    EXPECT_TRUE(sink.shouldLog(LogLevel::kFatal));
    EXPECT_FALSE(sink.shouldLog(LogLevel::kError));
    
    // Warn should allow Fatal, Error, Warn
    sink.setLevel(LogLevel::kWarn);
    EXPECT_TRUE(sink.shouldLog(LogLevel::kFatal));
    EXPECT_TRUE(sink.shouldLog(LogLevel::kError));
    EXPECT_TRUE(sink.shouldLog(LogLevel::kWarn));
    EXPECT_FALSE(sink.shouldLog(LogLevel::kInfo));
    
    // Verbose should allow all
    sink.setLevel(LogLevel::kVerbose);
    EXPECT_TRUE(sink.shouldLog(LogLevel::kFatal));
    EXPECT_TRUE(sink.shouldLog(LogLevel::kError));
    EXPECT_TRUE(sink.shouldLog(LogLevel::kWarn));
    EXPECT_TRUE(sink.shouldLog(LogLevel::kInfo));
    EXPECT_TRUE(sink.shouldLog(LogLevel::kDebug));
    EXPECT_TRUE(sink.shouldLog(LogLevel::kVerbose));
}

TEST(SinkBase, EnableDisableState) {
    FileSink sink("/tmp/test_enable.log");
    
    EXPECT_TRUE(sink.isEnabled());
    
    // Disable
    sink.setEnabled(false);
    EXPECT_FALSE(sink.isEnabled());
    
    // Get common parameters
    auto now = ::std::chrono::system_clock::now();
    UInt64 timestamp = ::std::chrono::duration_cast<::std::chrono::microseconds>(
        now.time_since_epoch()).count();
    UInt32 threadId = static_cast<UInt32>(::std::hash<::std::thread::id>{}(::std::this_thread::get_id()));
    
    // Write when disabled should not crash
    sink.write(timestamp, threadId, static_cast<lap::log::LogLevelType>(0x04), "TEST", "Message when disabled");
    sink.flush();
    
    // Enable again
    sink.setEnabled(true);
    EXPECT_TRUE(sink.isEnabled());
    
    // Write when enabled
    sink.write(timestamp, threadId, static_cast<lap::log::LogLevelType>(0x04), "TEST", "Message when enabled");
    sink.flush();
    
    ::unlink("/tmp/test_enable.log");
}

TEST(SinkBase, LogEntryParsing) {
    ConsoleSink sink(false, LogLevel::kVerbose);
    
    // Get common parameters
    auto now = ::std::chrono::system_clock::now();
    UInt64 timestamp = ::std::chrono::duration_cast<::std::chrono::microseconds>(
        now.time_since_epoch()).count();
    UInt32 threadId = static_cast<UInt32>(::std::hash<::std::thread::id>{}(::std::this_thread::get_id()));
    
    // Test with various message sizes
    const char* messages[] = {
        "Short",
        "Medium length message with some details",
        "Very long message with lots of content to test buffer handling and make sure everything works correctly even with extended text"
    };
    
    for (const auto* msg : messages) {
        sink.write(timestamp, threadId, static_cast<lap::log::LogLevelType>(0x04), "PARSE", msg);
    }
    
    sink.flush();
}

TEST(SinkBase, ConcurrentWrites) {
    const char* testFile = "/tmp/test_concurrent.log";
    ::unlink(testFile);
    
    FileSink sink(testFile, 10 * 1024 * 1024, 1, LogLevel::kVerbose);
    
    const int NUM_THREADS = 4;
    const int LOGS_PER_THREAD = 100;
    
    std::vector<std::thread> threads;
    
    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([&sink, t]() {
            auto now = ::std::chrono::system_clock::now();
            UInt64 timestamp = ::std::chrono::duration_cast<::std::chrono::microseconds>(
                now.time_since_epoch()).count();
            UInt32 threadId = static_cast<UInt32>(::std::hash<::std::thread::id>{}(::std::this_thread::get_id()));
            
            for (int i = 0; i < LOGS_PER_THREAD; ++i) {
                String msg = "Thread " + std::to_string(t) + " message #" + std::to_string(i);
                sink.write(timestamp, threadId, static_cast<lap::log::LogLevelType>(0x04), "CONC", msg.c_str());
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    sink.flush();
    
    // Verify all logs were written
    FILE* f = fopen(testFile, "r");
    ASSERT_NE(f, nullptr);
    
    int lineCount = 0;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), f) != nullptr) {
        ++lineCount;
    }
    fclose(f);
    
    EXPECT_EQ(lineCount, NUM_THREADS * LOGS_PER_THREAD);
    
    ::unlink(testFile);
}

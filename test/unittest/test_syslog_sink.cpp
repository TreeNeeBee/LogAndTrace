/**
 * @file        test_syslog_sink.cpp
 * @author      ddkv587 ( ddkv587@gmail.com )
 * @brief       SyslogSink unit tests
 * @date        2025-10-28
 */

#include <gtest/gtest.h>
#include <syslog.h>
#include <thread>
#include <chrono>
#include "CSyslogSink.hpp"

using namespace lap::log;
using namespace lap::core;

TEST(SyslogSink, BasicConstruction) {
    SyslogSink sink("TestApp", LOG_USER, LogLevel::kInfo);
    
    EXPECT_TRUE(sink.isEnabled());
    EXPECT_EQ(sink.getName(), "Syslog");
}

TEST(SyslogSink, LevelFiltering) {
    SyslogSink sink("TestApp", LOG_USER, LogLevel::kWarn);
    
    EXPECT_TRUE(sink.shouldLog(LogLevel::kFatal));
    EXPECT_TRUE(sink.shouldLog(LogLevel::kError));
    EXPECT_TRUE(sink.shouldLog(LogLevel::kWarn));
    EXPECT_FALSE(sink.shouldLog(LogLevel::kInfo));
    EXPECT_FALSE(sink.shouldLog(LogLevel::kDebug));
    EXPECT_FALSE(sink.shouldLog(LogLevel::kVerbose));
}

TEST(SyslogSink, WriteBasicLog) {
    SyslogSink sink("LAPTest", LOG_USER, LogLevel::kDebug);
    
    // Get common parameters
    auto now = ::std::chrono::system_clock::now();
    UInt64 timestamp = ::std::chrono::duration_cast<::std::chrono::microseconds>(
        now.time_since_epoch()).count();
    UInt32 threadId = static_cast<UInt32>(::std::hash<::std::thread::id>{}(::std::this_thread::get_id()));
    
    // Write logs at different levels
    sink.write(timestamp, threadId, static_cast<lap::log::LogLevelType>(0x04), "SYSLOG", "Info message to syslog");
    sink.write(timestamp, threadId, static_cast<lap::log::LogLevelType>(0x03), "SYSLOG", "Warning message to syslog");
    sink.write(timestamp, threadId, static_cast<lap::log::LogLevelType>(0x02), "SYSLOG", "Error message to syslog");
    
    sink.flush();
}

TEST(SyslogSink, FacilitySelection) {
    // Test different facilities
    SyslogSink userSink("TestUser", LOG_USER, LogLevel::kInfo);
    EXPECT_EQ(userSink.getName(), "Syslog");
    
    SyslogSink daemonSink("TestDaemon", LOG_DAEMON, LogLevel::kInfo);
    EXPECT_EQ(daemonSink.getName(), "Syslog");
    
    SyslogSink localSink("TestLocal", LOG_LOCAL0, LogLevel::kInfo);
    EXPECT_EQ(localSink.getName(), "Syslog");
}

TEST(SyslogSink, EnableDisable) {
    SyslogSink sink("TestApp", LOG_USER, LogLevel::kInfo);
    
    EXPECT_TRUE(sink.isEnabled());
    
    sink.setEnabled(false);
    EXPECT_FALSE(sink.isEnabled());
    
    // Get common parameters
    auto now = ::std::chrono::system_clock::now();
    UInt64 timestamp = ::std::chrono::duration_cast<::std::chrono::microseconds>(
        now.time_since_epoch()).count();
    UInt32 threadId = static_cast<UInt32>(::std::hash<::std::thread::id>{}(::std::this_thread::get_id()));
    
    // Write should not crash when disabled
    sink.write(timestamp, threadId, static_cast<lap::log::LogLevelType>(0x04), "SYSLOG", "Message when disabled");
    
    sink.setEnabled(true);
    EXPECT_TRUE(sink.isEnabled());
}

TEST(SyslogSink, LevelUpdate) {
    SyslogSink sink("TestApp", LOG_USER, LogLevel::kInfo);
    
    EXPECT_TRUE(sink.shouldLog(LogLevel::kInfo));
    EXPECT_FALSE(sink.shouldLog(LogLevel::kDebug));
    
    sink.setLevel(LogLevel::kDebug);
    EXPECT_TRUE(sink.shouldLog(LogLevel::kDebug));
    EXPECT_TRUE(sink.shouldLog(LogLevel::kInfo));
    
    sink.setLevel(LogLevel::kError);
    EXPECT_FALSE(sink.shouldLog(LogLevel::kWarn));
    EXPECT_TRUE(sink.shouldLog(LogLevel::kError));
}

TEST(SyslogSink, MultipleMessages) {
    SyslogSink sink("LAPMulti", LOG_USER, LogLevel::kVerbose);
    
    // Get common parameters
    auto now = ::std::chrono::system_clock::now();
    UInt64 timestamp = ::std::chrono::duration_cast<::std::chrono::microseconds>(
        now.time_since_epoch()).count();
    UInt32 threadId = static_cast<UInt32>(::std::hash<::std::thread::id>{}(::std::this_thread::get_id()));
    
    const int NUM_MESSAGES = 100;
    for (int i = 0; i < NUM_MESSAGES; ++i) {
        String msg = "Syslog message #" + std::to_string(i);
        sink.write(timestamp, threadId, static_cast<lap::log::LogLevelType>(0x04), "MULTI", msg.c_str());
    }
    
    sink.flush();
}

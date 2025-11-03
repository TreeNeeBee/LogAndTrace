#include <gtest/gtest.h>
#include "CCommon.hpp"

using namespace lap::log;

TEST(LogCommon, ToStringLevels) {
    EXPECT_STREQ(toString(LogLevel::kOff).data(), "Off");
    EXPECT_STREQ(toString(LogLevel::kFatal).data(), "Fatal");
    EXPECT_STREQ(toString(LogLevel::kError).data(), "Error");
    EXPECT_STREQ(toString(LogLevel::kWarn).data(), "Warn");
    EXPECT_STREQ(toString(LogLevel::kInfo).data(), "Info");
    EXPECT_STREQ(toString(LogLevel::kDebug).data(), "Debug");
    EXPECT_STREQ(toString(LogLevel::kVerbose).data(), "Verbose");
}

TEST(LogCommon, ModeOperators) {
    auto m = LogMode::kConsole | LogMode::kFile;
    EXPECT_TRUE((m & LogMode::kConsole) != 0);
    EXPECT_TRUE((m & LogMode::kFile) != 0);
    EXPECT_TRUE((m & LogMode::kDlt) == 0);
}

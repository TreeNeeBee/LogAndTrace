#include <gtest/gtest.h>
#include <string>
#include "CLogManager.hpp"
#include "CLogger.hpp"
#include "CLogStream.hpp"

using namespace lap::log;
using lap::core::InstanceSpecifier;

class LogFixture : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize with default config if available; else fallback to defaults
        LogManager::getInstance().initialize();
    }
    void TearDown() override {
        LogManager::getInstance().uninitialize();
    }
};

TEST_F(LogFixture, CreateLoggerAndBasicLogs) {
    auto &logger = LogManager::getInstance().registerLogger("TEST", "TestCtx", LogLevel::kInfo);

    EXPECT_TRUE(logger.IsEnabled(LogLevel::kInfo));

    // Exercise log paths (we don't assert on external DLT behavior)
    logger.LogInfo() << "hello";
    logger.LogWarn() << "warn";
    logger.LogError() << "err";
    logger.LogFatal() << "fatal";

    // With location
    logger.LogDebug().WithLocation(__FILE__, __LINE__) << "dbg";
}

TEST_F(LogFixture, LogStreamFormatAndBinaryHex) {
    auto &logger = LogManager::getInstance().registerLogger("FMTT", "FmtCtx", LogLevel::kVerbose);

    logger.LogVerbose() << HexFormat(0x12u) << BinFormat(static_cast<uint16_t>(0x34u)) << " text";
    logger.LogVerbose().logFormat("formatted %d %s", 42, "ok");
}

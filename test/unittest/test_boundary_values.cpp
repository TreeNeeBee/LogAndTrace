/**
 * @file        test_boundary_values.cpp
 * @brief       Test boundary values and edge cases for logging system
 * @date        2025-10-29
 */

#include <gtest/gtest.h>
#include <string>
#include <cstring>
#include "CLogManager.hpp"
#include "CLogger.hpp"
#include "CLogStream.hpp"
#include "CFileSink.hpp"
#include <core/CConfig.hpp>

using namespace lap::log;
using namespace lap::core;

class BoundaryValueTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize ConfigManager before LogManager (uses defaults if no config)
        lap::core::ConfigManager::getInstance();
        LogManager::getInstance().initialize();
    }
    
    void TearDown() override {
        LogManager::getInstance().uninitialize();
    }
};

// Test maximum log buffer size (MAX_LOG_SIZE = 200)
TEST_F(BoundaryValueTest, ExactMaxLogSize) {
    auto &logger = LogManager::getInstance().registerLogger("BNDRY", "MaxSize", LogLevel::kInfo);
    
    // Create a message exactly MAX_LOG_SIZE bytes (200)
    std::string exactMsg(LogStream::MAX_LOG_SIZE, 'A');
    
    // This should fit exactly without truncation
    logger.LogInfo() << exactMsg.c_str();
    
    // No assertion - just verify it doesn't crash
    SUCCEED();
}

// Test message exceeding MAX_LOG_SIZE
TEST_F(BoundaryValueTest, ExceedMaxLogSize) {
    auto &logger = LogManager::getInstance().registerLogger("BNDRY", "Exceed", LogLevel::kInfo);
    
    // Create a message larger than MAX_LOG_SIZE (300 bytes > 200)
    std::string longMsg(300, 'B');
    
    // Should be truncated to MAX_LOG_SIZE
    logger.LogInfo() << longMsg.c_str();
    
    // Verify no crash or memory corruption
    SUCCEED();
}

// Test very long message (stress test)
TEST_F(BoundaryValueTest, VeryLongMessage) {
    auto &logger = LogManager::getInstance().registerLogger("BNDRY", "VeryLong", LogLevel::kInfo);
    
    // Create a very long message (10KB)
    std::string veryLongMsg(10240, 'C');
    
    // Should be truncated gracefully
    logger.LogInfo() << veryLongMsg.c_str();
    
    SUCCEED();
}

// Test empty message
TEST_F(BoundaryValueTest, EmptyMessage) {
    auto &logger = LogManager::getInstance().registerLogger("BNDRY", "Empty", LogLevel::kInfo);
    
    // Log empty string
    logger.LogInfo() << "";
    
    // Log with no content
    logger.LogInfo();
    
    SUCCEED();
}

// Test single character
TEST_F(BoundaryValueTest, SingleCharacter) {
    auto &logger = LogManager::getInstance().registerLogger("BNDRY", "Single", LogLevel::kInfo);
    
    logger.LogInfo() << "X";
    logger.LogInfo() << 'Y';
    
    SUCCEED();
}

// Test multiple small appends approaching limit
TEST_F(BoundaryValueTest, MultipleAppendsNearLimit) {
    auto &logger = LogManager::getInstance().registerLogger("BNDRY", "Multi", LogLevel::kInfo);
    
    // Append multiple small strings that together approach MAX_LOG_SIZE
    // Use chain calls instead of saving LogStream
    logger.LogInfo() << "0123456789" << "0123456789" << "0123456789" << "0123456789"
                     << "0123456789" << "0123456789" << "0123456789" << "0123456789"
                     << "0123456789" << "0123456789" << "0123456789" << "0123456789"
                     << "0123456789" << "0123456789" << "0123456789" << "0123456789"
                     << "0123456789" << "0123456789" << "0123456789" << "0123456789"; // 200 bytes
    
    SUCCEED();
}

// Test multiple appends exceeding limit
TEST_F(BoundaryValueTest, MultipleAppendsExceedLimit) {
    auto &logger = LogManager::getInstance().registerLogger("BNDRY", "MultiEx", LogLevel::kInfo);
    
    // Append strings that together exceed MAX_LOG_SIZE (30 * 10 = 300 bytes > 200)
    logger.LogInfo() << "0123456789" << "0123456789" << "0123456789" << "0123456789"
                     << "0123456789" << "0123456789" << "0123456789" << "0123456789"
                     << "0123456789" << "0123456789" << "0123456789" << "0123456789"
                     << "0123456789" << "0123456789" << "0123456789" << "0123456789"
                     << "0123456789" << "0123456789" << "0123456789" << "0123456789"
                     << "0123456789" << "0123456789" << "0123456789" << "0123456789"
                     << "0123456789" << "0123456789" << "0123456789" << "0123456789"
                     << "0123456789" << "0123456789"; // 300 bytes total
    
    // Should be truncated gracefully
    SUCCEED();
}

// Test special characters and null bytes
TEST_F(BoundaryValueTest, SpecialCharacters) {
    auto &logger = LogManager::getInstance().registerLogger("BNDRY", "Special", LogLevel::kInfo);
    
    // Test various special characters
    logger.LogInfo() << "Tab:\t Newline:\n Return:\r Quote:\" Backslash:\\";
    
    // Test null byte in middle of string (should terminate at null)
    char msgWithNull[20] = "Hello\0World";
    logger.LogInfo() << msgWithNull;
    
    SUCCEED();
}

// Test all numeric boundary values
TEST_F(BoundaryValueTest, NumericBoundaries) {
    auto &logger = LogManager::getInstance().registerLogger("BNDRY", "Numeric", LogLevel::kInfo);
    
    // Test min/max values for different types
    logger.LogInfo() << "UInt8 max: " << static_cast<uint8_t>(255);
    logger.LogInfo() << "Int8 min: " << static_cast<int8_t>(-128);
    logger.LogInfo() << "Int8 max: " << static_cast<int8_t>(127);
    
    logger.LogInfo() << "UInt16 max: " << static_cast<uint16_t>(65535);
    logger.LogInfo() << "Int16 min: " << static_cast<int16_t>(-32768);
    logger.LogInfo() << "Int16 max: " << static_cast<int16_t>(32767);
    
    logger.LogInfo() << "UInt32 max: " << static_cast<uint32_t>(4294967295U);
    logger.LogInfo() << "Int32 min: " << static_cast<int32_t>(-2147483648);
    logger.LogInfo() << "Int32 max: " << static_cast<int32_t>(2147483647);
    
    logger.LogInfo() << "UInt64 max: " << static_cast<uint64_t>(18446744073709551615ULL);
    logger.LogInfo() << "Int64 min: " << static_cast<int64_t>(-9223372036854775807LL - 1);
    logger.LogInfo() << "Int64 max: " << static_cast<int64_t>(9223372036854775807LL);
    
    SUCCEED();
}

// Test hex and binary formatting with boundaries
TEST_F(BoundaryValueTest, HexBinaryBoundaries) {
    auto &logger = LogManager::getInstance().registerLogger("BNDRY", "HexBin", LogLevel::kInfo);
    
    logger.LogInfo() << "Hex8 0: " << HexFormat(static_cast<uint8_t>(0));
    logger.LogInfo() << "Hex8 max: " << HexFormat(static_cast<uint8_t>(0xFF));
    
    logger.LogInfo() << "Hex16 0: " << HexFormat(static_cast<uint16_t>(0));
    logger.LogInfo() << "Hex16 max: " << HexFormat(static_cast<uint16_t>(0xFFFF));
    
    logger.LogInfo() << "Hex32 0: " << HexFormat(static_cast<uint32_t>(0));
    logger.LogInfo() << "Hex32 max: " << HexFormat(static_cast<uint32_t>(0xFFFFFFFF));
    
    logger.LogInfo() << "Hex64 0: " << HexFormat(static_cast<uint64_t>(0));
    logger.LogInfo() << "Hex64 max: " << HexFormat(static_cast<uint64_t>(0xFFFFFFFFFFFFFFFFULL));
    
    logger.LogInfo() << "Bin8 0: " << BinFormat(static_cast<uint8_t>(0));
    logger.LogInfo() << "Bin8 max: " << BinFormat(static_cast<uint8_t>(0xFF));
    
    SUCCEED();
}

// Test rapid successive logs
TEST_F(BoundaryValueTest, RapidSuccessiveLogs) {
    auto &logger = LogManager::getInstance().registerLogger("BNDRY", "Rapid", LogLevel::kInfo);
    
    // Log 1000 messages rapidly
    for (int i = 0; i < 1000; ++i) {
        logger.LogInfo() << "Message " << i;
    }
    
    SUCCEED();
}

// Test logger with very long context ID (should be truncated to 4 bytes in FileSink)
TEST_F(BoundaryValueTest, LongContextId) {
    // Context ID longer than 4 bytes
    auto &logger = LogManager::getInstance().registerLogger("BNDRY", "VeryLongContextId", LogLevel::kInfo);
    
    logger.LogInfo() << "Testing long context ID";
    
    SUCCEED();
}

// Test message with only numbers
TEST_F(BoundaryValueTest, OnlyNumbers) {
    auto &logger = LogManager::getInstance().registerLogger("BNDRY", "Nums", LogLevel::kInfo);
    
    // Chain multiple numbers
    logger.LogInfo() << 0 << " " << 1 << " " << 2 << " " << 3 << " " << 4 << " "
                     << 5 << " " << 6 << " " << 7 << " " << 8 << " " << 9 << " "
                     << 10 << " " << 11 << " " << 12 << " " << 13 << " " << 14 << " "
                     << 15 << " " << 16 << " " << 17 << " " << 18 << " " << 19;
    
    SUCCEED();
}

// Test mixed content approaching limit
TEST_F(BoundaryValueTest, MixedContentNearLimit) {
    auto &logger = LogManager::getInstance().registerLogger("BNDRY", "Mixed", LogLevel::kInfo);
    
    // Mix of strings, numbers, hex, binary near MAX_LOG_SIZE
    logger.LogInfo() << "String1 " << 12345 << " Hex:" << HexFormat(static_cast<uint32_t>(0xABCD)) 
                     << " Binary:" << BinFormat(static_cast<uint8_t>(0xFF))
                     << " More text that fills up space "
                     << "Item0 " << "Item1 " << "Item2 " << "Item3 " << "Item4 "
                     << "Item5 " << "Item6 " << "Item7 " << "Item8 " << "Item9 ";
    
    SUCCEED();
}

// Test FileSink with long APPID (should be truncated to 4 bytes)
TEST_F(BoundaryValueTest, FileSinkLongAppId) {
    // Create a FileSink with very long APPID
    std::string longAppId = "VeryLongApplicationId123456789";
    auto sink = std::make_unique<FileSink>(
        StringView("/tmp/test_boundary_appid.log"),
        1024 * 1024,  // 1MB
        5,
        LogLevel::kVerbose,
        StringView(longAppId.c_str(), longAppId.size())
    );
    
    // Verify sink was created
    EXPECT_TRUE(sink != nullptr);
    
    // Clean up
    sink.reset();
    ::unlink("/tmp/test_boundary_appid.log");
}

// Test zero-sized values
TEST_F(BoundaryValueTest, ZeroValues) {
    auto &logger = LogManager::getInstance().registerLogger("BNDRY", "Zero", LogLevel::kInfo);
    
    logger.LogInfo() << "Zero: " << 0;
    logger.LogInfo() << "Zero hex: " << HexFormat(static_cast<uint32_t>(0));
    logger.LogInfo() << "Zero bin: " << BinFormat(static_cast<uint8_t>(0));
    
    SUCCEED();
}

// Test buffer exactly at various sizes
TEST_F(BoundaryValueTest, ExactBufferSizes) {
    auto &logger = LogManager::getInstance().registerLogger("BNDRY", "Exact", LogLevel::kInfo);
    
    // Test exact sizes: 10, 50, 100, 150, 199, 200 bytes
    std::vector<size_t> sizes = {10, 50, 100, 150, 199, 200};
    
    for (size_t size : sizes) {
        std::string msg(size, 'X');
        logger.LogInfo() << msg.c_str();
    }
    
    SUCCEED();
}

// Test off-by-one scenarios
TEST_F(BoundaryValueTest, OffByOneScenarios) {
    auto &logger = LogManager::getInstance().registerLogger("BNDRY", "OBO", LogLevel::kInfo);
    
    // 199 bytes (one less than max)
    std::string msg199(199, 'A');
    logger.LogInfo() << msg199.c_str();
    
    // 200 bytes (exactly max)
    std::string msg200(200, 'B');
    logger.LogInfo() << msg200.c_str();
    
    // 201 bytes (one more than max)
    std::string msg201(201, 'C');
    logger.LogInfo() << msg201.c_str();
    
    SUCCEED();
}

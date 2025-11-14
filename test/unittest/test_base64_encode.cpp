/**
 * @file        test_base64_encode.cpp
 * @brief       Unit tests for base64 encoding feature in LogStream
 * @date        2025-11-14
 */

#include <gtest/gtest.h>
#include "CLogManager.hpp"
#include "CLogger.hpp"
#include "CLogStream.hpp"
#include <lap/core/CInitialization.hpp>
#include <lap/core/CCrypto.hpp>
#include <lap/core/CConfig.hpp>
#include <thread>
#include <chrono>

using namespace lap::log;
using namespace lap::core;

class Base64EncodeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize ConfigManager before LogManager
        lap::core::ConfigManager::getInstance();
        
        // Initialize LogManager
        LogManager::getInstance();
    }

    void TearDown() override
    {
        // Cleanup is automatic
    }
};

// Test 1: Basic WithEncode functionality
TEST_F(Base64EncodeTest, BasicWithEncode)
{
    auto& logger = CreateLogger("TST1", "Base64 Basic Test", LogLevel::kDebug);
    
    // Normal log
    logger.LogInfo() << "Normal message";
    
    // Encoded log
    logger.LogInfo().WithEncode() << "Encoded message";
    
    // Should not crash
    SUCCEED();
}

// Test 2: WithEncode chaining with WithLevel
TEST_F(Base64EncodeTest, WithLevelAndEncode)
{
    auto& logger = CreateLogger("TST2", "Level+Encode Test", LogLevel::kDebug);
    
    // Test all log levels with encoding
    logger.WithLevel(LogLevel::kFatal).WithEncode() << "Fatal with encode";
    logger.WithLevel(LogLevel::kError).WithEncode() << "Error with encode";
    logger.WithLevel(LogLevel::kWarn).WithEncode() << "Warn with encode";
    logger.WithLevel(LogLevel::kInfo).WithEncode() << "Info with encode";
    logger.WithLevel(LogLevel::kDebug).WithEncode() << "Debug with encode";
    logger.WithLevel(LogLevel::kVerbose).WithEncode() << "Verbose with encode";
    
    SUCCEED();
}

// Test 3: WithEncode(false) should not encode
TEST_F(Base64EncodeTest, WithEncodeDisabled)
{
    auto& logger = CreateLogger("TST3", "Encode Disabled Test", LogLevel::kDebug);
    
    // Explicitly disable encoding
    logger.LogInfo().WithEncode(false) << "Not encoded";
    
    SUCCEED();
}

// Test 4: WithLocation combined with WithEncode
TEST_F(Base64EncodeTest, WithLocationAndEncode)
{
    auto& logger = CreateLogger("TST4", "Location+Encode Test", LogLevel::kDebug);
    
    // Location before encode
    logger.LogInfo().WithLocation(__FILE__, __LINE__).WithEncode() << "Location then encode";
    
    // Encode before location
    logger.LogInfo().WithEncode().WithLocation(__FILE__, __LINE__) << "Encode then location";
    
    SUCCEED();
}

// Test 5: Multiple consecutive encoded messages
TEST_F(Base64EncodeTest, ConsecutiveEncodedMessages)
{
    auto& logger = CreateLogger("TST5", "Consecutive Encode Test", LogLevel::kDebug);
    
    for (int i = 0; i < 10; ++i) {
        logger.LogInfo().WithEncode() << "Encoded message " << i;
    }
    
    SUCCEED();
}

// Test 6: Encoding with different data types
TEST_F(Base64EncodeTest, EncodeWithDifferentTypes)
{
    auto& logger = CreateLogger("TST6", "Data Types Test", LogLevel::kDebug);
    
    // Integer types
    logger.LogInfo().WithEncode() << "Int8: " << static_cast<Int8>(127);
    logger.LogInfo().WithEncode() << "UInt16: " << static_cast<UInt16>(65535);
    logger.LogInfo().WithEncode() << "Int32: " << static_cast<Int32>(-12345);
    logger.LogInfo().WithEncode() << "UInt64: " << static_cast<UInt64>(18446744073709551615ULL);
    
    // Float types
    logger.LogInfo().WithEncode() << "Float: " << 3.14159f;
    logger.LogInfo().WithEncode() << "Double: " << 3.141592653589793;
    
    // String types
    logger.LogInfo().WithEncode() << "String: Hello World";
    logger.LogInfo().WithEncode() << "StringView: " << StringView("Test View");
    
    SUCCEED();
}

// Test 7: Encoding with special characters
TEST_F(Base64EncodeTest, EncodeSpecialCharacters)
{
    auto& logger = CreateLogger("TST7", "Special Chars Test", LogLevel::kDebug);
    
    logger.LogInfo().WithEncode() << "Special: !@#$%^&*()_+-={}[]|\\:;\"'<>?,./";
    logger.LogInfo().WithEncode() << "Unicode: 中文测试 日本語 한글";
    logger.LogInfo().WithEncode() << "Newline: \n Tab: \t";
    
    SUCCEED();
}

// Test 8: Encoding with hex and binary formats
TEST_F(Base64EncodeTest, EncodeWithFormats)
{
    auto& logger = CreateLogger("TST8", "Format Test", LogLevel::kDebug);
    
    logger.LogInfo().WithEncode() << "Hex8: " << HexFormat(static_cast<UInt8>(0xFF));
    logger.LogInfo().WithEncode() << "Hex16: " << HexFormat(static_cast<UInt16>(0xABCD));
    logger.LogInfo().WithEncode() << "Hex32: " << HexFormat(static_cast<UInt32>(0x12345678));
    logger.LogInfo().WithEncode() << "Hex64: " << HexFormat(static_cast<UInt64>(0x123456789ABCDEF0));
    
    logger.LogInfo().WithEncode() << "Bin8: " << BinFormat(static_cast<UInt8>(0b10101010));
    
    SUCCEED();
}

// Test 9: Multi-threaded encoding
TEST_F(Base64EncodeTest, MultiThreadedEncode)
{
    auto& logger = CreateLogger("TST9", "MultiThread Test", LogLevel::kDebug);
    
    std::vector<std::thread> threads;
    const int numThreads = 5;
    const int messagesPerThread = 20;
    
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([&logger, t, messagesPerThread]() {
            for (int i = 0; i < messagesPerThread; ++i) {
                logger.LogInfo().WithEncode() << "Thread " << t << " Message " << i;
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    SUCCEED();
}

// Test 10: Encoding empty message
TEST_F(Base64EncodeTest, EncodeEmptyMessage)
{
    auto& logger = CreateLogger("TST10", "Empty Message Test", LogLevel::kDebug);
    
    logger.LogInfo().WithEncode() << "";
    
    SUCCEED();
}

// Test 11: Encoding very long message
TEST_F(Base64EncodeTest, EncodeLongMessage)
{
    auto& logger = CreateLogger("TST11", "Long Message Test", LogLevel::kDebug);
    
    // Create a message that's close to buffer limit
    String longMsg;
    for (int i = 0; i < 150; ++i) {
        longMsg += "X";
    }
    
    logger.LogInfo().WithEncode() << longMsg;
    
    SUCCEED();
}

// Test 12: Mix encoded and non-encoded messages
TEST_F(Base64EncodeTest, MixedEncodingMessages)
{
    auto& logger = CreateLogger("TST12", "Mixed Test", LogLevel::kDebug);
    
    logger.LogInfo() << "Normal 1";
    logger.LogInfo().WithEncode() << "Encoded 1";
    logger.LogInfo() << "Normal 2";
    logger.LogInfo().WithEncode() << "Encoded 2";
    logger.LogInfo() << "Normal 3";
    
    SUCCEED();
}

// Test 13: Verify base64 encoding is actually working
TEST_F(Base64EncodeTest, VerifyBase64Encoding)
{
    // Test the base64 encoding utility directly
    String testMsg = "Hello World!";
    String encoded = Crypto::Util::base64Encode(
        reinterpret_cast<const UInt8*>(testMsg.data()), 
        testMsg.size()
    );
    
    // Base64 of "Hello World!" should be "SGVsbG8gV29ybGQh"
    EXPECT_EQ(encoded, "SGVsbG8gV29ybGQh");
}

// Test 14: Encoding with IsEnabled check
TEST_F(Base64EncodeTest, EncodeWithLevelCheck)
{
    auto& logger = CreateLogger("TST14", "Level Check Test", LogLevel::kWarn);
    
    // These should not be logged (below Warn level)
    logger.LogDebug().WithEncode() << "Debug encoded (should not appear)";
    logger.LogVerbose().WithEncode() << "Verbose encoded (should not appear)";
    
    // These should be logged
    logger.LogWarn().WithEncode() << "Warn encoded (should appear)";
    logger.LogError().WithEncode() << "Error encoded (should appear)";
    
    SUCCEED();
}

// Test 15: Performance test - encoding overhead
TEST_F(Base64EncodeTest, EncodingPerformance)
{
    auto& logger = CreateLogger("TST15", "Performance Test", LogLevel::kDebug);
    
    const int iterations = 1000;
    
    // Test without encoding
    auto start1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        logger.LogDebug() << "Performance test message " << i;
    }
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count();
    
    // Test with encoding
    auto start2 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        logger.LogDebug().WithEncode() << "Performance test message " << i;
    }
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count();
    
    // Print performance results
    std::cout << "Without encoding: " << duration1 << " μs (" << (iterations * 1000000.0 / duration1) << " logs/sec)" << std::endl;
    std::cout << "With encoding: " << duration2 << " μs (" << (iterations * 1000000.0 / duration2) << " logs/sec)" << std::endl;
    std::cout << "Encoding overhead: " << ((duration2 - duration1) * 100.0 / duration1) << "%" << std::endl;
    
    SUCCEED();
}

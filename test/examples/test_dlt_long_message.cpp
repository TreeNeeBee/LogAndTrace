/**
 * @file        test_dlt_long_message.cpp
 * @brief       Test DLT with various message lengths
 * @date        2025-10-29
 */

#include <iostream>
#include <string>
#include <unistd.h>
#include "CLogManager.hpp"
#include "CLogger.hpp"

using namespace lap::log;

int main(int argc, char** argv)
{
    (void)argc;  // Unused
    (void)argv;  // Unused
    
    lap::core::MemManager::getInstance();  // Initialize memory manager first
    std::cout << "Testing DLT with various message lengths..." << std::endl;
    
    // Initialize LogManager with DLT enabled
    auto& logMgr = LogManager::getInstance();
    bool result = logMgr.initialize(lap::core::InstanceSpecifier("config_dlt.json"));
    
    if (!result) {
        std::cerr << "Failed to initialize LogManager with DLT config\n";
        return 1;
    }
    
    std::cout << "LogManager initialized with DLT sink\n";
    
    // Register a logger
    auto& logger = logMgr.registerLogger("DTST", "DLT Test Context", LogLevel::kVerbose);
    
    std::cout << "\n=== Test 1: Short message ===" << std::endl;
    logger.LogInfo() << "Short message";
    sleep(1);
    
    std::cout << "\n=== Test 2: Medium message (50 bytes) ===" << std::endl;
    std::string medium50(50, 'M');
    logger.LogInfo() << "Medium: " << medium50.c_str();
    sleep(1);
    
    std::cout << "\n=== Test 3: Near MAX_LOG_SIZE (190 bytes) ===" << std::endl;
    std::string near190(190, 'N');
    logger.LogInfo() << near190.c_str();
    sleep(1);
    
    std::cout << "\n=== Test 4: Exactly MAX_LOG_SIZE (200 bytes) ===" << std::endl;
    std::string exactly200(200, 'E');
    logger.LogInfo() << exactly200.c_str();
    sleep(1);
    
    std::cout << "\n=== Test 5: Exceed MAX_LOG_SIZE (300 bytes, should be truncated to 200) ===" << std::endl;
    std::string exceed300(300, 'X');
    logger.LogInfo() << exceed300.c_str();
    sleep(1);
    
    std::cout << "\n=== Test 6: Very long (10KB, should be truncated to 200) ===" << std::endl;
    std::string veryLong(10240, 'L');
    logger.LogInfo() << veryLong.c_str();
    sleep(1);
    
    std::cout << "\n=== Test 7: Multiple parts near limit ===" << std::endl;
    logger.LogInfo() << "Part1: " << std::string(50, 'A').c_str()
                     << " Part2: " << std::string(50, 'B').c_str()
                     << " Part3: " << std::string(50, 'C').c_str()
                     << " Part4: " << std::string(50, 'D').c_str();
    sleep(1);
    
    std::cout << "\n=== Test 8: Special characters ===" << std::endl;
    logger.LogInfo() << "Special: \t\n\r\"\\";
    sleep(1);
    
    std::cout << "\n=== Test 9: Unicode (if supported) ===" << std::endl;
    logger.LogInfo() << "Unicode: 你好世界 🚀 Test";
    sleep(1);
    
    std::cout << "\n=== Test 10: Numbers and formatting ===" << std::endl;
    logger.LogInfo() << "Numbers: " << 12345 << " Hex: " << HexFormat(0xABCDu) 
                     << " Binary: " << BinFormat(static_cast<uint8_t>(0xFF));
    sleep(1);
    
    std::cout << "\nAll tests completed. Check dlt-viewer for messages." << std::endl;
    std::cout << "Expected: All 10 messages should appear in DLT viewer" << std::endl;
    std::cout << "Note: Messages 5 and 6 will be truncated to 200 bytes" << std::endl;
    
    // Sleep to ensure DLT messages are flushed
    sleep(1);
    
    return 0;
}

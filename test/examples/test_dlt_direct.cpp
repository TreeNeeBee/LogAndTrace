/**
 * @file        test_dlt_direct.cpp
 * @brief       Direct DLT API test to debug message sending
 */

#include <iostream>
#include <dlt/dlt.h>
#include <unistd.h>

DLT_DECLARE_CONTEXT(testCtx);

int main() {
    std::cout << "=== Direct DLT API Test ===" << std::endl;
    
    // Register app
    int ret = dlt_register_app("TDIR", "Direct Test App");
    std::cout << "dlt_register_app() returned: " << ret << std::endl;
    
    if (ret < 0) {
        std::cerr << "Failed to register app" << std::endl;
        return 1;
    }
    
    // Register context with verbose mode
    ret = dlt_register_context_ll_ts(&testCtx, "TST1", "Test Context", DLT_LOG_VERBOSE, DLT_TRACE_STATUS_ON);
    std::cout << "dlt_register_context_ll_ts() returned: " << ret << std::endl;
    
    if (ret < 0) {
        std::cerr << "Failed to register context" << std::endl;
        dlt_unregister_app();
        return 1;
    }
    
    std::cout << "\n--- Sending test messages ---" << std::endl;
    
    // Test 1: Simple message with DLT_LOG macro
    std::cout << "Test 1: Using DLT_LOG macro" << std::endl;
    DLT_LOG(testCtx, DLT_LOG_INFO, DLT_STRING("Test message 1 from DLT_LOG macro"));
    
    // Test 2: Manual log write
    std::cout << "Test 2: Using dlt_user_log_write_start/finish" << std::endl;
    DltContextData contextData;
    ret = dlt_user_log_write_start(&testCtx, &contextData, DLT_LOG_INFO);
    std::cout << "  dlt_user_log_write_start() returned: " << ret << std::endl;
    
    if (ret > 0) {
        dlt_user_log_write_string(&contextData, "Test message 2 from manual write");
        ret = dlt_user_log_write_finish(&contextData);
        std::cout << "  dlt_user_log_write_finish() returned: " << ret << std::endl;
    }
    
    // Test 3: Multiple messages
    std::cout << "\nTest 3: Sending 5 messages of different levels" << std::endl;
    DLT_LOG(testCtx, DLT_LOG_FATAL, DLT_STRING("FATAL level message"));
    DLT_LOG(testCtx, DLT_LOG_ERROR, DLT_STRING("ERROR level message"));
    DLT_LOG(testCtx, DLT_LOG_WARN, DLT_STRING("WARN level message"));
    DLT_LOG(testCtx, DLT_LOG_INFO, DLT_STRING("INFO level message"));
    DLT_LOG(testCtx, DLT_LOG_VERBOSE, DLT_STRING("VERBOSE level message"));
    
    std::cout << "\n--- Waiting for DLT to process ---" << std::endl;
    sleep(1);
    
    // Cleanup
    dlt_unregister_context(&testCtx);
    dlt_unregister_app();
    
    std::cout << "\n✅ Test completed" << std::endl;
    std::cout << "Check dlt-viewer for messages from:" << std::endl;
    std::cout << "  Application ID: TDIR" << std::endl;
    std::cout << "  Context ID: TST1" << std::endl;
    
    return 0;
}

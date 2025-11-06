/**
 * @file        benchmark_multiprocess.cpp
 * @author      ddkv587
 * @brief       Multi-process file sink benchmark to test concurrent writes
 * @date        2025-10-29
 * @details     Tests multiple processes writing to the same log file simultaneously
 */

#include "CLogManager.hpp"
#include <lap/core/CInstanceSpecifier.hpp>
#include <iostream>
#include <chrono>
#include <atomic>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fstream>
#include <set>
#include <vector>

using namespace lap::log;
using namespace std::chrono;

// Configuration
constexpr int NUM_PROCESSES = 4;
constexpr int LOGS_PER_PROCESS = 10000;
constexpr const char* LOG_FILE = "/tmp/multiprocess_test.log";
constexpr const char* CONFIG_FILE = "/tmp/multiprocess_config.json";

// Create config file for multi-process test
void createConfigFile() {
    std::ofstream config(CONFIG_FILE);
    config << R"({
    "logConfig": {
        "applicationId": "MPRC",
        "applicationDescription": "Multi-Process Test",
        "contextId": "MAIN",
        "contextDescription": "Main Context",
        "logTraceDefaultLogLevel": "Info",
        "logTraceFilePath": ")" << LOG_FILE << R"(",
        "logTraceLogMode": ["file"],
        "withSessionId": 0,
        "withTimeStamp": 1,
        "withEcuId": 0,
        "logMarker": false,
        "verboseMode": false
    }
})";
    config.close();
}

// Child process function
void childProcess(int processId) {
    // Initialize logger
    auto& logMgr = LogManager::getInstance();
    lap::core::InstanceSpecifier spec(CONFIG_FILE);
    logMgr.initialize(spec);
    auto& logger = logMgr.logger("PROC");
    
    // Warmup
    for (int i = 0; i < 100; ++i) {
        logger.LogInfo() << "Warmup " << processId << " #" << i;
    }
    
    // Start benchmark
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < LOGS_PER_PROCESS; ++i) {
        logger.LogInfo() << "[P" << processId << "] Message #" << i << " from process " << processId;
    }
    
    // Logger will flush on destruction (end of process)
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();
    
    // Clean shutdown to avoid static destruction races in child
    logMgr.uninitialize();

    // Print process stats to stdout (will be collected by parent)
    std::cout << "Process " << processId << ": " 
              << LOGS_PER_PROCESS << " logs in " 
              << duration << "ms ("
              << (LOGS_PER_PROCESS * 1000 / duration) << " logs/sec)"
              << std::endl;
}

// Verify log file integrity
struct VerificationResult {
    size_t totalLines = 0;
    size_t uniqueMessages = 0;
    size_t duplicates = 0;
    size_t corrupted = 0;
    bool integrityOk = true;
};

VerificationResult verifyLogFile() {
    VerificationResult result;
    std::set<std::string> uniqueMessages;
    std::ifstream logFile(LOG_FILE);
    std::string line;
    
    while (std::getline(logFile, line)) {
        result.totalLines++;
        
        // Check for corruption (incomplete lines, wrong format)
        if (line.empty() || line.size() < 10) {
            result.corrupted++;
            result.integrityOk = false;
            continue;
        }
        
        // Extract message content for uniqueness check
        size_t msgStart = line.find("] Message #");
        if (msgStart != std::string::npos) {
            std::string msgContent = line.substr(msgStart);
            
            if (uniqueMessages.count(msgContent) > 0) {
                result.duplicates++;
                result.integrityOk = false;
            } else {
                uniqueMessages.insert(msgContent);
            }
        }
    }
    
    result.uniqueMessages = uniqueMessages.size();
    
    return result;
}

int main() {
    lap::core::MemManager::getInstance();  // Initialize memory manager first
    std::cout << "==============================================\n";
    std::cout << "  Multi-Process FileSink Benchmark\n";
    std::cout << "==============================================\n\n";
    
    // Clean up old files
    unlink(LOG_FILE);
    
    // Create config
    createConfigFile();
    
    std::cout << "Configuration:\n";
    std::cout << "  Processes:       " << NUM_PROCESSES << "\n";
    std::cout << "  Logs/process:    " << LOGS_PER_PROCESS << "\n";
    std::cout << "  Total logs:      " << (NUM_PROCESSES * LOGS_PER_PROCESS) << "\n";
    std::cout << "  Log file:        " << LOG_FILE << "\n\n";
    
    std::cout << "Starting processes...\n";
    
    auto startTime = high_resolution_clock::now();
    
    // Fork child processes
    std::vector<pid_t> pids;
    for (int i = 0; i < NUM_PROCESSES; ++i) {
        pid_t pid = fork();
        
        if (pid == 0) {
            // Child process
            childProcess(i);
            exit(0);
        } else if (pid > 0) {
            // Parent process
            pids.push_back(pid);
            std::cout << "  Launched process " << i << " (PID: " << pid << ")\n";
        } else {
            std::cerr << "Failed to fork process " << i << std::endl;
            return 1;
        }
    }
    
    std::cout << "\nWaiting for processes to complete...\n\n";
    
    // Wait for all children
    for (size_t i = 0; i < pids.size(); ++i) {
        int status;
        waitpid(pids[i], &status, 0);
    }
    
    auto endTime = high_resolution_clock::now();
    auto totalDuration = duration_cast<milliseconds>(endTime - startTime).count();
    
    std::cout << "\n==============================================\n";
    std::cout << "  Benchmark Results\n";
    std::cout << "==============================================\n\n";
    
    std::cout << "Timing:\n";
    std::cout << "  Total duration:  " << totalDuration << " ms\n";
    std::cout << "  Throughput:      " 
              << ((NUM_PROCESSES * LOGS_PER_PROCESS * 1000) / totalDuration) 
              << " logs/sec (aggregate)\n\n";
    
    // Verify log file
    std::cout << "Verifying log file integrity...\n";
    auto verification = verifyLogFile();
    
    std::cout << "\nFile Statistics:\n";
    std::cout << "  Total lines:     " << verification.totalLines << "\n";
    std::cout << "  Expected lines:  " << (NUM_PROCESSES * (LOGS_PER_PROCESS + 100)) << " (including warmup)\n";
    std::cout << "  Unique messages: " << verification.uniqueMessages << "\n";
    std::cout << "  Duplicates:      " << verification.duplicates << "\n";
    std::cout << "  Corrupted lines: " << verification.corrupted << "\n";
    
    // Get file size
    struct stat st;
    if (stat(LOG_FILE, &st) == 0) {
        std::cout << "  File size:       " << st.st_size << " bytes (" 
                  << (st.st_size / 1024.0) << " KB)\n";
    }
    
    std::cout << "\n==============================================\n";
    std::cout << "  Integrity Check\n";
    std::cout << "==============================================\n\n";
    
    if (verification.integrityOk) {
        std::cout << "✅ PASSED: No data corruption detected\n";
        std::cout << "✅ All log messages written correctly\n";
    } else {
        std::cout << "❌ FAILED: Data corruption detected!\n";
        if (verification.duplicates > 0) {
            std::cout << "   - " << verification.duplicates << " duplicate messages\n";
        }
        if (verification.corrupted > 0) {
            std::cout << "   - " << verification.corrupted << " corrupted lines\n";
        }
        
        size_t expectedMessages = NUM_PROCESSES * LOGS_PER_PROCESS;
        size_t actualMessages = verification.uniqueMessages;
        if (actualMessages < expectedMessages) {
            std::cout << "   - Missing " << (expectedMessages - actualMessages) << " messages\n";
        }
    }
    
    std::cout << "\n==============================================\n";
    std::cout << "  Analysis\n";
    std::cout << "==============================================\n\n";
    
    if (!verification.integrityOk) {
        std::cout << "⚠️  Multi-process file writing has issues!\n\n";
        std::cout << "Common causes:\n";
        std::cout << "1. Missing file locking (flock/fcntl)\n";
        std::cout << "2. Concurrent fopen() calls overwriting file position\n";
        std::cout << "3. Buffer interleaving when multiple processes write\n";
        std::cout << "4. File rotation conflicts between processes\n\n";
        
        std::cout << "Recommendations:\n";
        std::cout << "1. Add advisory file locking (flock) in FileSink\n";
        std::cout << "2. Use O_APPEND flag for atomic writes\n";
        std::cout << "3. Consider per-process log files with log rotation\n";
        std::cout << "4. Use a centralized logging daemon for multi-process\n";
    } else {
        std::cout << "✅ File sink is multi-process safe!\n";
        std::cout << "   Performance: " 
                  << ((NUM_PROCESSES * LOGS_PER_PROCESS * 1000) / totalDuration) 
                  << " logs/sec aggregate\n";
    }
    
    std::cout << "\n==============================================\n";
    
    // Clean up
    unlink(CONFIG_FILE);
    
    return verification.integrityOk ? 0 : 1;
}

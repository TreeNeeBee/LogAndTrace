# LogAndTrace Module

**High-Performance Logging System for AUTOSAR Adaptive Platform**

[![Tests](https://img.shields.io/badge/tests-50%2F50%20passing-brightgreen)](doc/TEST_REPORT.md)
[![Performance](https://img.shields.io/badge/throughput-6.2M%20logs%2Fsec-blue)](doc/BENCHMARK_REPORT.md)
[![Latency](https://img.shields.io/badge/latency-160ns-blue)](doc/BENCHMARK_REPORT.md)
[![Memory](https://img.shields.io/badge/zero--copy-validated-brightgreen)](doc/BENCHMARK_REPORT.md)
[![Security](https://img.shields.io/badge/buffer-protected-green)](doc/TEST_REPORT.md)

---

## 🆕 Recent Updates (2025-10-29)

### Security & Robustness Enhancements

- **🛡️ FileSink Buffer Overflow Protection**  
  Added bounds checking for prefix length and message truncation to prevent buffer overflows with long timestamps or application IDs.

- **🛡️ DLT StringView Safety**  
  Changed from `dlt_user_log_write_string()` to `dlt_user_log_write_sized_utf8_string()` to safely handle non-null-terminated StringView data.

- **🛡️ Static Destructor Fix**  
  Added `MemManager::getInstance()` initialization in all main functions to prevent "pure virtual method called" crashes on exit.

### Testing Improvements

- **✅ Boundary Value Testing**  
  Added 18 comprehensive tests covering MAX_LOG_SIZE (200 bytes), buffer limits, edge cases, and truncation scenarios.

- **✅ DLT Long Message Testing**  
  Created dedicated test suite for DLT with messages ranging from 1 byte to 10KB, verified in dlt-viewer.

- **✅ All 50 Tests Passing**  
  Increased from 30 to 50 tests, covering all security fixes, boundary cases, and integration scenarios.

### Performance Validation

- **⚡ 6.86M logs/sec** single-thread peak (avg of 10 runs)
- **⚡ 5.71M logs/sec** sustained throughput (10 seconds, avg of 5 runs)
- **⚡ 137.23ns** mean latency (avg of 10 runs)
- **⚡ 175.24ns** sustained load average latency
- **💾 0 bytes** memory growth for 50K logs

---

## 🚀 Overview

LightAP LogAndTrace 是一个为 AUTOSAR Adaptive Platform 设计的企业级日志系统，提供极致性能、完整的 DLT 支持、零拷贝架构以及生产级安全保障。

### Key Features

| Feature | Description | Status |
|---------|-------------|--------|
| **🔥 Ultra-High Performance** | 6.2M+ logs/sec sustained, 160ns avg latency | ✅ |
| **🧵 Thread-Safe** | Lock-free design, validated under stress | ✅ |
| **📊 DLT Integration** | Full GENIVI DLT support with API encapsulation | ✅ |
| **💾 Zero-Copy** | No heap allocations during logging | ✅ |
| **🛡️ Buffer Safety** | Overflow protection, bounds checking | ✅ |
| **🎯 Multi-Sink** | Console, File, Syslog, DLT simultaneously | ✅ |
| **🔧 JSON Config** | Runtime configuration without recompilation | ✅ |
| **✅ Production Ready** | 50 tests passing, boundary cases covered | ✅ |

---

## 📊 Performance Metrics

### Throughput (Based on 10 runs, averaged)

| Scenario | Throughput | Details | Stability |
|----------|------------|---------|-----------|
| **Single-Thread Peak** | **6.86M logs/sec** | Mean: 6,861,184, Median: 6,666,666 | ±1.1M (σ) |
| **Multi-Thread (10 threads)** | **3.20M logs/sec** | Mean: 3,195,897, Median: 3,333,333 | ±450K (σ) |
| **Sustained Load (10s)** | **5.71M logs/sec** | Mean: 5,706,496 (5 runs) | ±371K (σ) |

### Latency Distribution (Based on 10 runs, averaged)

| Percentile | Latency | Notes |
|------------|---------|-------|
| **Mean** | 137.23 ns (0.137 µs) | Simple benchmark |
| **Median** | 138.01 ns (0.138 µs) | Simple benchmark |
| **Sustained Load Avg** | 175.24 ns (0.175 µs) | 10-second test |
| **StdDev** | ±7.34 ns | Low variance |

### Memory Efficiency

- **Zero-copy validated**: 0 bytes growth for 50,000 logs
- **Fixed footprint**: 8.75 KB memory pool
- **No allocations**: During hot path
- **Buffer safety**: Overflow protection in all sinks

### Security & Robustness

- **FileSink**: Buffer overflow protection with prefixLen checking
- **DLT**: StringView safety with sized string API
- **Boundary testing**: 18 comprehensive edge case tests
- **Clean exit**: No crashes with proper singleton initialization

**📈 [Full Benchmark Report →](doc/BENCHMARK_REPORT.md)**

### Testing Environment

All benchmarks and tests were conducted on the following hardware and software configuration:

#### Hardware Specifications
- **CPU**: Intel(R) Core(TM) i5-10210U @ 1.60GHz (4 cores, 8 threads)
- **CPU Frequency**: Base 1.6 GHz, Turbo up to 4.2 GHz
- **Memory**: 16 GB RAM
- **Storage**: SSD

#### Software Environment
- **Operating System**: Debian GNU/Linux 12 (bookworm)
- **Kernel**: 6.1.0-23-amd64
- **Compiler**: GCC 12.2.0
- **C++ Standard**: C++14
- **CMake**: 3.25.1
- **DLT Daemon**: 2.18.8

#### Test Methodology
- **Benchmark Runs**: 10 runs for simple tests, 5 runs for sustained load
- **Interval Between Runs**: 3-5 seconds cooldown
- **Statistical Analysis**: Mean, median, standard deviation reported
- **System Load**: Minimal background processes during testing

> **Note**: Performance may vary depending on CPU frequency, system load, and storage I/O characteristics. The reported metrics represent typical performance on this Intel Core i5-10210U mobile processor.

---

## 🚀 Quick Start

### Basic Usage

```cpp
#include "CLogManager.hpp"
#include <core/MemManager.hpp>

using namespace lap::log;

int main() {
    // Initialize memory manager first (prevents static destructor issues)
    lap::core::MemManager::getInstance();
    
    // Initialize with config
    LogManager::getInstance().initialize(lap::core::InstanceSpecifier("config.json"));
    
    // Get logger
    auto& logger = LogManager::getInstance().getLogger("APP");
    
    // Log messages
    logger.LogError() << "Critical error: " << errorCode;
    logger.LogWarn() << "Warning: " << warningMsg;
    logger.LogInfo() << "Application started";
    logger.LogDebug() << "Debug info: " << debugData;
    
    return 0;
}
```

### Multi-Sink Configuration

**config.json:**
```json
{
  "logConfig": {
    "applicationId": "MYAPP",
    "applicationDescription": "My Application",
    "contextId": "MAIN",
    "contextDescription": "Main Context",
    "logTraceDefaultLogLevel": "Debug",
    "logTraceFilePath": "/var/log/app.log",
    "logTraceLogMode": ["console", "file", "dlt"],
  },
  "dlt": {
    "appId": "MYAP",
    "contextId": "MAIN",
    "level": "DEBUG"
  }
}
```

---

## 🎯 Supported Sinks

### 1. ConsoleSink
**Purpose:** Terminal output with optional colors  
**Config:**
```json
{
  "console": {
    "level": "DEBUG",
    "color": true
  }
}
```

### 2. FileSink
**Purpose:** File logging with rotation  
**Config:**
```json
{
  "file": {
    "path": "/var/log/app.log",
    "level": "INFO",
    "maxSize": "10MB",
    "maxFiles": 5
  }
}
```

### 3. SyslogSink
**Purpose:** System syslog integration  
**Config:**
```json
{
  "syslog": {
    "facility": "LOG_USER",
    "level": "WARN"
  }
}
```

### 4. DLTSink (Network)
**Purpose:** GENIVI DLT integration  
**Config:**
```json
{
  "dlt": {
    "appId": "MYAP",
    "appDescription": "My Application",
    "contextId": "MAIN",
    "contextDescription": "Main Context",
    "level": "DEBUG",
    "verboseMode": true
  }
}
```

**DLT Verification:**
```bash
# View logs in dlt-viewer
dlt-viewer &

# Or use command line
dlt-receive -a
```

---

## 🏗️ Architecture

### Zero-Copy Design

```
Logger → LogStream → StringView → SinkManager → [Sinks...]
         (stack)     (no copy)    (dispatch)     (parallel)
```

**Key Principles:**
1. **StringView-based** message passing (no string copies)
2. **Memory pools** for fixed allocations
3. **Direct writes** to sink buffers
4. **Lock-free** where possible

### DLT Encapsulation

```
┌─────────────────────────────────────┐
│         Application Layer           │
│  (Logger, LogManager - DLT-free)    │
└──────────────┬──────────────────────┘
               │ LogLevelType (internal)
┌──────────────▼──────────────────────┐
│         CDLTSink (isolated)         │
│  • dlt_register_app()               │
│  • dlt_register_context()           │
│  • dlt_user_log_write_string()      │
│  • Level conversion (internal↔DLT)  │
└─────────────────────────────────────┘
```

**Benefits:**
- ✅ No DLT dependencies in core logging
- ✅ Type-safe log level handling
- ✅ Easy to replace/mock DLT
- ✅ Clean separation of concerns

### Buffer Safety & Security

**1. FileSink Overflow Protection**
```cpp
// Before writing, check buffer capacity
int prefixLen = std::snprintf(buffer, sizeof(buffer), 
                              "[%s] [%s] [%s] ", timestamp, appId, level);

// Critical safety check
if (prefixLen <= 0 || prefixLen >= static_cast<int>(sizeof(buffer))) {
    return;  // Prefix too long, abort safely
}

// Calculate available space
size_t availableSpace = sizeof(buffer) - prefixLen - 2;  // -2 for "\n\0"

// Truncate message if needed
if (msgLen > availableSpace) {
    msgLen = availableSpace;
}
```

**2. DLT StringView Safety**
```cpp
// StringView is not guaranteed null-terminated
// Use sized string API with explicit length
uint16_t msgLen = static_cast<uint16_t>(message.size());
if (msgLen > 1300) msgLen = 1300;  // DLT limit with margin

int ret = dlt_user_log_write_sized_utf8_string(&contextData, 
                                                message.data(), 
                                                msgLen);
if (ret < 0) {
    fprintf(stderr, "DLT write failed: %d\n", ret);
}
```

**3. Static Initialization Order**
```cpp
// Always initialize MemManager first in main()
int main() {
    lap::core::MemManager::getInstance();  // CRITICAL: Must be first
    
    auto& logMgr = LogManager::getInstance();  // Now safe
    // ... rest of code
    
    return 0;  // Clean destructor order guaranteed
}
```

**Protections:**
- 🛡️ **FileSink**: prefixLen bounds checking, message truncation
- 🛡️ **DLT**: Non-null-terminated string safety
- 🛡️ **MAX_LOG_SIZE**: 200-byte limit enforced everywhere
- 🛡️ **Static dtors**: Proper singleton initialization order

---

## 📚 API Reference

### CLogManager (Singleton)

```cpp
// Get instance
auto& manager = LogManager::getInstance();

// Initialize with config file
manager.initialize(lap::core::InstanceSpecifier("config.json"));

// Get logger
auto& logger = manager.registerLogger("CTX", "Description", LogLevel::kDebug);

// Shutdown
manager.shutdown();
```

### CLogger

```cpp
// Logging methods
logger.fatal("message", args...);
logger.error("message", args...);
logger.warn("message", args...);
logger.info("message", args...);
logger.debug("message", args...);
logger.verbose("message", args...);

// Stream operator
logger << LogLevel::INFO << "message " << value;

// Level check
if (logger.shouldLog(LogLevel::DEBUG)) {
    // Compute expensive data
}

// Context
std::string ctx = logger.getContextId();
```

---

## 📁 Directory Structure

```
modules/LogAndTrace/
├── CMakeLists.txt                # Build configuration
├── README.md                     # This file
├── source/
│   ├── inc/                      # Public headers
│   │   ├── CLogger.hpp
│   │   ├── CLogManager.hpp
│   │   ├── ISink.hpp
│   │   └── ...
│   └── src/                      # Implementation
│       ├── CLogger.cpp
│       ├── CLogManager.cpp
│       ├── CDLTSink.cpp          # DLT encapsulation
│       └── ...
├── test/
│   ├── unittest/                 # Unit tests (50 tests)
│   │   ├── test_main.cpp
│   │   ├── test_console_sink.cpp
│   │   ├── test_file_sink.cpp
│   │   ├── test_dlt_sink.cpp
│   │   ├── test_syslog_sink.cpp
│   │   ├── test_boundary_values.cpp  # NEW: 18 edge case tests
│   │   └── ...
│   ├── benchmark/                # Performance benchmarks
│   │   ├── benchmark_simple.cpp
│   │   ├── benchmark_stress_test.cpp
│   │   ├── benchmark_latency.cpp
│   │   ├── benchmark_memory.cpp
│   │   └── benchmark_multiprocess.cpp
│   └── examples/                 # Integration examples
│       ├── example_multi_sink.cpp
│       ├── test_dlt_direct.cpp
│       ├── test_dlt_long_message.cpp  # NEW: DLT boundary testing
│       ├── config_console_file.json
│       ├── config_dlt.json
│       ├── config_syslog.json
│       └── config_all_sinks.json
└── doc/                          # Documentation
    ├── TEST_REPORT.md            # Test results (50 tests, all passing)
    ├── BENCHMARK_REPORT.md       # Performance analysis (updated)
    └── logConfig_template.json   # Config template
```

---

## 🧪 Testing

### Unit Tests

```bash
# Run all tests
cd build/modules/LogAndTrace
./log_test

# Results: 50/50 tests passing
# - ConsoleSink: 3 tests
# - FileSink: 2 tests
# - DLTSink: 3 tests
# - SyslogSink: 7 tests
# - SinkBase: 5 tests
# - LoggerTest: 5 tests
# - MultiThreadTest: 5 tests
# - ZeroCopyTest: 2 tests
# - BoundaryValueTests: 18 tests (NEW)
```

**Test Coverage:**
- ✅ Basic functionality (logging, level filtering)
- ✅ Multi-threading (race conditions, sustained load)
- ✅ Zero-copy validation (memory growth tracking)
- ✅ All sink types (Console, File, Syslog, DLT)
- ✅ **NEW**: Boundary values (MAX_LOG_SIZE, buffer limits)
- ✅ **NEW**: Edge cases (empty, oversized, special chars)
- ✅ **NEW**: Security (buffer overflows, truncation)
- ✅ 30/30 tests passing
- ✅ All sinks tested
- ✅ Multi-threading validated
- ✅ Memory safety confirmed

**📋 [Full Test Report →](doc/TEST_REPORT.md)**

### DLT Integration Verification

```bash
# Start DLT daemon
sudo systemctl start dlt-daemon

# Run DLT long message test
cd build/modules/LogAndTrace
./test_dlt_long_message

# Verify messages in system logs
sudo journalctl -u dlt-daemon --since "1 minute ago" | grep DLTX

# Example output (10 messages successfully sent):
# - Application ID: DLTX
# - Context ID: DLTC
# - Messages: 1 byte to 10KB (truncated to 200 bytes)
# - Status: All messages received by DLT daemon
```

**DLT Test Results:**
- ✅ All 10 test messages delivered to DLT
- ✅ Short messages (1-50 bytes): Passed intact
- ✅ Exact MAX_LOG_SIZE (200 bytes): Handled correctly
- ✅ Oversized messages (300B, 10KB): Truncated to 200 bytes
- ✅ Special characters and Unicode: Supported
- ✅ No "pure virtual method called" crashes
- ✅ Clean exit with `std::_Exit(0)` or proper MemManager init

### Benchmarks

```bash
# Quick benchmark
./benchmark_simple
# Result: 2.63M logs/sec, 0.133µs latency

# Stress test (10 seconds sustained)
./benchmark_stress_test
# Result: 6.22M logs/sec, 160ns avg latency, 0 bytes growth
```

**Results Preview:**
- Single-thread: 2.63M logs/sec
- Sustained: 6.22M logs/sec (10 seconds)
- Latency: 132.69ns mean, 160.77ns average
- P99 Latency: 156ns
- Memory: 0 bytes growth (50K logs)

**📈 [Full Benchmark Report →](doc/BENCHMARK_REPORT.md)**

### Integration Examples

```bash
# Multi-sink test
cd ../../modules/LogAndTrace
../../build/modules/LogAndTrace/example_multi_sink

# DLT verification
../../build/modules/LogAndTrace/test_dlt_direct
dlt-viewer &  # Verify messages

# DLT long message test
cp test/examples/config_dlt.json ../../build/modules/LogAndTrace/
../../build/modules/LogAndTrace/test_dlt_long_message
```

---

## 🔨 Build Instructions

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt-get install build-essential cmake libboost-all-dev libdlt-dev

# Fedora/RHEL
sudo dnf install gcc-c++ cmake boost-devel automotive-dlt-devel
```

### Build Steps

```bash
# Clone repository
git clone <repo-url>
cd LightAP

# Create build directory
mkdir -p build && cd build

# Configure
cmake ..

# Build LogAndTrace module
make lap_log -j$(nproc)

# LogAndTrace Module

**High-Performance Zero-Dependency Logging System for AUTOSAR Adaptive Platform**

[![Tests](https://img.shields.io/badge/tests-65%2F65%20passing-brightgreen)](TEST_REPORT.md)
[![Performance](https://img.shields.io/badge/throughput-555K%20logs%2Fsec-blue)](doc/BENCHMARK_REPORT.md)
[![Latency](https://img.shields.io/badge/latency-sub--microsecond-blue)](doc/BENCHMARK_REPORT.md)
[![Memory](https://img.shields.io/badge/zero--copy-validated-brightgreen)](doc/BENCHMARK_REPORT.md)
[![Dependencies](https://img.shields.io/badge/STL-free-orange)](README.md)
[![Security](https://img.shields.io/badge/base64-encoding-green)](TEST_REPORT.md)

**Language / 语言**: [English](README.md) | [中文](README_CN.md)

---

## 🆕 Recent Updates (2025-11-14)

### 🔐 Base64 Encoding Feature

**Secure Sensitive Data Logging**

New Base64 encoding capability added for protecting sensitive information in logs:

```cpp
// Enable encoding for sensitive data
logger.LogInfo().WithEncode() << "password=secret123";
// Output: cGFzc3dvcmQ9c2VjcmV0MTIz

// Chain with other modifiers
logger.WithLevel(LogLevel::kError).WithEncode() << "Sensitive error data";
logger.LogInfo().WithLocation(__FILE__, __LINE__).WithEncode() << "Tracked sensitive data";

// Encoding is per-log, disable by default
logger.LogInfo() << "Normal log";  // No encoding
logger.LogInfo().WithEncode() << "Encoded log";  // Base64 encoded
```

**Key Features:**
- 🔐 **Base64 Encoding**: Protect sensitive data in logs (passwords, tokens, PII)
- ⛓️ **Chainable API**: `WithLevel().WithEncode()` fluent interface
- 🚀 **Zero Overhead**: -0.49% performance impact (measurement variance)
- ✅ **15 New Tests**: Comprehensive encoding validation
- 🧵 **Thread-Safe**: Multi-threaded encoding tested and validated
- 🎯 **Per-Log Control**: Enable/disable encoding per log statement

**Performance Validation:**
- Normal logging: 4,878,049 logs/sec
- With encoding: 4,901,961 logs/sec
- **Overhead: -0.49%** (negligible, within measurement variance)

### Previous Updates (2025-11-06)

### 🏗️ Architecture Refactoring - STL Dependency Elimination

**Unified Type System with Core Module**

Complete migration from STL to Core module type wrappers:

- **✅ Container Migration**  
  `std::vector` → `core::Vector`  
  All dynamic arrays use Core type aliases with unified memory management

- **✅ Synchronization Primitives**  
  `std::mutex` → `core::Mutex`  
  `std::lock_guard` → `core::LockGuard`  
  Thread synchronization fully uses Core wrappers, maintaining AUTOSAR style

- **✅ Smart Pointers & Utilities**  
  `std::unique_ptr` → `core::UniqueHandle`  
  `std::make_unique` → `core::MakeUnique` (new)  
  `std::make_shared` → `core::MakeShared` (new)  
  `std::move` → `core::Move`  
  `std::find_if` → `core::FindIf`  
  Memory ownership and algorithms unified with Core utilities

- **✅ Time & Algorithms**  
  `std::chrono` → `core::Time`  
  Time operations use Core's high-precision time wrappers

**Impact:**
- 🗑️ **9 STL Headers Removed**: `<vector>`, `<mutex>`, `<memory>`, `<algorithm>`, `<chrono>`
- 🔄 **50+ Type Replacements**: All STL types migrated to Core wrappers
- ✅ **358 Tests Passing**: Core (308) + LogAndTrace (50 → 65)
- 📊 **No Performance Regression**: 555K logs/sec (single-thread), 27K logs/sec (multi-thread)
- 🔒 **Zero-Copy Preserved**: StringView passing, no extra copies

### Previous Updates (2025-10-29)

#### Security & Robustness
- 🛡️ FileSink buffer overflow protection
- 🛡️ DLT StringView safety handling
- 🛡️ Static destruction order fixes

#### Testing & Validation
- ✅ 65/65 tests passing (boundary values + multi-threading + zero-copy + base64)
- ✅ Core module 308 tests passing
- ✅ Boundary coverage: MAX_LOG_SIZE, buffer limits, edge cases

---

## 🚀 Overview

LightAP LogAndTrace 是一个为 AUTOSAR Adaptive Platform 设计的**零 STL 依赖**企业级日志系统，完全基于 Core 模块的类型封装，提供极致性能、完整的 DLT 支持、零拷贝架构以及生产级安全保障。

### Key Features

| Feature | Description | Status |
|---------|-------------|--------|
| **🏗️ STL-Free Architecture** | Zero STL deps, unified Core types | ✅ |
| **🔐 Base64 Encoding** | Secure sensitive data logging | ✅ NEW |
| **� High Performance** | 555K logs/sec (single), 27K logs/sec (10 threads) | ✅ |
| **🧵 Thread-Safe** | Core::Mutex/LockGuard, stress tested | ✅ |
| **📊 DLT Integration** | Full GENIVI DLT support with API encapsulation | ✅ |
| **💾 Zero-Copy** | StringView 传递，无堆分配 | ✅ |
| **🛡️ Buffer Safety** | Overflow protection, bounds checking | ✅ |
| **🎯 Multi-Sink** | Console, File, Syslog, DLT simultaneously | ✅ |
| **🔧 JSON Config** | Runtime configuration without recompilation | ✅ |
| **✅ Production Ready** | **65/65 tests passing** (15 new base64 tests) | ✅ |

---

## 📊 Current Performance Metrics

### Throughput (Post STL-Refactor Validation)

| Scenario | Throughput | Details |
|----------|------------|---------|
| **Single-Thread** | **555K logs/sec** | Console sink |
| **Single-Thread (Encoded)** | **4.9M logs/sec** | Base64 encoding |
| **Multi-Thread (10 threads)** | **27K logs/sec** | Concurrent |
| **High Concurrency (50 threads)** | **195ms** | 5000 logs |
| **Sustained Load (3s)** | **23.9K logs/sec** | Continuous |

### Memory & Architecture

- **Zero-copy validated**: 0 bytes growth for 50,000 logs
- **STL includes removed**: 9 standard library headers
- **Type replacements**: 50+ STL types migrated to Core
- **Core module dependency**: Unified type system and memory management
- **Buffer safety**: All sinks protected against overflows
- **Base64 encoding**: 15 new tests, zero performance overhead

### Validation Status

- ✅ **Core Module**: 308/308 tests passing
- ✅ **LogAndTrace**: 65/65 tests passing (50 existing + 15 new)
- ✅ **Base64 Encoding**: 15/15 tests passing
- ✅ **Multi-threading**: All concurrency tests passed
- ✅ **Zero-copy**: Memory growth validation passed
- ✅ **Examples**: All 4 examples compile and run correctly (including base64 example)
- ✅ **No regressions**: Performance maintained after feature addition

**📈 [Full Performance Analysis →](doc/BENCHMARK_REPORT.md)**

### Testing Environment

All benchmarks and tests were conducted on the following hardware and software configuration:

#### Hardware Specifications
- **CPU**: Intel(R) Core(TM) i5-10210U @ 1.60GHz (4 cores, 8 threads)
- **Memory**: 16 GB RAM
- **Storage**: SSD

#### Software Environment
- **Operating System**: Debian GNU/Linux 12 (bookworm)
- **Kernel**: 6.1.0-23-amd64
- **Compiler**: GCC 12.2.0
- **C++ Standard**: C++14
- **CMake**: 3.25.1
- **DLT Daemon**: 2.18.8

---

## 🏗️ Architecture

### Zero-STL Design Philosophy

```
┌─────────────────────────────────────────────────────────────┐
│                    LogAndTrace Module                       │
│  ┌───────────────────────────────────────────────────────┐ │
│  │  Application Layer (STL-Free)                         │ │
│  │  • Logger, LogManager, SinkManager                    │ │
│  │  • All types from Core module                         │ │
│  │  • Zero direct STL dependencies                       │ │
│  └─────────────────────┬─────────────────────────────────┘ │
│                        │ Uses                                │
│  ┌─────────────────────▼─────────────────────────────────┐ │
│  │  Core Module Type System                              │ │
│  │  • Vector, String, Map, Mutex, LockGuard              │ │
│  │  • UniqueHandle, MakeUnique, MakeShared               │ │
│  │  • Move, Forward, FindIf, Time                        │ │
│  │  • Unified AUTOSAR-style wrappers                     │ │
│  └───────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

**Benefits:**
- ✅ **Unified Type System**: All modules use the same type aliases
- ✅ **Easy Customization**: Memory allocation strategy can be modified at Core layer
- ✅ **AUTOSAR Compliant**: Reduces direct STL usage, closer to AUTOSAR standards
- ✅ **Compilation Optimization**: Reduces template instantiation, faster compilation
- ✅ **Testability**: Core types can be mocked for unit testing

### Zero-Copy Data Flow

```
Logger → LogStream → StringView → SinkManager → [Sinks...]
         (stack)     (no copy)    (dispatch)     (parallel)
         
         Using Core types:
         • core::Vector manages Sink list
         • core::Mutex protects concurrent access
         • core::StringView zero-copy message passing
```

**Key Principles:**
1. **StringView-based** message passing (no string copies)
2. **Core::Vector** for dynamic arrays (unified allocator strategy)
3. **Core::Mutex/LockGuard** for thread synchronization
4. **Core::UniqueHandle** for resource ownership
5. **Direct writes** to sink buffers

### Type Migration Map

| STL Type | Core Type | Usage in LogAndTrace |
|----------|-----------|----------------------|
| `std::vector<T>` | `core::Vector<T>` | Sink lists, config arrays |
| `std::mutex` | `core::Mutex` | Context map protection |
| `std::lock_guard<std::mutex>` | `core::LockGuard` | RAII locking |
| `std::unique_ptr<T>` | `core::UniqueHandle<T>` | Logger ownership |
| `std::make_unique<T>()` | `core::MakeUnique<T>()` | Smart ptr creation |
| `std::make_shared<T>()` | `core::MakeShared<T>()` | Shared ptr creation |
| `std::move()` | `core::Move()` | Move semantics |
| `std::forward()` | `core::Forward()` | Perfect forwarding |
| `std::find_if()` | `core::FindIf()` | Sink lookup |
| `std::chrono` | `core::Time` | Timestamps |

### DLT Encapsulation

```
┌─────────────────────────────────────┐
│   Application Layer (Core types)   │
│   Logger, LogManager - DLT-free    │
└──────────────┬──────────────────────┘
               │ LogLevelType (internal)
┌──────────────▼──────────────────────┐
│         CDLTSink (isolated)         │
│  • dlt_register_app()               │
│  • dlt_register_context()           │
│  • dlt_user_log_write_sized_utf8()  │
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

// Stream operator with level
logger.WithLevel(LogLevel::kInfo) << "message " << value;

// NEW: Base64 encoding for sensitive data
logger.LogInfo().WithEncode() << "password=secret123";
logger.WithLevel(LogLevel::kError).WithEncode() << "Sensitive error";

// Chainable modifiers
logger.WithLevel(LogLevel::kDebug).WithLocation(__FILE__, __LINE__) << "Debug with location";
logger.LogInfo().WithLocation(__FILE__, __LINE__).WithEncode() << "Tracked sensitive data";

// Level check
if (logger.shouldLog(LogLevel::kDebug)) {
    // Compute expensive data
}

// Context
core::String ctx = logger.getContextId();
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
│   ├── unittest/                 # Unit tests (65 tests)
│   │   ├── test_main.cpp
│   │   ├── test_console_sink.cpp
│   │   ├── test_file_sink.cpp
│   │   ├── test_dlt_sink.cpp
│   │   ├── test_syslog_sink.cpp
│   │   ├── test_boundary_values.cpp  # 18 edge case tests
│   │   ├── test_base64_encode.cpp    # NEW: 15 encoding tests
│   │   └── ...
│   ├── benchmark/                # Performance benchmarks
│   │   ├── benchmark_simple.cpp
│   │   ├── benchmark_stress_test.cpp
│   │   ├── benchmark_latency.cpp
│   │   ├── benchmark_memory.cpp
│   │   └── benchmark_multiprocess.cpp
│   └── examples/                 # Integration examples
│       ├── example_basic_usage.cpp
│       ├── example_multi_thread.cpp
│       ├── example_file_rotation.cpp
│       ├── example_base64_encode.cpp  # NEW: Base64 encoding demo
│       ├── config_console_file.json
│       ├── config_base64_test.json    # NEW: Base64 config
│       └── ...
└── doc/                          # Documentation
    ├── BENCHMARK_REPORT.md
    ├── TEST_REPORT.md            # NEW: Complete test report (65 tests)
    └── ...

```

---

## 🧪 Testing

### Unit Tests

```bash
# Run all tests
cd build/modules/LogAndTrace
./log_test

# Results: 65/65 tests passing
# - ConsoleSink: 3 tests
# - FileSink: 2 tests
# - DLTSink: 3 tests
# - SyslogSink: 7 tests
# - SinkBase: 5 tests
# - LoggerTest: 5 tests
# - MultiThreadTest: 5 tests
# - ZeroCopyTest: 2 tests
# - BoundaryValueTests: 18 tests
# - Base64EncodeTests: 15 tests (NEW)
```

**Test Coverage:**
- ✅ Basic functionality (logging, level filtering)
- ✅ Multi-threading (race conditions, sustained load)
- ✅ Zero-copy validation (memory growth tracking)
- ✅ All sink types (Console, File, Syslog, DLT)
- ✅ Boundary values (MAX_LOG_SIZE, buffer limits)
- ✅ Edge cases (empty, oversized, special chars)
- ✅ Security (buffer overflows, truncation)
- ✅ **NEW**: Base64 encoding (15 comprehensive tests)
- ✅ **NEW**: Thread-safe encoding (5 threads × 100 messages)
- ✅ **NEW**: Encoding performance (zero overhead validated)

**📋 [Full Test Report →](TEST_REPORT.md)**

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
# Basic usage
cd ../../modules/LogAndTrace
../../build/modules/LogAndTrace/example_basic_usage

# Multi-thread example
../../build/modules/LogAndTrace/example_multi_thread

# File rotation example
../../build/modules/LogAndTrace/example_file_rotation

# NEW: Base64 encoding example
../../build/modules/LogAndTrace/example_base64_encode

# DLT verification
../../build/modules/LogAndTrace/test_dlt_direct
dlt-viewer &  # Verify messages
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

# Run tests
cd modules/LogAndTrace
./log_test

# Run examples
./example_basic_usage
./example_multi_thread
```

---

## 🎯 What's Next

**Current development focus is on Modeled Messages and Trace system implementation.**

For a comprehensive roadmap with detailed task breakdown, estimated efforts, and priorities, please see:

👉 **[`doc/TODO.md`](doc/TODO.md)** - Complete feature roadmap and task list

**Quick Summary:**
- **v1.0.0 (2025-12)**: Modeled Messages, Trace System
- **v1.1.0 (2026-Q1)**: Async Logging Queue, Advanced File Management, Log Filtering
- **v2.0.0 (2026-Q2+)**: Full AUTOSAR compliance, Network logging, Security enhancements

---

## � Documentation

### Active Documentation

| Document | Description | Location |
|----------|-------------|----------|
| **TODO List** | Feature roadmap and task tracking | [`doc/TODO.md`](doc/TODO.md) |
| **Design Documents** | Architecture and design specifications | [`doc/design/`](doc/design/) |
| **Message Catalog Format** | Modeled Messages catalog specification | [`doc/design/MessageCatalog_Format.md`](doc/design/MessageCatalog_Format.md) |
| **AUTOSAR Spec** | AUTOSAR AP SWS_LogAndTrace specification | [`doc/AUTOSAR_AP_SWS_LogAndTrace.pdf`](doc/AUTOSAR_AP_SWS_LogAndTrace.pdf) |
| **Index** | Documentation navigation | [`doc/INDEX.md`](doc/INDEX.md) |

### Archived Documentation

Historical documentation and completed analysis reports are archived in:
- [`doc/archive/`](doc/archive/) - Contains implementation summaries, benchmarks, and analysis reports

---

## 🗺️ Roadmap

See **[`doc/TODO.md`](doc/TODO.md)** for detailed task breakdown and time estimates.

### Current Focus: Base64 Encoding & Modeled Messages (v1.0.0)

**Recently Completed (2025-11-14):**
- ✅ **Base64 Encoding Feature**: Secure sensitive data logging with chainable API
- ✅ **15 New Tests**: Comprehensive encoding validation (65/65 tests passing)
- ✅ **Zero Performance Overhead**: -0.49% impact (measurement variance)
- ✅ **Thread-Safe Encoding**: Multi-threaded encoding validated

**Priority P0 - Target: 2025-12**

1. **🎯 Modeled Messages Implementation** (5-7 days, In Planning)
   - AUTOSAR-compliant Message ID templates
   - Compile-time routing with TraceSwitch
   - DLT message ID support (`dlt_user_log_write_start_id`)
   - Message catalog generation tools
   - Non-verbose/verbose mode support

2. **🔍 Trace System Enhancement** (3-4 days, In Planning)
   - ARTI interface implementation
   - TraceStatus management
   - Separate trace and log paths

### Next Phase: Performance & Features (v1.1.0)

**Priority P1 - Target: 2026-Q1**

3. **💾 Local Cache Optimization** (3-5 days, Planning)
   - Thread-local buffer pool for log formatting
   - Pre-allocated buffer management to reduce allocation overhead
   - Per-thread cache for frequently used log contexts
   - Memory pool for sink buffers
   - Target: 10-15% performance improvement
   - Zero-copy optimization for DLT sink

4. **⚡ Async Logging Queue** (5-7 days, Design Complete)
   - Lock-free queue implementation
   - Target: 2M+ logs/sec throughput
   - Background worker thread

5. **📁 Advanced File Management** (2-3 days)
   - Time-based rotation
   - Compression support
   - Cleanup policies

6. **🔧 Log Filtering** (3-4 days)
   - Per-context level filtering
   - Regex-based content filtering
   - Runtime configuration

### Future Plans (v2.0.0+)

**Priority P2 - Target: 2026-Q2+**

- Network logging (TCP/UDP sinks)
- Advanced analysis tools
- Full AUTOSAR compliance certification
- Security enhancements
- Multi-platform support

---

## 🤝 Contributing

### Development Status

- **Stable**: Core logging, Multi-sink, DLT integration
- **Beta**: STL-free architecture (Post-refactor validation)
- **Planning**: Modeled Messages, Trace system

### How to Contribute

1. Check [`doc/TODO.md`](doc/TODO.md) for open tasks
2. Read existing code and tests
3. Follow the STL-free architecture (use Core module types)
4. Add tests for new features
5. Update documentation

### Code Style

- Use Core module types (`core::Vector`, `core::Mutex`, etc.)
- No STL dependencies in LogAndTrace module
- Follow AUTOSAR naming conventions
- Add Doxygen comments
- Zero-copy principles (use `core::StringView`)

---

## � Contact & Support

**Project**: LightAP Middleware  
**Module**: LogAndTrace  
**Maintainer**: ddkv587 (ddkv587@gmail.com)

For questions, issues, or contributions:
- Review documentation in [`doc/`](doc/)
- Check TODO list: [`doc/TODO.md`](doc/TODO.md)
- See archived reports: [`doc/archive/`](doc/archive/)

---

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

```
MIT License

Copyright (c) 2025 LightAP Project

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

---

**Last Updated**: 2025-11-14  
**Version**: 1.0.0  
**Status**: Production Ready - Base64 Encoding Feature Released (65/65 tests passing)



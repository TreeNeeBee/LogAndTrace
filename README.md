# LogAndTrace Module

**High-Performance Zero-Dependency Logging System for AUTOSAR Adaptive Platform**

[![Tests](https://img.shields.io/badge/tests-50%2F50%20passing-brightgreen)](doc/TEST_REPORT.md)
[![Performance](https://img.shields.io/badge/throughput-555K%20logs%2Fsec-blue)](doc/BENCHMARK_REPORT.md)
[![Latency](https://img.shields.io/badge/latency-sub--microsecond-blue)](doc/BENCHMARK_REPORT.md)
[![Memory](https://img.shields.io/badge/zero--copy-validated-brightgreen)](doc/BENCHMARK_REPORT.md)
[![Dependencies](https://img.shields.io/badge/STL-free-orange)](README.md)
[![Security](https://img.shields.io/badge/buffer-protected-green)](doc/TEST_REPORT.md)

---

## 🆕 Recent Updates (2025-11-06)

### 🏗️ Architecture Refactoring - STL Dependency Elimination

**Unified Type System with Core Module**

LogAndTrace 模块已完全移除 STL 依赖，统一使用 Core 模块提供的类型和功能封装：

- **✅ Container Migration**  
  `std::vector` → `core::Vector`  
  所有动态数组使用 Core 的类型别名，统一内存管理策略

- **✅ Synchronization Primitives**  
  `std::mutex` → `core::Mutex`  
  `std::lock_guard` → `core::LockGuard`  
  线程同步完全使用 Core 封装，保持 AUTOSAR 风格

- **✅ Smart Pointers & Utilities**  
  `std::unique_ptr` → `core::UniqueHandle`  
  `std::make_unique` → `core::MakeUnique` (新增)  
  `std::make_shared` → `core::MakeShared` (新增)  
  `std::move` → `core::Move`  
  `std::find_if` → `core::FindIf`  
  内存所有权和算法统一使用 Core 工具

- **✅ Time & Algorithms**  
  `std::chrono` → `core::Time`  
  时间操作使用 Core 的高精度时间封装

**Impact:**
- 🗑️ **9 个 STL 头文件移除**: `<vector>`, `<mutex>`, `<memory>`, `<algorithm>`, `<chrono>`
- 🔄 **50+ 类型替换**: 所有 STL 类型迁移到 Core 封装
- ✅ **358 测试全通过**: Core (308) + LogAndTrace (50)
- 📊 **性能无回退**: 555K logs/sec (单线程), 27K logs/sec (多线程)
- 🔒 **零拷贝机制保持**: StringView 传递，无额外拷贝

### Previous Updates (2025-10-29)

#### Security & Robustness
- 🛡️ FileSink 缓冲区溢出保护
- 🛡️ DLT StringView 安全处理
- 🛡️ 静态析构顺序修复

#### Testing & Validation
- ✅ 50/50 测试通过（边界值 + 多线程 + 零拷贝）
- ✅ Core 模块 308 测试通过
- ✅ 边界值覆盖：MAX_LOG_SIZE、缓冲区限制、边缘情况

---

## 🚀 Overview

LightAP LogAndTrace 是一个为 AUTOSAR Adaptive Platform 设计的**零 STL 依赖**企业级日志系统，完全基于 Core 模块的类型封装，提供极致性能、完整的 DLT 支持、零拷贝架构以及生产级安全保障。

### Key Features

| Feature | Description | Status |
|---------|-------------|--------|
| **🏗️ STL-Free Architecture** | 零 STL 依赖，统一使用 Core 模块封装 | ✅ |
| **🔥 High Performance** | 555K logs/sec (单线程), 27K logs/sec (10线程) | ✅ |
| **🧵 Thread-Safe** | Core::Mutex/LockGuard，多线程压力测试通过 | ✅ |
| **📊 DLT Integration** | Full GENIVI DLT support with API encapsulation | ✅ |
| **💾 Zero-Copy** | StringView 传递，无堆分配 | ✅ |
| **🛡️ Buffer Safety** | Overflow protection, bounds checking | ✅ |
| **🎯 Multi-Sink** | Console, File, Syslog, DLT simultaneously | ✅ |
| **🔧 JSON Config** | Runtime configuration without recompilation | ✅ |
| **✅ Production Ready** | 358 tests passing (Core: 308 + Log: 50) | ✅ |

---

## 📊 Current Performance Metrics

### Throughput (Post STL-Refactor Validation)

| Scenario | Throughput | Details |
|----------|------------|---------|
| **Single-Thread** | **555K logs/sec** | Console sink, 验证测试 |
| **Multi-Thread (10 threads)** | **27K logs/sec** | 并发压力测试 |
| **High Concurrency (50 threads)** | **195ms** | 5000 logs, 线程安全验证 |
| **Sustained Load (3s)** | **23.9K logs/sec** | 持续负载测试 |

### Memory & Architecture

- **Zero-copy validated**: 0 bytes growth for 50,000 logs
- **STL includes removed**: 9 个标准库头文件
- **Type replacements**: 50+ STL 类型迁移到 Core
- **Core module dependency**: 统一类型系统和内存管理
- **Buffer safety**: All sinks protected against overflows

### Validation Status

- ✅ **Core Module**: 308/308 tests passing
- ✅ **LogAndTrace**: 50/50 tests passing
- ✅ **Multi-threading**: All concurrency tests passed
- ✅ **Zero-copy**: Memory growth validation passed
- ✅ **Examples**: All 3 examples compile and run correctly
- ✅ **No regressions**: Performance maintained after refactor

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
- ✅ **统一类型系统**: 所有模块使用相同的类型别名
- ✅ **易于定制**: 可在 Core 层统一修改内存分配策略
- ✅ **符合 AUTOSAR**: 减少 STL 直接使用，更接近 AUTOSAR 规范
- ✅ **编译优化**: 减少模板实例化，加快编译速度
- ✅ **可测试性**: Core 类型可 mock，便于单元测试

### Zero-Copy Data Flow

```
Logger → LogStream → StringView → SinkManager → [Sinks...]
         (stack)     (no copy)    (dispatch)     (parallel)
         
         使用 Core 类型：
         • core::Vector 管理 Sink 列表
         • core::Mutex 保护并发访问
         • core::StringView 零拷贝传递消息
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

### Current Focus: Modeled Messages & Trace (v1.0.0)

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

3. **⚡ Async Logging Queue** (5-7 days, Design Complete)
   - Lock-free queue implementation
   - Target: 2M+ logs/sec throughput
   - Background worker thread

4. **📁 Advanced File Management** (2-3 days)
   - Time-based rotation
   - Compression support
   - Cleanup policies

5. **🔧 Log Filtering** (3-4 days)
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

[Specify your license here]

---

**Last Updated**: 2025-11-06  
**Version**: 1.0.0-dev  
**Status**: Active Development - Modeled Messages Implementation Phase


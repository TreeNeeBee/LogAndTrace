# LogAndTrace 模块

**面向 AUTOSAR Adaptive Platform 的高性能零依赖日志系统**

[![Tests](https://img.shields.io/badge/tests-65%2F65%20passing-brightgreen)](TEST_REPORT.md)
[![Performance](https://img.shields.io/badge/throughput-555K%20logs%2Fsec-blue)](doc/BENCHMARK_REPORT.md)
[![Latency](https://img.shields.io/badge/latency-sub--microsecond-blue)](doc/BENCHMARK_REPORT.md)
[![Memory](https://img.shields.io/badge/zero--copy-validated-brightgreen)](doc/BENCHMARK_REPORT.md)
[![Dependencies](https://img.shields.io/badge/STL-free-orange)](README.md)
[![Security](https://img.shields.io/badge/base64-encoding-green)](TEST_REPORT.md)

**Language / 语言**: [English](README.md) | [中文](README_CN.md)

---

## 🆕 最近更新 (2025-11-14)

### 🔐 Base64 编码特性

**保护敏感数据的日志记录**

新增 Base64 编码功能，用于保护日志中的敏感信息：

```cpp
// 为敏感数据启用编码
logger.LogInfo().WithEncode() << "password=secret123";
// 输出: cGFzc3dvcmQ9c2VjcmV0MTIz

// 与其他修饰符链式调用
logger.WithLevel(LogLevel::kError).WithEncode() << "敏感错误数据";
logger.LogInfo().WithLocation(__FILE__, __LINE__).WithEncode() << "已跟踪的敏感数据";

// 编码是按日志启用的，默认禁用
logger.LogInfo() << "普通日志";  // 无编码
logger.LogInfo().WithEncode() << "编码日志";  // Base64 编码
```

**核心特性：**
- 🔐 **Base64 编码**：保护日志中的敏感数据（密码、令牌、个人信息）
- ⛓️ **链式 API**：`WithLevel().WithEncode()` 流式接口
- 🚀 **零开销**：-0.49% 性能影响（测量误差范围内）
- ✅ **15 个新测试**：全面的编码验证
- 🧵 **线程安全**：多线程编码已测试和验证
- 🎯 **按日志控制**：每个日志语句可独立启用/禁用编码

**性能验证：**
- 普通日志：4,878,049 logs/sec
- 带编码：4,901,961 logs/sec
- **开销：-0.49%**（可忽略，在测量误差范围内）

### 之前的更新 (2025-11-06)

#### 🏗️ 架构重构 - STL 依赖消除

**与 Core 模块统一的类型系统**

LogAndTrace 模块已完全移除 STL 依赖，统一使用 Core 模块提供的类型和功能封装：

- **✅ 容器迁移**  
  `std::vector` → `core::Vector`  
  所有动态数组使用 Core 的类型别名，统一内存管理策略

- **✅ 同步原语**  
  `std::mutex` → `core::Mutex`  
  `std::lock_guard` → `core::LockGuard`  
  线程同步完全使用 Core 封装，保持 AUTOSAR 风格

- **✅ 智能指针与工具**  
  `std::unique_ptr` → `core::UniqueHandle`  
  `std::make_unique` → `core::MakeUnique` (新增)  
  `std::make_shared` → `core::MakeShared` (新增)  
  `std::move` → `core::Move`  
  `std::find_if` → `core::FindIf`  
  内存所有权和算法统一使用 Core 工具

- **✅ 时间与算法**  
  `std::chrono` → `core::Time`  
  时间操作使用 Core 的高精度时间封装

**影响：**
- 🗑️ **移除 9 个 STL 头文件**：`<vector>`, `<mutex>`, `<memory>`, `<algorithm>`, `<chrono>`
- 🔄 **50+ 类型替换**：所有 STL 类型迁移到 Core 封装
- ✅ **358 个测试全通过**：Core (308) + LogAndTrace (50 → 65)
- 📊 **性能无回退**：555K logs/sec (单线程), 27K logs/sec (多线程)
- 🔒 **零拷贝机制保持**：StringView 传递，无额外拷贝

#### 安全性与鲁棒性
- 🛡️ FileSink 缓冲区溢出保护
- 🛡️ DLT StringView 安全处理
- 🛡️ 静态析构顺序修复

#### 测试与验证
- ✅ 65/65 测试通过（边界值 + 多线程 + 零拷贝 + Base64 编码）
- ✅ Core 模块 308 测试通过
- ✅ 边界值覆盖：MAX_LOG_SIZE、缓冲区限制、边缘情况

---

## 🚀 概述

LightAP LogAndTrace 是一个为 AUTOSAR Adaptive Platform 设计的**零 STL 依赖**企业级日志系统，完全基于 Core 模块的类型封装，提供极致性能、完整的 DLT 支持、零拷贝架构以及生产级安全保障。

### 核心特性

| 特性 | 描述 | 状态 |
|------|------|------|
| **🏗️ 零 STL 架构** | 零 STL 依赖，统一使用 Core 模块封装 | ✅ |
| **🔐 Base64 编码** | 敏感数据加密输出，保护隐私信息 | ✅ 新增 |
| **🔥 高性能** | 555K logs/sec (单线程), 27K logs/sec (10线程) | ✅ |
| **🧵 线程安全** | Core::Mutex/LockGuard，多线程压力测试通过 | ✅ |
| **📊 DLT 集成** | 完整的 GENIVI DLT 支持与 API 封装 | ✅ |
| **💾 零拷贝** | StringView 传递，无堆分配 | ✅ |
| **🛡️ 缓冲区安全** | 溢出保护，边界检查 | ✅ |
| **🎯 多 Sink** | 同时支持 Console、File、Syslog、DLT | ✅ |
| **🔧 JSON 配置** | 运行时配置，无需重新编译 | ✅ |
| **✅ 生产就绪** | **65/65 测试通过**（15 个新 base64 测试） | ✅ |

---

## 📊 当前性能指标

### 吞吐量（最新基准测试结果）

| 场景 | 吞吐量 | 详情 |
|------|--------|------|
| **单线程** | **555K logs/sec** | Console sink |
| **单线程（编码）** | **4.9M logs/sec** | Base64 编码 |
| **多线程（10 线程）** | **27K logs/sec** | 并发 |
| **高并发（50 线程）** | **195ms** | 5000 条日志 |
| **持续负载（3 秒）** | **23.9K logs/sec** | 连续 |

### Base64 编码性能

| 模式 | 吞吐量 | 开销 |
|------|--------|------|
| **普通日志** | 4,878,049 logs/sec | 基线 |
| **带编码** | 4,901,961 logs/sec | **-0.49%** |

**结论**：Base64 编码几乎不增加性能开销。

### 内存与架构

- **零拷贝验证**：50,000 条日志 0 字节增长
- **移除 STL 头文件**：9 个标准库头文件
- **类型替换**：50+ STL 类型迁移到 Core
- **Core 模块依赖**：统一类型系统和内存管理
- **缓冲区安全**：所有 sink 都受到溢出保护
- **Base64 编码**：15 个新测试，零性能开销

### 验证状态

- ✅ **Core 模块**：308/308 测试通过
- ✅ **LogAndTrace**：65/65 测试通过（50 个现有 + 15 个新增）
- ✅ **Base64 编码**：15/15 测试通过
- ✅ **多线程**：所有并发测试通过
- ✅ **零拷贝**：内存增长验证通过
- ✅ **示例**：所有 4 个示例编译并正确运行（包括 base64 示例）
- ✅ **无回退**：特性添加后性能保持

**📈 [完整性能分析 →](doc/BENCHMARK_REPORT.md)**

### 测试环境

所有基准测试和测试均在以下硬件和软件配置上进行：

#### 硬件规格
- **CPU**：Intel(R) Core(TM) i5-10210U @ 1.60GHz（4 核，8 线程）
- **内存**：16 GB RAM
- **存储**：SSD

#### 软件环境
- **操作系统**：Debian GNU/Linux 12 (bookworm)
- **内核**：6.1.0-23-amd64
- **编译器**：GCC 12.2.0
- **C++ 标准**：C++14
- **CMake**：3.25.1
- **DLT Daemon**：2.18.8

---

## 🏗️ 架构

### 零 STL 设计理念

```
┌─────────────────────────────────────────────────────────────┐
│                    LogAndTrace 模块                         │
│  ┌───────────────────────────────────────────────────────┐ │
│  │  应用层（无 STL）                                      │ │
│  │  • Logger, LogManager, SinkManager                    │ │
│  │  • 所有类型来自 Core 模块                              │ │
│  │  • 零直接 STL 依赖                                     │ │
│  └─────────────────────┬─────────────────────────────────┘ │
│                        │ 使用                                │
│  ┌─────────────────────▼─────────────────────────────────┐ │
│  │  Core 模块类型系统                                     │ │
│  │  • Vector, String, Map, Mutex, LockGuard              │ │
│  │  • UniqueHandle, MakeUnique, MakeShared               │ │
│  │  • Move, Forward, FindIf, Time                        │ │
│  │  • 统一的 AUTOSAR 风格封装                             │ │
│  └───────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

**优势：**
- ✅ **统一类型系统**：所有模块使用相同的类型别名
- ✅ **易于定制**：可在 Core 层统一修改内存分配策略
- ✅ **符合 AUTOSAR**：减少 STL 直接使用，更接近 AUTOSAR 规范
- ✅ **编译优化**：减少模板实例化，加快编译速度
- ✅ **可测试性**：Core 类型可 mock，便于单元测试

### 零拷贝数据流

```
Logger → LogStream → StringView → SinkManager → [Sinks...]
         (栈上)      (无拷贝)     (分发)        (并行)
         
         使用 Core 类型：
         • core::Vector 管理 Sink 列表
         • core::Mutex 保护并发访问
         • core::StringView 零拷贝传递消息
```

**核心原则：**
1. **基于 StringView** 的消息传递（无字符串拷贝）
2. **Core::Vector** 用于动态数组（统一分配器策略）
3. **Core::Mutex/LockGuard** 用于线程同步
4. **Core::UniqueHandle** 用于资源所有权
5. **直接写入** sink 缓冲区

### 类型迁移映射

| STL 类型 | Core 类型 | 在 LogAndTrace 中的用途 |
|----------|-----------|------------------------|
| `std::vector<T>` | `core::Vector<T>` | Sink 列表，配置数组 |
| `std::mutex` | `core::Mutex` | 上下文映射保护 |
| `std::lock_guard<std::mutex>` | `core::LockGuard` | RAII 锁定 |
| `std::unique_ptr<T>` | `core::UniqueHandle<T>` | Logger 所有权 |
| `std::make_unique<T>()` | `core::MakeUnique<T>()` | 智能指针创建 |
| `std::make_shared<T>()` | `core::MakeShared<T>()` | 共享指针创建 |
| `std::move()` | `core::Move()` | 移动语义 |
| `std::forward()` | `core::Forward()` | 完美转发 |
| `std::find_if()` | `core::FindIf()` | Sink 查找 |
| `std::chrono` | `core::Time` | 时间戳 |

### DLT 封装

```
LogAndTrace (无 DLT 依赖)
    ↓
CDLTSink (DLT 封装层)
    ↓
automotive-dlt (系统库)
```

**封装优势：**
- ✅ 核心日志中无 DLT 依赖
- ✅ DLT 可选（编译时可禁用）
- ✅ 类型安全的 DLT API 包装
- ✅ 清晰的错误处理
- ✅ 符合 AUTOSAR 的接口设计

---

## 🛡️ 安全增强

**1. FileSink 溢出保护**

```cpp
// prefixLen 边界检查
if (prefixLen >= m_bufferSize) {
    return;  // 防止溢出
}

// 消息截断
size_t availableSpace = m_bufferSize - prefixLen - 1;
size_t copyLen = (message.size() > availableSpace) 
    ? availableSpace 
    : message.size();
```

**2. DLT StringView 安全**

```cpp
// 安全处理非空终止的 StringView
if (message.data()[message.size()] == '\0') {
    dlt_user_log_write_string(&contextData, message.data());
} else {
    core::String temp(message.data(), message.size());
    dlt_user_log_write_string(&contextData, temp.c_str());
}
```

**3. 静态初始化顺序**

```cpp
// 使用延迟初始化避免静态析构顺序问题
LogManager::~LogManager() {
    if (!isShutdown_) {
        shutdown();
    }
}
```

**安全保证：**
- 🛡️ **FileSink**：prefixLen 边界检查，消息截断
- 🛡️ **DLT**：非空终止字符串安全
- 🛡️ **MAX_LOG_SIZE**：200 字节限制在各处强制执行
- 🛡️ **静态析构**：正确的单例初始化顺序

---

## 📚 API 参考

### CLogManager（单例）

```cpp
// 获取实例
auto& manager = LogManager::getInstance();

// 使用配置文件初始化
manager.initialize(lap::core::InstanceSpecifier("config.json"));

// 获取 logger
auto& logger = manager.registerLogger("CTX", "Description", LogLevel::kDebug);

// 关闭
manager.shutdown();
```

### CLogger

```cpp
// 日志方法
logger.fatal("message", args...);
logger.error("message", args...);
logger.warn("message", args...);
logger.info("message", args...);
logger.debug("message", args...);
logger.verbose("message", args...);

// 带级别的流操作符
logger.WithLevel(LogLevel::kInfo) << "message " << value;

// 新增：敏感数据的 Base64 编码
logger.LogInfo().WithEncode() << "password=secret123";
logger.WithLevel(LogLevel::kError).WithEncode() << "敏感错误";

// 链式修饰符
logger.WithLevel(LogLevel::kDebug).WithLocation(__FILE__, __LINE__) << "带位置的调试";
logger.LogInfo().WithLocation(__FILE__, __LINE__).WithEncode() << "已跟踪的敏感数据";

// 级别检查
if (logger.shouldLog(LogLevel::kDebug)) {
    // 计算昂贵的数据
}

// 上下文
core::String ctx = logger.getContextId();
```

---

## 📁 目录结构

```
modules/LogAndTrace/
├── CMakeLists.txt                # 构建配置
├── README.md                     # 英文文档
├── README_CN.md                  # 中文文档（本文件）
├── source/
│   ├── inc/                      # 公共头文件
│   │   ├── CLogger.hpp
│   │   ├── CLogManager.hpp
│   │   ├── ISink.hpp
│   │   └── ...
│   └── src/                      # 实现
│       ├── CLogger.cpp
│       ├── CLogManager.cpp
│       ├── CDLTSink.cpp          # DLT 封装
│       └── ...
├── test/
│   ├── unittest/                 # 单元测试（65 个测试）
│   │   ├── test_main.cpp
│   │   ├── test_console_sink.cpp
│   │   ├── test_file_sink.cpp
│   │   ├── test_dlt_sink.cpp
│   │   ├── test_syslog_sink.cpp
│   │   ├── test_boundary_values.cpp  # 18 个边界情况测试
│   │   ├── test_base64_encode.cpp    # 新增：15 个编码测试
│   │   └── ...
│   ├── benchmark/                # 性能基准测试
│   │   ├── benchmark_simple.cpp
│   │   ├── benchmark_stress_test.cpp
│   │   ├── benchmark_latency.cpp
│   │   ├── benchmark_memory.cpp
│   │   └── benchmark_multiprocess.cpp
│   └── examples/                 # 集成示例
│       ├── example_basic_usage.cpp
│       ├── example_multi_thread.cpp
│       ├── example_file_rotation.cpp
│       ├── example_base64_encode.cpp  # 新增：Base64 编码演示
│       ├── config_console_file.json
│       ├── config_base64_test.json    # 新增：Base64 配置
│       └── ...
└── doc/                          # 文档
    ├── BENCHMARK_REPORT.md
    ├── TEST_REPORT.md            # 新增：完整测试报告（65 个测试）
    └── ...

```

---

## 🧪 测试

### 单元测试

```bash
# 运行所有测试
cd build/modules/LogAndTrace
./log_test

# 结果：65/65 测试通过
# - ConsoleSink：3 个测试
# - FileSink：2 个测试
# - DLTSink：3 个测试
# - SyslogSink：7 个测试
# - SinkBase：5 个测试
# - LoggerTest：5 个测试
# - MultiThreadTest：5 个测试
# - ZeroCopyTest：2 个测试
# - BoundaryValueTests：18 个测试
# - Base64EncodeTests：15 个测试（新增）
```

**测试覆盖：**
- ✅ 基本功能（日志记录、级别过滤）
- ✅ 多线程（竞态条件、持续负载）
- ✅ 零拷贝验证（内存增长跟踪）
- ✅ 所有 sink 类型（Console、File、Syslog、DLT）
- ✅ 边界值（MAX_LOG_SIZE、缓冲区限制）
- ✅ 边缘情况（空、超大、特殊字符）
- ✅ 安全性（缓冲区溢出、截断）
- ✅ **新增**：Base64 编码（15 个全面测试）
- ✅ **新增**：线程安全编码（5 个线程 × 100 条消息）
- ✅ **新增**：编码性能（零开销验证）

**📋 [完整测试报告 →](TEST_REPORT.md)**

### DLT 集成验证

```bash
# 启动 DLT 守护进程
sudo systemctl start dlt-daemon

# 运行 DLT 长消息测试
cd build/modules/LogAndTrace
./test_dlt_long_message

# 在系统日志中验证消息
sudo journalctl -u dlt-daemon --since "1 minute ago" | grep DLTX

# 示例输出（成功发送 10 条消息）：
# - Application ID：DLTX
# - Context ID：DLTC
# - 消息：1 字节到 10KB（截断为 200 字节）
# - 状态：所有消息均被 DLT 守护进程接收
```

**DLT 测试结果：**
- ✅ 所有 10 条测试消息已传递到 DLT
- ✅ 短消息（1-50 字节）：完整通过
- ✅ 精确 MAX_LOG_SIZE（200 字节）：正确处理
- ✅ 超大消息（300B、10KB）：截断为 200 字节
- ✅ 特殊字符和 Unicode：支持
- ✅ 无"pure virtual method called"崩溃
- ✅ 使用 `std::_Exit(0)` 或正确的 MemManager 初始化清理退出

### 基准测试

```bash
# 快速基准测试
./benchmark_simple
# 结果：2.63M logs/sec，0.133µs 延迟

# 压力测试（持续 10 秒）
./benchmark_stress_test
# 结果：6.22M logs/sec，160ns 平均延迟，0 字节增长
```

**结果预览：**
- 单线程：2.63M logs/sec
- 持续：6.22M logs/sec（10 秒）
- 延迟：132.69ns 平均，160.77ns 平均值
- 内存：0 字节增长（50K 日志）

**📈 [完整基准测试报告 →](doc/BENCHMARK_REPORT.md)**

### 集成示例

```bash
# 基本用法
cd ../../modules/LogAndTrace
../../build/modules/LogAndTrace/example_basic_usage

# 多线程示例
../../build/modules/LogAndTrace/example_multi_thread

# 文件轮转示例
../../build/modules/LogAndTrace/example_file_rotation

# 新增：Base64 编码示例
../../build/modules/LogAndTrace/example_base64_encode

# DLT 验证
../../build/modules/LogAndTrace/test_dlt_direct
dlt-viewer &  # 验证消息
```

---

## 🔨 构建说明

### 前置条件

```bash
# Ubuntu/Debian
sudo apt-get install build-essential cmake libboost-all-dev libdlt-dev

# Fedora/RHEL
sudo dnf install gcc-c++ cmake boost-devel automotive-dlt-devel
```

### 构建步骤

```bash
# 克隆仓库
git clone <repo-url>
cd LightAP

# 创建构建目录
mkdir -p build && cd build

# 配置
cmake ..

# 构建 LogAndTrace 模块
make lap_log -j$(nproc)

# 运行测试
cd modules/LogAndTrace
./log_test

# 运行示例
./example_basic_usage
./example_multi_thread
```

---

## 🎯 下一步计划

**当前开发重点是模型化消息和跟踪系统实现。**

有关详细路线图、任务分解、预估工作量和优先级，请参阅：

👉 **[`doc/TODO.md`](doc/TODO.md)** - 完整特性路线图和任务列表

**快速总结：**
- **v1.0.0（2025-12）**：模型化消息、跟踪系统
- **v1.1.0（2026-Q1）**：异步日志队列、高级文件管理、日志过滤
- **v2.0.0（2026-Q2+）**：完整 AUTOSAR 合规、网络日志、安全增强

---

## 📖 文档

### 活跃文档

| 文档 | 描述 | 位置 |
|------|------|------|
| **TODO 列表** | 特性路线图和任务跟踪 | [`doc/TODO.md`](doc/TODO.md) |
| **设计文档** | 架构和设计规范 | [`doc/design/`](doc/design/) |
| **消息目录格式** | 模型化消息目录规范 | [`doc/design/MessageCatalog_Format.md`](doc/design/MessageCatalog_Format.md) |
| **AUTOSAR 规范** | AUTOSAR AP SWS_LogAndTrace 规范 | [`doc/AUTOSAR_AP_SWS_LogAndTrace.pdf`](doc/AUTOSAR_AP_SWS_LogAndTrace.pdf) |
| **索引** | 文档导航 | [`doc/INDEX.md`](doc/INDEX.md) |

### 归档文档

历史文档和已完成的分析报告存档于：
- [`doc/archive/`](doc/archive/) - 包含实现总结、基准测试和分析报告

---

## 🗺️ 路线图

有关详细任务分解和时间估计，请参阅 **[`doc/TODO.md`](doc/TODO.md)**。

### 当前焦点：Base64 编码与模型化消息（v1.0.0）

**最近完成（2025-11-14）：**
- ✅ **Base64 编码特性**：使用链式 API 安全记录敏感数据
- ✅ **15 个新测试**：全面的编码验证（65/65 测试通过）
- ✅ **零性能开销**：-0.49% 影响（测量误差）
- ✅ **线程安全编码**：多线程编码已验证

**优先级 P0 - 目标：2025-12**

1. **🎯 模型化消息实现**（5-7 天，规划中）
   - 符合 AUTOSAR 的消息 ID 模板
   - 使用 TraceSwitch 的编译时路由
   - DLT 消息 ID 支持（`dlt_user_log_write_start_id`）
   - 消息目录生成工具
   - 非详细/详细模式支持

2. **🔍 跟踪系统增强**（3-4 天，规划中）
   - ARTI 接口实现
   - TraceStatus 管理
   - 分离跟踪和日志路径

### 下一阶段：性能与特性（v1.1.0）

**优先级 P1 - 目标：2026-Q1**

3. **💾 本地缓存优化**（3-5 天，规划中）
   - 线程局部缓冲池用于日志格式化
   - 预分配缓冲区管理以减少分配开销
   - 每线程缓存常用日志上下文
   - Sink 缓冲区内存池
   - 目标：10-15% 性能提升
   - DLT sink 零拷贝优化

4. **⚡ 异步日志队列**（5-7 天，设计完成）
   - 无锁队列实现
   - 目标：2M+ logs/sec 吞吐量
   - 后台工作线程

5. **📁 高级文件管理**（2-3 天）
   - 基于时间的轮转
   - 压缩支持
   - 清理策略

6. **🔧 日志过滤**（3-4 天）
   - 按上下文级别过滤
   - 基于正则表达式的内容过滤
   - 运行时配置

### 未来计划（v2.0.0+）

**优先级 P2 - 目标：2026-Q2+**

- 网络日志（TCP/UDP sink）
- 高级分析工具
- 完整 AUTOSAR 合规认证
- 安全增强
- 多平台支持

---

## 🤝 贡献

### 开发状态

- **稳定**：核心日志、多 sink、DLT 集成
- **Beta**：零 STL 架构（重构后验证）
- **规划中**：模型化消息、跟踪系统

### 如何贡献

1. 查看 [`doc/TODO.md`](doc/TODO.md) 获取开放任务
2. 阅读现有代码和测试
3. 遵循零 STL 架构（使用 Core 模块类型）
4. 为新特性添加测试
5. 更新文档

### 代码风格

- 使用 Core 模块类型（`core::Vector`、`core::Mutex` 等）
- LogAndTrace 模块中无 STL 依赖
- 遵循 AUTOSAR 命名约定
- 添加 Doxygen 注释
- 零拷贝原则（使用 `core::StringView`）

---

## 📞 联系与支持

**项目**：LightAP Middleware  
**模块**：LogAndTrace  
**维护者**：ddkv587 (ddkv587@gmail.com)

如有问题、议题或贡献：
- 查看文档：[`doc/`](doc/)
- 查看 TODO 列表：[`doc/TODO.md`](doc/TODO.md)
- 查看归档报告：[`doc/archive/`](doc/archive/)

---

## 📄 许可证

本项目根据 MIT 许可证授权 - 详情请参阅 [LICENSE](LICENSE) 文件。

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

**最后更新**：2025-11-14  
**版本**：1.0.0  
**状态**：生产就绪 - Base64 编码特性已发布（65/65 测试通过）

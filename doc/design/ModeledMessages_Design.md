# Modeled Messages 设计文档

## 一、概念说明

### 1.1 什么是 Modeled Messages？

**Modeled Messages**（建模消息）是 AUTOSAR AP 规范中定义的一种高性能日志/追踪机制。

#### 对比：Non-modeled vs Modeled

| 特性 | Non-modeled Messages | Modeled Messages |
|------|---------------------|------------------|
| **使用方式** | `logger.LogInfo() << "Count: " << count;` | `logger.Log(MsgId{}, "count", count);` |
| **格式化时机** | 运行时格式化字符串 | 编译期确定格式 |
| **性能开销** | 中等（字符串拼接、内存分配） | 极低（编译期优化） |
| **类型安全** | 弱（依赖 operator<<） | 强（模板参数检查） |
| **消息统一性** | 格式不统一 | 格式预定义，统一 |
| **工具支持** | 难以自动化分析 | 易于生成文档和分析 |
| **Trace 集成** | 不支持 | 支持路由到 ARTI/LTTNG |
| **编译期优化** | 不支持 | 支持（可完全优化掉） |

### 1.2 核心设计思想

```
用户代码
    ↓
logger.Log(MsgId, params...)
    ↓
编译期决策（TraceSwitch）
    ↓
    ├─→ Route::None       → 编译器优化掉（零开销）
    ├─→ Route::Logger     → 路由到标准 Logger
    ├─→ Route::Arti       → 路由到 ARTI Trace（LTTNG）
    └─→ Route::Both       → 同时路由到 Logger 和 ARTI
```

**关键特性**：
1. **编译期路由**：通过 `TraceSwitch` 模板特化决定消息去向
2. **零运行时开销**：如果配置为 `None`，整个调用被优化掉
3. **类型安全**：编译期检查参数类型
4. **统一格式**：消息 ID + 命名参数，易于解析

---

## 二、架构设计

### 2.1 核心组件

```
┌─────────────────────────────────────────────────────────────┐
│                    Application Code                         │
│  logger.Log(StartupMsg{}, "version", "1.0", "pid", 1234);  │
└─────────────────────┬───────────────────────────────────────┘
                      ↓
┌─────────────────────────────────────────────────────────────┐
│                  CModeledMessage.hpp                         │
│  - MessageId<ID> 基类模板                                    │
│  - 消息 ID 类型定义（用户定义）                               │
└─────────────────────┬───────────────────────────────────────┘
                      ↓
┌─────────────────────────────────────────────────────────────┐
│                   CTraceSwitch.hpp                           │
│  - TraceSwitch<MsgId>::route 编译期路由配置                  │
│  - 默认：Route::Logger                                       │
│  - 用户特化：Route::None/Arti/Both                           │
└─────────────────────┬───────────────────────────────────────┘
                      ↓
┌─────────────────────────────────────────────────────────────┐
│                  Logger::Log() 实现                          │
│  if constexpr (route == Route::Logger)                      │
│      logModeledMessage(id, params...);                      │
│  if constexpr (route == Route::Arti)                        │
│      apext::log::TraceArti(id, params...);                  │
└─────────────────────┬───────────────────────────────────────┘
                      ↓
        ┌─────────────┴─────────────┐
        ↓                           ↓
┌────────────────┐          ┌──────────────────┐
│  Logger 路径    │          │   ARTI 路径       │
│  - 格式化       │          │   - LTTNG        │
│  - Sink 输出   │          │   - 硬件 Tracer   │
└────────────────┘          └──────────────────┘
```

---

## 三、详细设计

### 3.1 MessageId 基类模板

**文件**：`CModeledMessage.hpp`

```cpp
namespace lap::log {

// MessageId 基类模板
template<uint32_t ID>
struct MessageId {
    static constexpr uint32_t id = ID;
    
    // 编译期获取消息 ID
    constexpr uint32_t getId() const noexcept { return id; }
};

} // namespace lap::log
```

**使用示例**：
```cpp
// 用户定义消息 ID
struct StartupMessage : MessageId<1000> {};
struct ShutdownMessage : MessageId<1001> {};
struct ErrorMessage : MessageId<1002> {};

// 编译期访问 ID
static_assert(StartupMessage::id == 1000, "ID mismatch");
```

---

### 3.2 TraceSwitch 路由配置

**文件**：`CTraceSwitch.hpp`

```cpp
namespace lap::log {

// 路由枚举
enum class TraceRoute : uint8_t {
    None,       // 丢弃消息（编译期优化掉）
    Logger,     // 只路由到 Logger
    Arti,       // 只路由到 ARTI Trace
    Both        // 同时路由到 Logger 和 ARTI
};

// TraceSwitch 默认配置
template<typename MsgId>
struct TraceSwitch {
    // 默认路由到 Logger（符合 AUTOSAR SWS_LOG_20001）
    static constexpr TraceRoute route = TraceRoute::Logger;
};

} // namespace lap::log
```

**用户特化示例**：
```cpp
// 特化 StartupMessage 路由到 Logger 和 ARTI
template<>
struct TraceSwitch<StartupMessage> {
    static constexpr TraceRoute route = TraceRoute::Both;
};

// 特化 DebugMessage 完全丢弃（Release 版本零开销）
template<>
struct TraceSwitch<DebugMessage> {
    static constexpr TraceRoute route = TraceRoute::None;
};
```

---

### 3.3 Logger::Log() 实现

**文件**：`CLogger.cpp`

```cpp
namespace lap::log {

// 辅助函数：格式化 Modeled Message
template<typename MsgId, typename... Params>
void Logger::logModeledMessage(const MsgId& id, const Params&... params) const noexcept
{
    // 创建 LogStream
    LogStream stream(LogLevel::kInfo, *this);
    
    // 添加消息 ID
    stream << "[MsgId:" << id.getId() << "] ";
    
    // 添加参数（键值对形式）
    formatParams(stream, params...);
    
    // 自动 Flush（LogStream 析构时）
}

// 参数格式化（递归展开）
template<typename T, typename... Rest>
void formatParams(LogStream& stream, const char* name, const T& value, const Rest&... rest) const noexcept
{
    stream << name << "=" << value;
    
    if constexpr (sizeof...(rest) > 0) {
        stream << ", ";
        formatParams(stream, rest...);
    }
}

// 主入口：Logger::Log()
template<typename MsgId, typename... Params>
void Log(const MsgId& id, const Params&... params) noexcept
{
    // 编译期获取路由配置
    constexpr auto route = TraceSwitch<MsgId>::route;
    
    // 编译期路由决策 - 零运行时开销
    if constexpr (route == TraceRoute::Logger || route == TraceRoute::Both) {
        logModeledMessage(id, params...);
    }
    
    if constexpr (route == TraceRoute::Arti || route == TraceRoute::Both) {
        apext::log::TraceArti(id, params...);
    }
    
    // 如果 route == TraceRoute::None，整个函数体为空
    // 编译器会完全优化掉这个调用
}

} // namespace lap::log
```

---

### 3.4 ARTI Trace 接口

**文件**：`apext/log/trace_arti.h`

```cpp
namespace apext::log {

// 基础模板 - 默认空实现
template<typename MsgId, typename... Params>
inline void TraceArti(const MsgId& msg_id, const Params&... params) noexcept
{
    // 默认空实现
    // 用户可以特化此模板对接具体的 Trace 后端（LTTNG/ETW/DTrace）
}

} // namespace apext::log
```

**LTTNG 集成示例**：
```cpp
#ifdef USE_LTTNG
#include <lttng/tracepoint.h>

namespace apext::log {

// LTTNG 特化
template<>
inline void TraceArti<StartupMessage, const char*, const char*>(
    const StartupMessage& msg_id,
    const char* param1_name,
    const char* param1_value) noexcept
{
    tracepoint(myapp, startup, param1_value);
}

} // namespace apext::log
#endif
```

---

## 四、使用示例

### 4.1 定义消息 ID

```cpp
// MyMessages.hpp
#include <lap/log/CModeledMessage.hpp>

namespace myapp {

// 定义应用消息 ID
struct StartupMessage : lap::log::MessageId<1000> {};
struct ConfigLoadedMessage : lap::log::MessageId<1001> {};
struct ConnectionMessage : lap::log::MessageId<1002> {};
struct ErrorMessage : lap::log::MessageId<1003> {};
struct ShutdownMessage : lap::log::MessageId<1004> {};

} // namespace myapp
```

### 4.2 配置路由

```cpp
// MyTraceSwitches.hpp
#include <lap/log/CTraceSwitch.hpp>
#include "MyMessages.hpp"

namespace lap::log {

// StartupMessage: 路由到 Logger 和 ARTI（重要事件）
template<>
struct TraceSwitch<myapp::StartupMessage> {
    static constexpr TraceRoute route = TraceRoute::Both;
};

// ConfigLoadedMessage: 只记录日志
template<>
struct TraceSwitch<myapp::ConfigLoadedMessage> {
    static constexpr TraceRoute route = TraceRoute::Logger;
};

// ConnectionMessage: 只发送到 ARTI（高频事件，不污染日志）
template<>
struct TraceSwitch<myapp::ConnectionMessage> {
    static constexpr TraceRoute route = TraceRoute::Arti;
};

// ErrorMessage: 同时记录
template<>
struct TraceSwitch<myapp::ErrorMessage> {
    static constexpr TraceRoute route = TraceRoute::Both;
};

// ShutdownMessage: Release 版本优化掉（Debug Only）
template<>
struct TraceSwitch<myapp::ShutdownMessage> {
#ifdef NDEBUG
    static constexpr TraceRoute route = TraceRoute::None;
#else
    static constexpr TraceRoute route = TraceRoute::Logger;
#endif
};

} // namespace lap::log
```

### 4.3 使用 Modeled Messages

```cpp
// main.cpp
#include <lap/log/CLogManager.hpp>
#include "MyMessages.hpp"
#include "MyTraceSwitches.hpp"

int main(int argc, char* argv[]) {
    auto& logger = lap::log::LogManager::getInstance().getLogger("APP");
    
    // 启动消息 - 路由到 Logger 和 ARTI
    logger.Log(myapp::StartupMessage{}, 
               "version", "1.0.0", 
               "pid", getpid(), 
               "timestamp", time(nullptr));
    
    // 配置加载 - 只记录日志
    logger.Log(myapp::ConfigLoadedMessage{}, 
               "file", "/etc/app.conf", 
               "items", 42);
    
    // 连接事件 - 只发送到 ARTI（高频）
    for (int i = 0; i < 1000; ++i) {
        logger.Log(myapp::ConnectionMessage{}, 
                   "client_id", i, 
                   "latency_us", rand() % 1000);
    }
    
    // 错误 - 同时记录
    if (error) {
        logger.Log(myapp::ErrorMessage{}, 
                   "code", 500, 
                   "message", "Internal error");
    }
    
    // 关闭 - Release 版本完全优化掉
    logger.Log(myapp::ShutdownMessage{}, "exitCode", 0);
    
    return 0;
}
```

**输出示例**（假设 Logger 路由）：
```
[INFO] [APP] [MsgId:1000] version=1.0.0, pid=12345, timestamp=1699123456
[INFO] [APP] [MsgId:1001] file=/etc/app.conf, items=42
[ERROR] [APP] [MsgId:1003] code=500, message=Internal error
```

---

## 五、性能特性

### 5.1 零开销保证

#### 示例 1：完全优化掉

```cpp
struct DebugMsg : MessageId<9999> {};

template<>
struct TraceSwitch<DebugMsg> {
    static constexpr TraceRoute route = TraceRoute::None;
};

// Release 版本
for (int i = 0; i < 1000000; ++i) {
    logger.Log(DebugMsg{}, "iteration", i);  // 完全被优化掉！
}

// 汇编输出：空（循环都被优化掉）
```

#### 示例 2：最小 ARTI 开销

```cpp
struct PerfMsg : MessageId<8888> {};

template<>
struct TraceSwitch<PerfMsg> {
    static constexpr TraceRoute route = TraceRoute::Arti;
};

// 使用 LTTNG tracepoint（极低开销：~50-100ns）
logger.Log(PerfMsg{}, "value", 42);
```

### 5.2 性能对比

| 场景 | Non-modeled | Modeled (Logger) | Modeled (ARTI) | Modeled (None) |
|------|-------------|------------------|----------------|----------------|
| **简单消息** | ~1-2 μs | ~800 ns | ~100 ns | **0 ns** |
| **复杂消息** | ~5-10 μs | ~2 μs | ~200 ns | **0 ns** |
| **高频调用** | 影响性能 | 适中 | 几乎无影响 | **完全无影响** |

---

## 六、配置方案

### 6.1 编译期配置（推荐）

通过模板特化在编译期决定路由：

```cpp
// Debug 版本：记录所有
#ifndef NDEBUG
template<> struct TraceSwitch<DebugMsg> { 
    static constexpr TraceRoute route = TraceRoute::Logger; 
};
#else
// Release 版本：优化掉
template<> struct TraceSwitch<DebugMsg> { 
    static constexpr TraceRoute route = TraceRoute::None; 
};
#endif
```

### 6.2 配置文件方案（可选）

运行时动态配置（需要额外开销）：

```json
{
  "traceConfig": {
    "messageSwitches": {
      "1000": "both",      // StartupMessage
      "1001": "logger",    // ConfigLoadedMessage
      "1002": "arti",      // ConnectionMessage
      "1003": "both",      // ErrorMessage
      "1004": "none"       // ShutdownMessage
    }
  }
}
```

**实现**：需要额外的运行时查表逻辑（有性能开销）

---

## 七、工具集成

### 7.1 消息目录生成

```python
# tools/generate_message_catalog.py
# 从代码中提取所有 MessageId 定义
# 生成消息目录文档

# 输出：
# Message Catalog
# ================
# 1000 - StartupMessage: Application startup event
# 1001 - ConfigLoadedMessage: Configuration loaded
# ...
```

### 7.2 Trace 分析工具

```python
# tools/analyze_traces.py
# 解析 LTTNG trace 文件
# 根据 Message ID 关联日志

# 输出：
# Timeline Analysis
# =================
# T+0ms   : [1000] Startup version=1.0.0
# T+10ms  : [1001] Config loaded items=42
# T+15ms  : [1002] Connection client_id=1
# ...
```

---

## 八、实施步骤

### Phase 1: 基础设施（3 天）

1. 创建 `CModeledMessage.hpp`
   - 定义 `MessageId<ID>` 基类
   - 添加工具函数

2. 创建 `CTraceSwitch.hpp`
   - 定义 `TraceRoute` 枚举
   - 定义 `TraceSwitch<MsgId>` 模板
   - 提供默认配置

3. 单元测试
   - 测试 MessageId 类型
   - 测试编译期路由

### Phase 2: Logger 集成（2 天）

1. 实现 `Logger::logModeledMessage()`
   - 参数格式化
   - LogStream 集成

2. 实现 `Logger::Log()` 模板
   - 编译期路由逻辑
   - 集成 TraceSwitch

3. 单元测试
   - 测试各种路由配置
   - 性能基准测试

### Phase 3: ARTI 接口（2 天）

1. 创建 `apext/log/trace_arti.h`
   - 基础模板定义
   - 文档和示例

2. LTTNG 集成示例（可选）
   - 示例特化
   - 集成测试

3. 文档
   - 使用指南
   - 最佳实践

---

## 九、总结

### 9.1 核心优势

1. **零运行时开销**：编译期优化，Release 版本完全无开销
2. **类型安全**：编译期检查，避免运行时错误
3. **统一格式**：易于解析和分析
4. **灵活路由**：支持 Logger/ARTI/Both/None 四种模式
5. **工具友好**：易于生成文档和分析工具

### 9.2 适用场景

✅ **适合 Modeled Messages**：
- 性能关键路径（高频日志）
- 需要 Trace 分析（时序分析、性能剖析）
- 消息格式固定（状态变更、事件通知）
- Release 版本需要优化（Debug 消息）

❌ **不适合 Modeled Messages**：
- 消息格式高度动态（依赖运行时数据）
- 一次性调试消息（临时添加）
- 非性能关键路径（偶尔调用）

### 9.3 与 Non-modeled 共存

**推荐策略**：
- **Non-modeled**：用于一般日志、调试信息
- **Modeled**：用于性能关键、需要 Trace 分析的场景

```cpp
// 一般日志 - Non-modeled
logger.LogInfo() << "Starting initialization...";

// 性能关键 - Modeled
logger.Log(InitStartMsg{}, "timestamp", now());

// 复杂调试 - Non-modeled
logger.LogDebug() << "Config details: " << dumpConfig();

// 高频事件 - Modeled
for (auto& item : items) {
    logger.Log(ItemProcessedMsg{}, "id", item.id, "status", item.status);
}
```

---

**文档版本**：1.0  
**最后更新**：2025-11-06  
**作者**：GitHub Copilot  
**状态**：设计完成，待实现

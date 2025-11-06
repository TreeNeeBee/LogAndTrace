# LogAndTrace模块优化方案

## 当前架构分析

### 核心组件
1. **CLogManager** - 单例管理器，负责DLT初始化和Logger注册
2. **Logger** - 日志上下文，对应DLT的Context
3. **LogStream** - 流式日志输出接口，负责实际写入
4. **LogMode** - 支持Remote(DLT)、File、Console三种模式

### 现有问题
1. ❌ **Console模式未实现** - 虽然配置支持`LogMode::kConsole`，但实际没有控制台输出逻辑
2. ⚠️ **依赖外部DLT守护进程** - 需要dlt-daemon运行，否则日志丢失
3. ⚠️ **缺少本地fallback** - DLT不可用时无日志可查
4. ⚠️ **同步阻塞** - 使用`std::mutex`保护，无异步机制
5. ⚠️ **性能瓶颈** - 每条日志都通过DLT IPC，开销大

---

## 优化方案

### Phase 1: 增加Console本地输出 ✅ [优先级: 高]

#### 实现目标
- 支持日志同时输出到console（stdout/stderr）
- 遵循配置文件中的`logTraceLogMode`设置
- 带颜色高亮，易于调试

#### 技术方案
```cpp
// 1. 在LogStream析构时增加console输出
LogStream::~LogStream() {
    if (m_bWriteEnable) {
        dlt_user_log_write_finish(&m_logLocalData);
        
        // 新增: Console输出
        if (shouldOutputToConsole()) {
            writeToConsole();
        }
    }
}

// 2. Console输出实现
void LogStream::writeToConsole() {
    // 构造格式: [时间] [级别] [Context] 消息
    // 使用ANSI颜色码区分级别
    auto timestamp = getCurrentTimestamp();
    auto levelColor = getLevelColor(m_logLevel);
    
    fprintf(stderr, "%s[%s] [%s] [%s]%s ",
            levelColor,
            timestamp.c_str(),
            getLevelName(m_logLevel),
            m_logger.getContextId().c_str(),
            ANSI_RESET);
    
    // 输出实际消息（需要缓存LogStream内容）
    fprintf(stderr, "%s\n", m_messageBuffer.c_str());
    fflush(stderr);
}
```

#### 修改点
1. **CLogStream.hpp/cpp**
   - 增加`m_messageBuffer`成员缓存消息
   - 增加`writeToConsole()`方法
   - 修改所有`operator<<`同时写入buffer
   
2. **CLogManager.hpp/cpp**
   - 增加`isConsoleEnabled()`方法
   - 读取配置的`logTraceLogMode`

3. **CCommon.hpp**
   - 增加ANSI颜色常量定义

---

### Phase 2: 性能优化 [优先级: 中]

#### 2.1 异步日志队列
```cpp
class AsyncLogQueue {
    std::queue<LogEntry> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::thread m_worker;
    
    void workerLoop() {
        while (running) {
            auto entry = dequeue();
            // 批量写入DLT
            dlt_user_log_write_raw(...);
        }
    }
};
```

**优势**：
- 日志调用立即返回（微秒级）
- 批量写入提升吞吐
- 避免阻塞业务线程

**劣势**：
- 增加内存开销
- 进程崩溃可能丢失队列中日志

#### 2.2 零拷贝优化
- 使用`string_view`避免拷贝
- 预分配buffer池
- 消息组装延迟到写入时

---

### Phase 3: 架构增强 [优先级: 中]

#### 3.1 多Sink支持
```cpp
class LogSink {
public:
    virtual void write(LogLevel, StringView msg) = 0;
};

class DltSink : public LogSink { ... };
class ConsoleSink : public LogSink { ... };
class FileSink : public LogSink { ... };
class SyslogSink : public LogSink { ... };

// LogStream同时写入多个sink
vector<unique_ptr<LogSink>> m_sinks;
```

#### 3.2 结构化日志
```cpp
// 支持JSON格式输出
logger.Info()
    .Field("user_id", 12345)
    .Field("action", "login")
    .Field("duration_ms", 123)
    .Msg("User logged in");

// 输出: {"timestamp":...,"level":"INFO","msg":"User logged in","user_id":12345,...}
```

#### 3.3 日志过滤和采样
```cpp
// 按条件过滤
logger.SetFilter([](LogLevel level, StringView msg) {
    return level >= LogLevel::kWarn || msg.find("critical") != npos;
});

// 采样（只记录1/100）
logger.SetSampleRate(0.01);
```

---

### Phase 4: 可靠性增强 [优先级: 低]

#### 4.1 DLT故障降级
```cpp
if (!dlt_available()) {
    // 自动切换到file模式
    fallbackToFile();
}
```

#### 4.2 日志回放
- 支持读取本地日志文件
- 转换为DLT格式重新注入

#### 4.3 日志压缩和轮转
- 自动压缩历史日志
- 按大小/时间轮转

---

## 实施优先级

### 立即实施 (本次)
✅ **Console输出** - 解决调试困难问题

### 短期 (1-2周)
- 用`core::Mutex`替换`std::mutex` (与Persistency统一)
- 增加性能测试基准

### 中期 (1个月)
- 异步日志队列
- 多Sink架构

### 长期 (按需)
- 结构化日志
- 日志分析工具

---

## 风险评估

| 风险 | 影响 | 缓解措施 |
|------|------|----------|
| Console输出影响性能 | 中 | 默认关闭，仅开发环境启用 |
| 消息缓存增加内存 | 低 | 限制单条消息4KB |
| 多线程竞态 | 中 | 完善锁保护 |
| DLT兼容性 | 低 | 保持向后兼容 |

---

## 性能目标

| 指标 | 当前 | 目标 |
|------|------|------|
| 日志调用延迟 | ~50us (同步) | <5us (异步) |
| 吞吐量 | ~20K msg/s | >100K msg/s |
| 内存开销 | ~100B/msg | <50B/msg |
| CPU占用 | ~2% | <1% |

---

## 测试计划

1. **功能测试**
   - Console输出正确性
   - 多模式组合
   - 配置加载

2. **性能测试**
   - 延迟基准测试
   - 吞吐量压测
   - 多线程并发

3. **可靠性测试**
   - DLT daemon崩溃场景
   - 磁盘满场景
   - 长时间运行稳定性

---

## 代码变更估算

| 文件 | 修改行数 | 复杂度 |
|------|----------|--------|
| CLogStream.cpp | +150 | 中 |
| CLogStream.hpp | +30 | 低 |
| CLogManager.cpp | +50 | 低 |
| CCommon.hpp | +20 | 低 |
| 新增测试 | +200 | 中 |
| **总计** | **~450** | **中** |

预计工作量: **2-3天**

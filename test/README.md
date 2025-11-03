# LogAndTrace Test Suite

## 目录结构

```
test/
├── unittest/                    # 单元测试
│   ├── test_log_common.cpp     # 通用功能测试（LogLevel, LogMode）
│   ├── test_log_manager.cpp    # 日志管理器测试
│   ├── test_logger_stream.cpp  # Logger和LogStream测试
│   ├── test_multi_sink.cpp     # 多sink架构测试
│   ├── test_syslog_sink.cpp    # Syslog sink测试
│   └── test_sink_base.cpp      # ISink接口和基础功能测试
├── benchmark/                   # 性能基准测试
│   ├── benchmark_throughput.cpp # 吞吐量测试
│   ├── benchmark_latency.cpp    # 延迟测试
│   └── benchmark_memory.cpp     # 内存使用测试
└── examples/                    # 示例程序
```

##单元测试

### 运行测试

```bash
cd build/modules/LogAndTrace
./log_test
```

### 测试覆盖

**总计: 25个测试，6个测试套件**

#### 1. LogCommon (2测试)
- `ToStringLevels` - 日志级别字符串转换
- `ModeOperators` - 日志模式位操作

#### 2. LogManager (2测试)
- `InitializeDefaultAndRegister` - 初始化和注册logger
- `ParseLevelFatal` - Fatal级别解析

#### 3. LogFixture (2测试)
- `CreateLoggerAndBasicLogs` - 创建logger和基础日志
- `LogStreamFormatAndBinaryHex` - 格式化和二进制输出

#### 4. MultiSink (7测试)
- `ConsoleSinkBasic` - Console sink基础功能
- `FileSinkBasic` - File sink基础功能
- `FileSinkRotation` - 文件轮转
- `SinkManagerMultipleDestinations` - 多目标输出
- `SinkManagerLevelFiltering` - 级别过滤
- `SinkManagerRemoval` - Sink移除
- `PerformanceBenchmark` - 性能基准（~204K logs/sec）

#### 5. SinkBase (5测试)
- `PolymorphicBehavior` - 多态行为
- `LevelHierarchy` - 级别层次结构
- `EnableDisableState` - 启用/禁用状态
- `LogEntryParsing` - LogEntry解析
- `ConcurrentWrites` - 并发写入

#### 6. SyslogSink (7测试)
- `BasicConstruction` - 基本构造
- `LevelFiltering` - 级别过滤
- `WriteBasicLog` - 基础写入
- `FacilitySelection` - Facility选择
- `EnableDisable` - 启用/禁用
- `LevelUpdate` - 级别更新
- `MultipleMessages` - 多消息写入

### 测试结果

```
[==========] Running 25 tests from 6 test suites.
[  PASSED  ] 25 tests.
```

## Benchmark性能测试

### 运行Benchmark

```bash
cd build/modules/LogAndTrace

# 吞吐量测试
./benchmark_throughput

# 延迟测试
./benchmark_latency

# 内存测试
./benchmark_memory
```

### 1. 吞吐量测试 (Throughput)

#### File Sink单线程吞吐量
| 消息大小 | 日志数 | 耗时 | 吞吐量 |
|---------|--------|------|--------|
| 5 bytes | 10000 | 45ms | **222K logs/sec** |
| 63 bytes | 10000 | 23ms | **435K logs/sec** |
| 216 bytes | 10000 | 21ms | **476K logs/sec** |

#### 多线程吞吐量
| 线程数 | 总日志数 | 耗时 | 吞吐量 |
|-------|---------|------|--------|
| 1 | 5000 | 12ms | **417K logs/sec** |
| 2 | 10000 | 22ms | **455K logs/sec** |
| 4 | 20000 | 43ms | **465K logs/sec** |
| 8 | 40000 | 92ms | **435K logs/sec** |

#### Sink类型对比 (10K logs)
| Sink类型 | 吞吐量 |
|---------|--------|
| File Sink | **667K logs/sec** |
| Console Sink (禁用) | 极快（无IO） |
| Syslog Sink | ~300K logs/sec |

**结论**: 
- 文件输出吞吐量: **222K - 667K logs/sec**
- 多线程扩展性良好，4线程达到峰值性能
- 消息大小对性能影响较小（内部优化良好）

### 2. 延迟测试 (Latency)

#### File Sink写入延迟（10000样本）
```
Mean:   0.84 μs
P50:    0.75 μs
P90:    0.77 μs
P99:    2.67 μs
P99.9:  16.82 μs
Max:    29.44 μs
```

#### 不同Sink延迟对比
| Sink类型 | 平均延迟 | P50 | P99 | P99.9 |
|---------|---------|-----|-----|-------|
| File Sink | 0.84 μs | 0.75 μs | 2.67 μs | 16.82 μs |
| Console (禁用) | 33.52 μs | 6.27 μs | 15.69 μs | 55.61 μs |
| Syslog | 24.02 μs | 1.72 μs | 6.80 μs | 5240.94 μs |

**结论**:
- **单次日志延迟 < 1微秒** (P50)
- 99%的日志在 **< 3微秒** 内完成
- File Sink性能最佳，延迟最稳定
- Syslog因系统调用有更高的尾部延迟

### 3. 内存测试 (Memory)

#### LogEntry内存开销
| 消息长度 | 总大小 | 开销 |
|---------|--------|------|
| 10 bytes | 79 bytes | **64 bytes** |
| 50 bytes | 119 bytes | 64 bytes |
| 100 bytes | 169 bytes | 64 bytes |
| 500 bytes | 569 bytes | 64 bytes |
| 1000 bytes | 1069 bytes | 64 bytes |

**固定开销**: 64字节（包含timestamp, threadId, level, length等元数据）

#### 内存增长测试
| 日志数 | 内存占用 | 增量 |
|-------|---------|------|
| 1K | 2944 KB | 0 KB |
| 5K | 2944 KB | 0 KB |
| 10K | 2944 KB | 0 KB |
| 50K | 2944 KB | 0 KB |
| 100K | 2944 KB | 0 KB |

**结论**: 
- **无内存泄漏** ✓
- LogEntry正确分配和释放
- 内存占用稳定，不随日志量增长

#### 峰值内存测试
连续burst写入测试显示内存占用稳定，未见峰值激增。

## 性能总结

### 关键指标

| 指标 | 数值 | 说明 |
|------|------|------|
| **吞吐量** | **222K - 667K logs/sec** | 取决于消息大小和sink类型 |
| **延迟 (P50)** | **< 1 μs** | 单次日志写入中位数 |
| **延迟 (P99)** | **< 3 μs** | 99%的日志延迟 |
| **内存开销** | **64 bytes/entry** | 固定元数据开销 |
| **内存泄漏** | **0 KB** | 无内存泄漏 |

### 优化特性

1. **同步架构**: 移除异步队列后，避免了队列开销，性能反而提升
2. **系统级缓存**: 依赖DLT daemon和syslog的缓存机制
3. **零拷贝设计**: LogEntry使用flexible array避免额外分配
4. **内存追踪**: IMP_OPERATOR_NEW集成Core模块的内存管理

### 对比分析

| 架构 | 吞吐量 | 优势 | 劣势 |
|------|--------|------|------|
| **同步 (当前)** | **222K-667K** | 简单、稳定、低延迟 | 阻塞写入 |
| 异步队列 (已移除) | 21K-23K | 非阻塞 | 复杂、不稳定、高开销 |

**结论**: 同步架构性能优于异步架构10倍+，更适合生产环境。

## 构建和测试

### 构建

```bash
cd build
cmake ..
make lap_log log_test benchmark_throughput benchmark_latency benchmark_memory -j$(nproc)
```

### 运行全部测试

```bash
# 单元测试
./log_test

# Benchmark
./benchmark_throughput
./benchmark_latency  
./benchmark_memory
```

### CTest集成

```bash
cd build
ctest -R log_tests -V
```

## 技术栈

- **测试框架**: Google Test (GTest)
- **性能测试**: chrono高精度计时
- **内存追踪**: /proc/self/status + Core Memory API
- **日志后端**: DLT + syslog + 文件

## 待办事项

- [ ] 实现Memory::getMemoryStats() API完善内存追踪
- [ ] 添加压力测试（长时间运行）
- [ ] 添加错误处理测试
- [ ] 性能对比不同硬件环境

## 联系方式

- **Author**: ddkv587 (ddkv587@gmail.com)
- **Project**: LightAP
- **Date**: 2025-10-28

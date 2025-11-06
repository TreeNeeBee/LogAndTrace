# Modeled Messages 消息表格式设计

## 一、工作原理

### 1.1 运行时 vs 分析时

```
┌─────────────────────────────────────────────────────────────┐
│                    编译期（Build Time）                      │
├─────────────────────────────────────────────────────────────┤
│ 1. 用户定义消息 ID：                                          │
│    struct StartupMsg : MessageId<1000> {};                  │
│                                                             │
│ 2. 代码生成工具扫描源码，生成消息表：                          │
│    message_catalog.json                                     │
│    {                                                        │
│      "1000": {                                              │
│        "name": "StartupMsg",                                │
│        "format": "Application started: version={0}, pid={1}",│
│        "params": ["version:string", "pid:int"]              │
│      }                                                      │
│    }                                                        │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│                    运行时（Runtime）                         │
├─────────────────────────────────────────────────────────────┤
│ 程序执行：                                                    │
│   logger.Log(StartupMsg{}, "version", "1.0.0", "pid", 1234);│
│                                                             │
│ 实际输出到日志文件/DLT：                                       │
│   [BINARY] 1000 | "1.0.0" | 1234                           │
│          ^^^^    ^^^^^^^^   ^^^^                            │
│         消息ID    参数1      参数2                            │
│                                                             │
│ 或者文本格式：                                                │
│   [MsgId:1000] version=1.0.0, pid=1234                     │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│                    分析时（Analysis Time）                   │
├─────────────────────────────────────────────────────────────┤
│ 分析工具加载：                                                │
│   1. message_catalog.json（消息表）                          │
│   2. app.log 或 dlt.trace（运行时日志）                      │
│                                                             │
│ 还原完整消息：                                                │
│   ID 1000 → "Application started: version=1.0.0, pid=1234" │
│                                                             │
│ 可视化、过滤、统计、时序分析...                                │
└─────────────────────────────────────────────────────────────┘
```

---

## 二、消息表格式详解

### 2.1 JSON 格式（推荐）

**文件名**：`message_catalog.json`

```json
{
  "version": "1.0",
  "application": "MyApp",
  "timestamp": "2025-11-06T12:00:00Z",
  "messages": {
    "1000": {
      "name": "StartupMessage",
      "severity": "INFO",
      "format": "Application started: version={version}, pid={pid}, timestamp={timestamp}",
      "description": "Application startup event",
      "parameters": [
        {
          "name": "version",
          "type": "string",
          "description": "Application version"
        },
        {
          "name": "pid",
          "type": "int32",
          "description": "Process ID"
        },
        {
          "name": "timestamp",
          "type": "uint64",
          "description": "Unix timestamp in milliseconds"
        }
      ],
      "category": "lifecycle",
      "source_file": "src/main.cpp",
      "source_line": 42
    },
    "1001": {
      "name": "ConfigLoadedMessage",
      "severity": "INFO",
      "format": "Configuration loaded: file={file}, items={items}, duration_ms={duration}",
      "description": "Configuration file successfully loaded",
      "parameters": [
        {
          "name": "file",
          "type": "string",
          "description": "Configuration file path"
        },
        {
          "name": "items",
          "type": "uint32",
          "description": "Number of configuration items"
        },
        {
          "name": "duration",
          "type": "uint32",
          "description": "Load duration in milliseconds"
        }
      ],
      "category": "configuration"
    },
    "1002": {
      "name": "ConnectionEstablished",
      "severity": "DEBUG",
      "format": "Connection established: client_id={id}, latency_us={latency}",
      "description": "Client connection established",
      "parameters": [
        {
          "name": "id",
          "type": "uint32",
          "description": "Client ID"
        },
        {
          "name": "latency",
          "type": "uint32",
          "description": "Connection latency in microseconds"
        }
      ],
      "category": "network",
      "frequency": "high"
    },
    "1003": {
      "name": "ErrorMessage",
      "severity": "ERROR",
      "format": "Error occurred: code={code}, message={message}, context={context}",
      "description": "Generic error event",
      "parameters": [
        {
          "name": "code",
          "type": "int32",
          "description": "Error code"
        },
        {
          "name": "message",
          "type": "string",
          "description": "Error message"
        },
        {
          "name": "context",
          "type": "string",
          "description": "Error context"
        }
      ],
      "category": "error"
    }
  }
}
```

---

### 2.2 XML 格式（DLT 兼容）

**文件名**：`message_catalog.xml`

```xml
<?xml version="1.0" encoding="UTF-8"?>
<MessageCatalog version="1.0" application="MyApp">
  <Message id="1000" name="StartupMessage" severity="INFO">
    <Format>Application started: version={version}, pid={pid}</Format>
    <Description>Application startup event</Description>
    <Parameters>
      <Parameter name="version" type="string">Application version</Parameter>
      <Parameter name="pid" type="int32">Process ID</Parameter>
    </Parameters>
    <Metadata category="lifecycle" source="src/main.cpp" line="42"/>
  </Message>
  
  <Message id="1001" name="ConfigLoadedMessage" severity="INFO">
    <Format>Configuration loaded: file={file}, items={items}</Format>
    <Description>Configuration file loaded</Description>
    <Parameters>
      <Parameter name="file" type="string">Config file path</Parameter>
      <Parameter name="items" type="uint32">Number of items</Parameter>
    </Parameters>
  </Message>
</MessageCatalog>
```

---

### 2.3 二进制格式（极致性能）

**AUTOSAR FIBEX 格式**（用于汽车行业）

```
[Header: 16 bytes]
  Magic: "MSGCAT\0\0"
  Version: 0x0100
  MessageCount: 1000
  Reserved: 0x00000000

[Message Table: N * 64 bytes]
  Message 1000:
    ID: 1000 (4 bytes)
    NameOffset: 0x1000 (4 bytes)
    FormatOffset: 0x1100 (4 bytes)
    ParamCount: 3 (2 bytes)
    Severity: INFO (1 byte)
    Reserved: 0x00 (1 byte)
    ...

[String Pool]
  Offset 0x1000: "StartupMessage\0"
  Offset 0x1100: "Application started: version={version}, pid={pid}\0"
  ...
```

---

## 三、运行时输出格式

### 3.1 文本格式（易读，调试友好）

```
[2025-11-06 12:00:00.123] [INFO] [APP] [MsgId:1000] version=1.0.0, pid=12345, timestamp=1699269600000
[2025-11-06 12:00:00.456] [INFO] [APP] [MsgId:1001] file=/etc/app.conf, items=42, duration_ms=15
[2025-11-06 12:00:01.789] [DEBUG] [APP] [MsgId:1002] id=1, latency_us=150
[2025-11-06 12:00:02.012] [ERROR] [APP] [MsgId:1003] code=500, message="Timeout", context="DB connection"
```

**优点**：
- ✅ 人类可读
- ✅ 无需工具即可查看
- ✅ grep/awk 友好

**缺点**：
- ❌ 文件较大（包含参数名）
- ❌ 解析稍慢

---

### 3.2 二进制格式（极致性能，DLT 标准）

**DLT Message Format**：

```
[DLT Header: 16 bytes]
  StandardHeader: 0x35 (with timestamp, ECU ID)
  Timestamp: 0x12345678
  ECU ID: "ECU1"
  
[DLT Payload]
  Message Type: LOG
  Log Level: INFO
  
  [Argument 1: Message ID]
    Type: UINT32
    Value: 1000
  
  [Argument 2: version]
    Type: STRING
    Length: 5
    Value: "1.0.0"
  
  [Argument 3: pid]
    Type: UINT32
    Value: 12345
  
  [Argument 4: timestamp]
    Type: UINT64
    Value: 1699269600000
```

**优点**：
- ✅ 极小体积（无参数名，类型编码紧凑）
- ✅ 解析极快
- ✅ 工业标准（DLT）

**缺点**：
- ❌ 必须用工具查看（DLT Viewer）
- ❌ 调试不便

---

### 3.3 混合格式（推荐，平衡性能和易用性）

**方案**：文本 + 紧凑编码

```
[INFO] [APP] [1000] 1.0.0|12345|1699269600000
       ^^^^   ^^^^   ^^^^^^^^^^^^^^^^^^^^^
       级别   上下文  消息ID + 参数值（分隔符）
```

**解析时**：
```python
# 1. 读取日志行
line = "[INFO] [APP] [1000] 1.0.0|12345|1699269600000"

# 2. 提取消息 ID
msg_id = 1000

# 3. 从消息表加载格式
catalog = load_catalog("message_catalog.json")
msg_info = catalog["messages"]["1000"]
format_str = msg_info["format"]
# "Application started: version={version}, pid={pid}, timestamp={timestamp}"

# 4. 解析参数值
values = ["1.0.0", "12345", "1699269600000"]

# 5. 还原完整消息
message = format_str.format(version=values[0], pid=values[1], timestamp=values[2])
# "Application started: version=1.0.0, pid=12345, timestamp=1699269600000"
```

---

## 四、动态数据处理方案

### 4.1 问题：运行时动态数据如何打印？

#### **场景 1：动态数量的数据**

```cpp
// 问题：参数数量在编译期不确定
std::vector<int> items = {1, 2, 3, 4, 5}; // 数量动态
logger.Log(ItemListMsg{}, "items", items); // 如何处理？
```

#### **方案 A：序列化为字符串**（推荐）

```cpp
struct ItemListMessage : MessageId<2000> {};

// 使用时
std::vector<int> items = {1, 2, 3, 4, 5};
std::string items_str = formatVector(items); // "1,2,3,4,5"
logger.Log(ItemListMessage{}, "count", items.size(), "items", items_str.c_str());

// 输出
[MsgId:2000] count=5, items="1,2,3,4,5"

// 消息表
{
  "2000": {
    "format": "Item list: count={count}, items={items}",
    "parameters": [
      {"name": "count", "type": "uint32"},
      {"name": "items", "type": "string", "description": "Comma-separated list"}
    ]
  }
}
```

#### **方案 B：JSON 序列化**（结构化数据）

```cpp
struct ComplexDataMessage : MessageId<2001> {};

// 使用时
json data = {
  {"user", "alice"},
  {"age", 25},
  {"items", {1, 2, 3}}
};
std::string data_str = data.dump(); // '{"user":"alice","age":25,"items":[1,2,3]}'
logger.Log(ComplexDataMessage{}, "data", data_str.c_str());

// 输出
[MsgId:2001] data='{"user":"alice","age":25,"items":[1,2,3]}'

// 分析时可以解析 JSON
```

#### **方案 C：多消息组合**（高频数据）

```cpp
struct ItemGroupStartMessage : MessageId<2002> {};
struct ItemMessage : MessageId<2003> {};
struct ItemGroupEndMessage : MessageId<2004> {};

// 使用时
logger.Log(ItemGroupStartMessage{}, "count", items.size());
for (const auto& item : items) {
    logger.Log(ItemMessage{}, "id", item.id, "value", item.value);
}
logger.Log(ItemGroupEndMessage{}, "count", items.size());

// 输出
[MsgId:2002] count=5
[MsgId:2003] id=1, value=100
[MsgId:2003] id=2, value=200
[MsgId:2003] id=3, value=300
[MsgId:2003] id=4, value=400
[MsgId:2003] id=5, value=500
[MsgId:2004] count=5

// 分析工具可以自动关联成组
```

---

### 4.2 问题：非预定义类型如何处理？

#### **场景 2：自定义类**

```cpp
class UserInfo {
public:
    std::string name;
    int age;
    std::vector<std::string> roles;
};

UserInfo user = {"Alice", 25, {"admin", "developer"}};
logger.Log(UserLoginMsg{}, "user", user); // 如何序列化？
```

#### **方案：实现序列化接口**

```cpp
// 1. 为自定义类型实现 toString()
class UserInfo {
public:
    std::string toString() const {
        std::ostringstream oss;
        oss << "User{name=" << name 
            << ", age=" << age 
            << ", roles=[" << joinRoles() << "]}";
        return oss.str();
    }
};

// 2. 使用时
logger.Log(UserLoginMsg{}, "user", user.toString().c_str());

// 输出
[MsgId:3000] user="User{name=Alice, age=25, roles=[admin,developer]}"
```

或者使用 JSON：

```cpp
// 1. 实现 toJson()
class UserInfo {
public:
    json toJson() const {
        return {
            {"name", name},
            {"age", age},
            {"roles", roles}
        };
    }
};

// 2. 使用时
logger.Log(UserLoginMsg{}, "user_json", user.toJson().dump().c_str());

// 输出
[MsgId:3000] user_json='{"name":"Alice","age":25,"roles":["admin","developer"]}'
```

---

### 4.3 问题：如何处理大数据（如二进制数据）？

#### **场景 3：二进制数据（图像、文件内容）**

```cpp
std::vector<uint8_t> imageData(1024 * 1024); // 1MB 图像
logger.Log(ImageReceivedMsg{}, "image", imageData); // 不可行！
```

#### **方案：引用方式**

```cpp
struct ImageReceivedMessage : MessageId<4000> {};

// 1. 保存数据到文件
std::string imageFile = saveToFile(imageData, "/tmp/image_12345.png");

// 2. 只记录文件路径和元数据
logger.Log(ImageReceivedMessage{}, 
           "file", imageFile.c_str(),
           "size", imageData.size(),
           "format", "PNG");

// 输出
[MsgId:4000] file="/tmp/image_12345.png", size=1048576, format="PNG"

// 分析工具可以根据路径加载文件
```

或者记录摘要：

```cpp
// 只记录哈希值和元信息
std::string hash = computeSHA256(imageData);
logger.Log(ImageReceivedMessage{}, 
           "hash", hash.c_str(),
           "size", imageData.size());

// 输出
[MsgId:4000] hash="a1b2c3d4...", size=1048576
```

---

## 五、实现建议

### 5.1 运行时实现（Logger::logModeledMessage）

```cpp
// CLogger.cpp

template<typename MsgId, typename... Params>
void Logger::logModeledMessage(const MsgId& id, const Params&... params) const noexcept
{
    // 获取消息 ID
    constexpr uint32_t msg_id = MsgId::id;
    
    // 创建 LogStream
    LogStream stream(LogLevel::kInfo, *this);
    
    // 方案 A：文本格式（易读）
    stream << "[MsgId:" << msg_id << "] ";
    formatParams(stream, params...);
    
    // 方案 B：紧凑格式（省空间）
    // stream << "[" << msg_id << "] ";
    // formatParamsCompact(stream, params...); // 输出: 1.0.0|12345|1699269600000
    
    // 方案 C：DLT 二进制格式（通过 DLT sink）
    // if (dltSinkEnabled) {
    //     sendToDLT(msg_id, params...);
    // }
}

// 参数格式化（键值对）
template<typename T, typename... Rest>
void Logger::formatParams(LogStream& stream, const char* name, const T& value, const Rest&... rest) const noexcept
{
    stream << name << "=" << value;
    
    if constexpr (sizeof...(rest) > 0) {
        stream << ", ";
        formatParams(stream, rest...);
    }
}

// 参数格式化（紧凑，只有值）
template<typename T, typename... Rest>
void Logger::formatParamsCompact(LogStream& stream, const T& value, const Rest&... rest) const noexcept
{
    stream << value;
    
    if constexpr (sizeof...(rest) > 0) {
        stream << "|";
        formatParamsCompact(stream, rest...);
    }
}
```

---

### 5.2 消息表生成工具

```python
#!/usr/bin/env python3
# tools/generate_message_catalog.py

import re
import json
import sys
from pathlib import Path

def scan_source_files(src_dir):
    """扫描源码，提取所有 MessageId 定义"""
    messages = {}
    
    # 正则：匹配 struct XXX : MessageId<NNNN> {};
    pattern = r'struct\s+(\w+)\s*:\s*MessageId<(\d+)>\s*\{\s*\}'
    
    for file in Path(src_dir).rglob("*.hpp"):
        with open(file) as f:
            content = f.read()
            for match in re.finditer(pattern, content):
                name = match.group(1)
                msg_id = match.group(2)
                
                messages[msg_id] = {
                    "name": name,
                    "format": f"Message {msg_id} - {name}",  # 默认格式
                    "parameters": [],
                    "source_file": str(file),
                    "source_line": content[:match.start()].count('\n') + 1
                }
    
    return messages

def generate_catalog(messages, output_file):
    """生成消息表 JSON 文件"""
    catalog = {
        "version": "1.0",
        "application": "LightAP",
        "messages": messages
    }
    
    with open(output_file, 'w') as f:
        json.dump(catalog, f, indent=2)
    
    print(f"Generated message catalog: {output_file}")
    print(f"Total messages: {len(messages)}")

if __name__ == "__main__":
    src_dir = sys.argv[1] if len(sys.argv) > 1 else "."
    output = sys.argv[2] if len(sys.argv) > 2 else "message_catalog.json"
    
    messages = scan_source_files(src_dir)
    generate_catalog(messages, output)
```

**使用**：
```bash
# 扫描源码生成消息表
./tools/generate_message_catalog.py modules/LogAndTrace/source/inc message_catalog.json

# 输出
Generated message catalog: message_catalog.json
Total messages: 42
```

---

### 5.3 日志分析工具

```python
#!/usr/bin/env python3
# tools/analyze_logs.py

import json
import re
import sys

def load_catalog(catalog_file):
    """加载消息表"""
    with open(catalog_file) as f:
        return json.load(f)

def parse_log_line(line):
    """解析日志行，提取 [MsgId:NNNN] 和参数"""
    # 匹配: [MsgId:1000] version=1.0.0, pid=12345
    match = re.search(r'\[MsgId:(\d+)\]\s+(.+)', line)
    if not match:
        return None
    
    msg_id = match.group(1)
    params_str = match.group(2)
    
    # 解析参数
    params = {}
    for param in params_str.split(', '):
        if '=' in param:
            key, value = param.split('=', 1)
            params[key] = value.strip('"')
    
    return {"msg_id": msg_id, "params": params}

def format_message(catalog, log_entry):
    """根据消息表还原完整消息"""
    msg_id = log_entry["msg_id"]
    params = log_entry["params"]
    
    if msg_id not in catalog["messages"]:
        return f"[Unknown Message ID: {msg_id}] {params}"
    
    msg_info = catalog["messages"][msg_id]
    format_str = msg_info["format"]
    
    # 替换 {param_name} 占位符
    for key, value in params.items():
        format_str = format_str.replace(f"{{{key}}}", value)
    
    return format_str

def analyze_logs(catalog_file, log_file):
    """分析日志文件"""
    catalog = load_catalog(catalog_file)
    
    print("=== Log Analysis ===\n")
    
    with open(log_file) as f:
        for line in f:
            log_entry = parse_log_line(line)
            if log_entry:
                formatted = format_message(catalog, log_entry)
                print(f"[{log_entry['msg_id']}] {formatted}")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: analyze_logs.py <message_catalog.json> <app.log>")
        sys.exit(1)
    
    analyze_logs(sys.argv[1], sys.argv[2])
```

**使用**：
```bash
# 分析日志
./tools/analyze_logs.py message_catalog.json /var/log/app.log

# 输出
=== Log Analysis ===

[1000] Application started: version=1.0.0, pid=12345, timestamp=1699269600000
[1001] Configuration loaded: file=/etc/app.conf, items=42, duration_ms=15
[1002] Connection established: client_id=1, latency_us=150
[1003] Error occurred: code=500, message=Timeout, context=DB connection
```

---

## 六、总结

### 6.1 消息表机制

| 阶段 | 内容 | 目的 |
|------|------|------|
| **编译期** | 生成 `message_catalog.json` | 定义所有消息格式 |
| **运行时** | 输出 `[MsgId:NNNN] + 参数值` | 极小开销，高性能 |
| **分析时** | 加载消息表 + 日志 → 还原完整消息 | 可视化、分析、调试 |

### 6.2 动态数据处理策略

| 数据类型 | 推荐方案 | 示例 |
|---------|---------|------|
| 动态数量 | 序列化为字符串 | `"1,2,3,4,5"` |
| 复杂结构 | JSON 序列化 | `'{"name":"Alice","age":25}'` |
| 自定义类 | 实现 toString() 或 toJson() | `user.toString()` |
| 大数据 | 保存到文件，记录路径 | `"/tmp/data.bin"` |
| 二进制数据 | 记录哈希值 + 元信息 | `hash + size` |

### 6.3 实现优先级

1. **高优先级**：文本格式 + 键值对输出（易调试）
2. **中优先级**：消息表生成工具
3. **低优先级**：二进制 DLT 格式（极致性能）

---

**文档版本**：1.0  
**最后更新**：2025-11-06  
**状态**：设计完成

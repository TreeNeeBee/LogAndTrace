# LogAndTrace TODO List

## 🎯 High Priority (P0)

### 1. Modeled Messages Implementation (AUTOSAR SWS_LOG_20001-20004)

**Status**: Not Started  
**Complexity**: High  
**Estimated Effort**: 5-7 days

#### Requirements
- [ ] **MessageId Template** - Compile-time message ID definition
  - [ ] `template<uint32_t ID, LogLevel Level> struct MessageId`
  - [ ] Unique ID validation (compile-time)
  - [ ] Fixed severity level per message type
  
- [ ] **TraceSwitch** - Compile-time routing configuration
  - [ ] `enum class TraceRoute { None, Logger, Arti, Both }`
  - [ ] `template<typename MsgId> struct TraceSwitch`
  - [ ] Zero-overhead routing with `if constexpr`
  
- [ ] **ARTI Interface** - Trace interface support
  - [ ] ARTI API wrapper (trace_arti.h)
  - [ ] TraceArti() template function
  - [ ] Integration with TraceSwitch
  
- [ ] **Log() Template Function** - Main API
  - [ ] `template<typename MsgId, typename... Params> void Log(const MsgId&, const Params&...)`
  - [ ] Variadic parameter formatting (key=value pairs)
  - [ ] Output format: `[MsgId:NNNN] key1=val1, key2=val2`
  
- [ ] **DLT Message ID Support**
  - [ ] Use `dlt_user_log_write_start_id()` API
  - [ ] Parse `[MsgId:NNNN]` format from message string
  - [ ] Extract message ID and pass to DLT
  - [ ] Support both verbose and non-verbose DLT modes
  - [ ] Configuration: `DLTConfig.useMessageId` flag
  
- [ ] **Message Catalog**
  - [ ] JSON format definition
  - [ ] Python tool: `generate_message_catalog.py`
  - [ ] Python tool: `analyze_logs.py`
  - [ ] Metadata: name, level, route, description, parameters
  
- [ ] **Documentation**
  - [ ] User guide with examples
  - [ ] Implementation guide
  - [ ] AUTOSAR compliance analysis
  - [ ] DLT non-verbose message format specification
  
- [ ] **Testing**
  - [ ] Unit tests for MessageId
  - [ ] Unit tests for TraceSwitch
  - [ ] Unit tests for Log() function
  - [ ] Integration test with DLT
  - [ ] Example application

**References**:
- AUTOSAR AP R24-11: SWS_LOG_20001, SWS_LOG_20002, SWS_LOG_20003, SWS_LOG_20004
- DLT API: `dlt_user_log_write_start_id(DltContext*, DltContextData*, DltLogLevelType, uint32_t msgid)`
- Design doc: `doc/design/MessageCatalog_Format.md`

**Dependencies**:
- Core module (TypeDef, String, Memory)
- DLT library (libdlt)
- Python 3.x (for tools)

---

### 2. Trace System Enhancement

**Status**: Not Started  
**Complexity**: Medium  
**Estimated Effort**: 3-4 days

#### Requirements
- [ ] **TraceStatus Management**
  - [ ] Per-context trace status configuration
  - [ ] Runtime trace enable/disable
  - [ ] TraceStatus enum: kDefault, kOn, kOff
  
- [ ] **Trace Output Optimization**
  - [ ] Separate trace and log paths
  - [ ] Trace-specific formatting
  - [ ] Trace buffer management
  
- [ ] **ARTI Integration**
  - [ ] Full ARTI API implementation
  - [ ] Trace message routing
  - [ ] Trace filtering
  
- [ ] **Testing**
  - [ ] Trace enable/disable tests
  - [ ] ARTI integration tests
  - [ ] Performance benchmarks

**References**:
- AUTOSAR AP: TraceStatus specification
- Current implementation: `CCommon.hpp` (TraceStatus enum)

---

## 🔧 Medium Priority (P1)

### 3. Async Logging Queue

**Status**: Design Complete (see archive/PHASE2_ASYNC_QUEUE_SUMMARY.md)  
**Complexity**: High  
**Estimated Effort**: 5-7 days

#### Requirements
- [ ] Lock-free queue implementation
- [ ] Background worker thread
- [ ] Batch write optimization
- [ ] Graceful shutdown
- [ ] Overflow handling strategy
- [ ] Performance validation (target: 1M+ logs/sec)

**Design Available**: `doc/archive/PHASE2_ASYNC_QUEUE_SUMMARY.md`

---

### 4. File Rotation Enhancement

**Status**: Basic implementation exists  
**Complexity**: Medium  
**Estimated Effort**: 2-3 days

#### Requirements
- [ ] Time-based rotation (hourly, daily)
- [ ] Compression support (gzip)
- [ ] Rotation callback hooks
- [ ] Old file cleanup policies
- [ ] Atomic rotation (no log loss)

---

### 5. Log Filtering

**Status**: Not Started  
**Complexity**: Medium  
**Estimated Effort**: 3-4 days

#### Requirements
- [ ] Per-context level filtering
- [ ] Regex-based content filtering
- [ ] Category-based filtering
- [ ] Runtime filter update
- [ ] Filter configuration (JSON)

---

## 🎨 Low Priority (P2)

### 6. Advanced Formatting

**Status**: Not Started  
**Complexity**: Low  
**Estimated Effort**: 2 days

- [ ] Custom timestamp formats
- [ ] Color schemes
- [ ] JSON output format
- [ ] XML output format

---

### 7. Network Logging

**Status**: Not Started  
**Complexity**: High  
**Estimated Effort**: 5-7 days

- [ ] TCP sink
- [ ] UDP sink
- [ ] Syslog over network
- [ ] TLS support
- [ ] Reconnection logic

---

### 8. Log Analysis Tools

**Status**: Basic tools exist (see tools/)  
**Complexity**: Medium  
**Estimated Effort**: 3-5 days

- [x] Message catalog generator (`generate_message_catalog.py`)
- [x] Log analyzer (`analyze_logs.py`)
- [ ] Real-time log viewer (TUI)
- [ ] Log aggregation tool
- [ ] Performance profiler
- [ ] DLT message parser

---

## 📚 Documentation Improvements

### 9. User Documentation

- [ ] Getting started guide
- [ ] API reference (Doxygen)
- [ ] Configuration guide
- [ ] Best practices
- [ ] Troubleshooting guide
- [ ] Migration guide (from other logging libraries)

### 10. Developer Documentation

- [ ] Architecture overview
- [ ] Code structure
- [ ] Extension guide (custom sinks)
- [ ] Performance tuning guide
- [ ] Contributing guide

---

## 🧪 Testing & Quality

### 11. Test Coverage

**Current**: 50 tests passing  
**Target**: 90%+ coverage

- [ ] Edge case tests
- [ ] Stress tests
- [ ] Memory leak tests (Valgrind)
- [ ] Thread sanitizer tests
- [ ] Performance regression tests

### 12. CI/CD Integration

- [ ] Automated testing pipeline
- [ ] Performance benchmarking
- [ ] Code coverage reporting
- [ ] Static analysis (cppcheck)
- [ ] Memory profiling

---

## 🔄 Refactoring & Optimization

### 13. Code Quality

- [ ] Reduce code duplication
- [ ] Improve error handling
- [ ] Add more assertions
- [ ] Improve comments
- [ ] Code review todos

### 14. Performance Optimization

- [ ] SIMD for string formatting
- [ ] Memory pool optimization
- [ ] Cache line alignment
- [ ] Branch prediction hints

---

## 🛠️ Infrastructure

### 15. Build System

- [ ] CMake presets
- [ ] Install target
- [ ] Package generation (DEB/RPM)
- [ ] Conan package
- [ ] vcpkg port

### 16. Platform Support

- [x] Linux x86_64
- [ ] Linux ARM64
- [ ] QNX
- [ ] Android
- [ ] Windows (for development/testing)

---

## 📊 Monitoring & Observability

### 17. Metrics & Statistics

- [ ] Log rate tracking
- [ ] Sink performance metrics
- [ ] Memory usage monitoring
- [ ] Error rate tracking
- [ ] Export metrics (Prometheus format)

---

## 🔐 Security

### 18. Security Enhancements

- [ ] Log sanitization (PII removal)
- [ ] Audit logging
- [ ] Secure log storage
- [ ] Log encryption
- [ ] Access control

---

## 📝 Notes

### Completed Major Milestones

- ✅ STL dependency removal (2025-11-06)
- ✅ Multi-sink architecture (2025-10-29)
- ✅ Zero-copy optimization (2025-10-28)
- ✅ DLT integration (2025-10-27)
- ✅ Thread-safe implementation (2025-10-26)

### Version Planning

- **v1.0.0** (Target: 2025-12): Modeled Messages + Trace + Async Queue
- **v1.1.0** (Target: 2026-Q1): Advanced filtering + Network logging
- **v2.0.0** (Target: 2026-Q2): Full AUTOSAR compliance + All features

---

**Last Updated**: 2025-11-06  
**Maintainer**: LightAP Development Team

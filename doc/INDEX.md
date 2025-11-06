# LogAndTrace Documentation Index

This directory contains comprehensive documentation for the LightAP LogAndTrace module.

## 📋 Active Documentation

### Core Documents

#### TODO.md
**Feature roadmap and task tracking**
- Priority-based task breakdown (P0/P1/P2)
- Detailed implementation plans
- Time estimates and version targets
- Current focus: Modeled Messages and Trace System

#### logConfig_template.json
**Configuration template**
- Sink configuration examples
- Log level settings
- File rotation parameters
- DLT and Syslog options

### Specifications

#### AUTOSAR_AP_SWS_LogAndTrace.pdf
**Official AUTOSAR specification**
- Standard reference document
- API requirements and specifications
- Compliance guidelines

### Design Documents

#### design/MessageCatalog_Format.md
**Message catalog specification**
- MessageId format definition
- Catalog file structure
- Tool integration guidelines

## 📦 Archived Documentation

Historical documentation and completed analysis reports are located in the `archive/` subdirectory:

### Implementation Reports (archive/)

- **CONSOLE_OUTPUT_IMPLEMENTATION.md** - Console sink implementation details
- **MULTI_SINK_SUMMARY.md** - Multi-sink architecture and implementation
- **SINK_MANAGER_INTEGRATION_ANALYSIS.md** - Sink manager integration analysis
- **REFACTORING_INNER_LOG.md** - Internal logging refactoring documentation

### Optimization Reports (archive/)

- **OPTIMIZATION_PLAN.md** - Performance optimization plan
- **OPTIMIZATION_SUMMARY.md** - Summary of completed optimizations
- **PHASE2_ASYNC_QUEUE_SUMMARY.md** - Asynchronous queue design (planned)

### Compliance & API (archive/)

- **AUTOSAR_Compliance_Analysis.md** - AUTOSAR compliance analysis
- **DLT_API_DOCUMENTATION.md** - DLT API documentation and integration guide

### Test Reports (archive/)

- **TEST_REPORT.md** - Comprehensive test results (50/50 passing)
- **BENCHMARK_REPORT.md** - Performance benchmarks (555K logs/sec)

## 🔗 Related Documentation

- **Main README**: `../README.md` - Module overview, quick start, and current status
- **Examples**: `../test/examples/` - Usage examples and sample code
- **Source Code**: `../source/inc/` - Header file documentation (Doxygen comments)
- **Tests**: `../test/unittest/` - Unit test source code

## 📂 Document Organization

```
doc/
├── INDEX.md                              # This file - documentation index
├── TODO.md                               # Feature roadmap and task list
├── AUTOSAR_AP_SWS_LogAndTrace.pdf       # AUTOSAR specification
├── logConfig_template.json              # Configuration template
├── design/                               # Design specifications
│   └── MessageCatalog_Format.md         # Message catalog format
└── archive/                              # Historical documentation
    ├── AUTOSAR_Compliance_Analysis.md
    ├── BENCHMARK_REPORT.md
    ├── CONSOLE_OUTPUT_IMPLEMENTATION.md
    ├── DLT_API_DOCUMENTATION.md
    ├── MULTI_SINK_SUMMARY.md
    ├── OPTIMIZATION_PLAN.md
    ├── OPTIMIZATION_SUMMARY.md
    ├── PHASE2_ASYNC_QUEUE_SUMMARY.md
    ├── REFACTORING_INNER_LOG.md
    ├── SINK_MANAGER_INTEGRATION_ANALYSIS.md
    └── TEST_REPORT.md
```

## 🚀 Quick Navigation

**For new contributors:**
1. Start with `../README.md` for module overview
2. Read `TODO.md` for current development priorities
3. Review `archive/TEST_REPORT.md` for current test status
4. Check `archive/BENCHMARK_REPORT.md` for performance metrics

**For feature development:**
1. Check `TODO.md` for task priorities and estimates
2. Review `design/` for relevant design documents
3. Reference `AUTOSAR_AP_SWS_LogAndTrace.pdf` for compliance requirements
4. See `archive/` for historical context and previous implementations

**For configuration:**
1. Use `logConfig_template.json` as a starting point
2. See `archive/DLT_API_DOCUMENTATION.md` for DLT-specific settings
3. Review `../test/examples/` for practical examples

---

**Last Updated**: 2025-11-06  
**Documentation Status**: Reorganized - Active docs separated from archive

doc/
├── AUTOSAR_AP_SWS_LogAndTrace.pdf      # Specification
├── logConfig_template.json             # Config template
├── DLT_API_DOCUMENTATION.md            # API docs
├── CONSOLE_OUTPUT_IMPLEMENTATION.md    # Implementation
├── MULTI_SINK_SUMMARY.md               # Architecture
├── SINK_MANAGER_INTEGRATION_ANALYSIS.md # Analysis
├── REFACTORING_INNER_LOG.md            # Development
├── OPTIMIZATION_PLAN.md                # Planning
├── OPTIMIZATION_SUMMARY.md             # Results
├── PHASE2_ASYNC_QUEUE_SUMMARY.md       # Advanced features
├── TEST_REPORT.md                      # Testing
├── BENCHMARK_REPORT.md                 # Performance
└── INDEX.md                            # This file
```

## Contributing

When adding new documentation:
1. Place specification documents (PDF) in the root of doc/
2. Place markdown documentation with descriptive ALL_CAPS names
3. Update this INDEX.md with the new document
4. Link related documents in the "Related Documentation" section
5. Keep development notes separate from user-facing documentation

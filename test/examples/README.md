# LightAP LogAndTrace Examples

This directory contains practical examples demonstrating the usage of the LightAP logging system.

## Examples

### 1. example_basic_usage.cpp
**Purpose:** Demonstrates basic logging functionality and API usage.

**Features:**
- Initialize LogManager with ConfigManager
- Use default logger
- Log messages at different levels (Fatal, Error, Warn, Info, Debug, Verbose)
- Register custom loggers with specific context
- Use location information in logs

**Usage:**
```bash
./example_basic_usage [config_file.json]
```

If no config file is provided, it looks for `config_console_file.json` in the current directory.

### 2. example_multi_thread.cpp
**Purpose:** Demonstrates thread-safe concurrent logging.

**Features:**
- Multi-threaded logging with 10 worker threads
- Synchronized thread startup
- Thread-safe message logging
- Performance statistics

**Usage:**
```bash
./example_multi_thread
```

### 3. example_file_rotation.cpp
**Purpose:** Demonstrates file logging with automatic rotation.

**Features:**
- High-volume logging (10,000 messages)
- Automatic file rotation based on size
- Backup file management
- Progress tracking

**Usage:**
```bash
./example_file_rotation
```

**Configuration:**
Configure file rotation in your JSON config:
```json
{
  "log": {
    "fileRotation": {
      "maxSize": 1048576,    // 1MB
      "maxFiles": 5          // Keep 5 backup files
    }
  }
}
```

## Configuration Files

### config_console_file.json
Console and file output with moderate log levels.

### config_dlt.json
DLT (Diagnostic Log and Trace) sink configuration.

### config_syslog.json
System log (syslog) integration.

### config_all_sinks.json
All available sinks enabled simultaneously.

## Building Examples

From the build directory:
```bash
make example_basic_usage
make example_multi_thread
make example_file_rotation
# Or build all at once:
make example_basic_usage example_multi_thread example_file_rotation
```

## Running Examples

1. Navigate to the build directory containing the examples
2. Copy the desired config file to the current directory
3. Run the example:

```bash
cd build/modules/LogAndTrace
cp ../../../modules/LogAndTrace/test/examples/config_console_file.json .
./example_basic_usage
```

## Common Issues

**Issue:** "Failed to initialize LogManager"
- **Solution:** Ensure config file exists in the current directory or provide correct path

**Issue:** Logs not appearing
- **Solution:** Check log level configuration in config file - ensure it matches or is below the level you're logging at

**Issue:** File rotation not working
- **Solution:** Verify `fileRotation` section in config and ensure `maxSize` is set appropriately

## See Also

- Unit tests in `test/unittest/` for comprehensive API testing
- Benchmarks in `test/benchmark/` for performance analysis
- Source code in `source/inc/` and `source/src/` for implementation details

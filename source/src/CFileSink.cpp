/**
 * @file        CFileSink.cpp
 * @author      ddkv587 ( ddkv587@gmail.com )
 * @brief       File sink implementation
 * @date        2025-10-28
 */

#include "CFileSink.hpp"
#include <cstring>
#include <ctime>

namespace lap
{
namespace log
{
    FileSink::FileSink(
        core::StringView filePath,
        core::Size maxSize,
        core::UInt32 maxFiles,
        LogLevel minLevel,
        core::StringView appId
    ) noexcept
        : m_filePath(filePath.data(), filePath.size())
        , m_file()
        , m_maxSize(maxSize)
        , m_maxFiles(maxFiles)
        , m_currentSize(0)
        , m_enabled(true)
        , m_minLevel(minLevel)
    {
        // Store appId (max 4 bytes)
        size_t appIdLen = (appId.size() > 4) ? 4 : appId.size();
        std::memcpy(m_appId, appId.data(), appIdLen);
        m_appId[appIdLen] = '\0';
        
        openFile();
    }
    
    FileSink::~FileSink() noexcept
    {
        // Force sync to disk before closing
        if (m_file.isOpen()) {
            m_file.fsync();
        }
        closeFile();
    }
    
    void FileSink::write(
        core::UInt64 timestamp,
        core::UInt32 threadId,
        LogLevelType level,
        core::StringView contextId,
        core::StringView message
    ) noexcept
    {
        UNUSED(threadId);
        
        if (!isEnabled() || !m_file.isOpen()) {
            return;
        }
        
        // Message is from LogStream buffer with MAX_LOG_SIZE=200 bytes fixed size
        // Format: [YYYY-MM-DD HH:MM:SS.mmm] [LEVEL] [CONTEXT] message\n
        // Timestamp prefix: ~27 bytes, level: 7 bytes, context: variable
        // Total prefix typically < 100 bytes, so 512 byte buffer is sufficient
        
        // Convert timestamp
        time_t seconds = timestamp / 1000000;
        core::UInt32 microseconds = timestamp % 1000000;
        core::UInt32 milliseconds = microseconds / 1000;
        
        struct tm tmInfo;
        localtime_r(&seconds, &tmInfo);
        
        // Get level name (5 chars fixed width)
        const char* levelName;
        switch (level) {
            case 0x01:  levelName = "FATAL"; break;
            case 0x02:  levelName = "ERROR"; break;
            case 0x03:  levelName = "WARN "; break;
            case 0x04:  levelName = "INFO "; break;
            case 0x05:  levelName = "DEBUG"; break;
            case 0x06:  levelName = "VERB "; break;
            default:    levelName = "UNKNW"; break;
        }
        
        // Optimized: Fixed-size buffer sized for prefix + MAX_LOG_SIZE + newline
        // Format: [timestamp] [APPID] [LEVEL] [context] message\n
        // 128 (prefix reserve) + 200 (MAX_LOG_SIZE) + 1 (newline) = 329, rounded to 512 for alignment
        char buffer[512];
        
        // Format timestamp + appId + level + context prefix
        int prefixLen = snprintf(
            buffer,
            sizeof(buffer),
            "[%04d-%02d-%02d %02d:%02d:%02d.%03u] [%s] [%s] [%.*s] ",
            tmInfo.tm_year + 1900,
            tmInfo.tm_mon + 1,
            tmInfo.tm_mday,
            tmInfo.tm_hour,
            tmInfo.tm_min,
            tmInfo.tm_sec,
            milliseconds,
            m_appId,
            levelName,
            static_cast<int>(contextId.size()), contextId.data()
        );
        
        if (prefixLen <= 0 || prefixLen >= static_cast<int>(sizeof(buffer))) {
            return;  // Format error or buffer too small
        }
        
        // Calculate available space for message
        size_t availableSpace = sizeof(buffer) - prefixLen - 2; // Reserve 2 bytes for \n and \0
        size_t msgLen = message.size();
        
        // Truncate message if necessary
        if (msgLen > availableSpace) {
            msgLen = availableSpace;
        }
        
        // Copy message
        std::memcpy(buffer + prefixLen, message.data(), msgLen);
        size_t totalLen = prefixLen + msgLen;
        buffer[totalLen] = '\n';
        totalLen++;
        
        // Direct unbuffered write via fd (O_APPEND ensures atomic append)
        core::Int64 bytesWritten = m_file.write(buffer, totalLen);
        if (bytesWritten > 0) {
            m_currentSize += static_cast<core::Size>(bytesWritten);
            
            // Check if rotation is needed
            checkRotation();
        }
    }
    
    void FileSink::flush() noexcept
    {
        // Note: flush() only ensures data is sent to OS buffer cache
        // It does NOT force fsync() - that's expensive and done only in destructor/rotate
        // For unbuffered fd writes with O_APPEND, data is already in kernel buffer
        // No explicit action needed here as ::write() already completed
    }
    
    core::Bool FileSink::shouldLog(LogLevel level) const noexcept
    {
        if (!m_enabled || !m_file.isOpen()) {
            return false;
        }
        
        // Lower numeric value = higher priority
        using LevelType = typename std::underlying_type<LogLevel>::type;
        return static_cast<LevelType>(level) <= static_cast<LevelType>(m_minLevel);
    }
    
    core::Bool FileSink::rotate() noexcept
    {
        if (!m_file.isOpen()) {
            return false;
        }
        
        // Acquire exclusive lock for rotation (multi-process safety)
        if (!m_file.lock(true)) {
            return false;  // Failed to lock, skip rotation
        }
        
        // Close current file
        closeFile();
        
        // Rotate backup files: app.log.N -> app.log.N+1
        for (core::Int32 i = m_maxFiles - 1; i >= 1; --i) {
            core::String oldPath = m_filePath + "." + std::to_string(i);
            core::String newPath = m_filePath + "." + std::to_string(i + 1);
            
            // Delete oldest backup if exists
            if (i == static_cast<core::Int32>(m_maxFiles) - 1) {
                core::File::remove(newPath);
            }
            
            // Rename (move operation)
            core::File::rename(oldPath, newPath);
        }
        
        // Rename current log file to .1
        core::String backupPath = m_filePath + ".1";
        core::File::rename(m_filePath, backupPath);
        
        // Open new log file
        m_currentSize = 0;
        return openFile();
        
        // Lock is automatically released when file is closed/reopened
    }
    
    core::Bool FileSink::openFile() noexcept
    {
        using OpenMode = core::File::OpenMode;
        
        // Open with O_APPEND for atomic multi-process writes
        // O_APPEND ensures kernel-level atomic positioning + write operations
        // Single write() syscall with complete log line (< PIPE_BUF) guarantees atomicity
        // No O_SYNC needed - buffered I/O is fine as long as writes are not interrupted
        core::UInt32 flags = static_cast<core::UInt32>(OpenMode::WriteOnly) |
                              static_cast<core::UInt32>(OpenMode::Create) |
                              static_cast<core::UInt32>(OpenMode::Append) |
                              static_cast<core::UInt32>(OpenMode::CloseOnExec);
        
        if (!m_file.open(m_filePath, flags, 0644)) {
            return false;
        }
        
        // Get current file size
        struct stat st;
        if (m_file.fstat(&st)) {
            m_currentSize = st.st_size;
        }
        
        return true;
    }
    
    void FileSink::closeFile() noexcept
    {
        m_file.close();
    }
    
    void FileSink::checkRotation() noexcept
    {
        if (m_maxSize > 0 && m_currentSize >= m_maxSize) {
            rotate();
        }
    }

} // namespace log
} // namespace lap

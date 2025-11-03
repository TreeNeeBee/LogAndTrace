/**
 * @file        CConsoleSink.cpp
 * @author      ddkv587 ( ddkv587@gmail.com )
 * @brief       Console sink implementation
 * @date        2025-10-28
 */

#include "CConsoleSink.hpp"
#include <cstdio>
#include <ctime>
#include <chrono>

namespace lap
{
namespace log
{
    ConsoleSink::ConsoleSink(core::Bool colorized, LogLevel minLevel) noexcept
        : m_enabled(true)
        , m_colorized(colorized)
        , m_minLevel(minLevel)
    {
    }
    
    void ConsoleSink::write(
        core::UInt64 timestamp,
        core::UInt32 threadId,
        LogLevelType level,
        core::StringView contextId,
        core::StringView message
    ) noexcept
    {
        UNUSED(threadId);
        
        if (!m_enabled) {
            return;
        }
        
        // Format timestamp
        char timeBuffer[16];
        formatTimestamp(timestamp, timeBuffer);
        
        // Get level info
        const char* levelColor = m_colorized ? getLevelColor(level) : "";
        const char* levelName = getLevelName(level);
        const char* resetColor = m_colorized ? ANSI_RESET : "";
        const char* boldColor = m_colorized ? ANSI_BOLD : "";
        
        // Output formatted log
        // Format: [BOLD][COLOR][TIME] [LEVEL] [CONTEXT][RESET] message\n
        fprintf(stderr, "%s%s[%s] [%s] [%.*s]%s %.*s\n",
                boldColor,
                levelColor,
                timeBuffer,
                levelName,
                static_cast<int>(contextId.size()), contextId.data(),
                resetColor,
                static_cast<int>(message.size()), message.data());
    }
    
    void ConsoleSink::flush() noexcept
    {
        if (m_enabled) {
            fflush(stderr);
        }
    }
    
    core::Bool ConsoleSink::shouldLog(LogLevel level) const noexcept
    {
        if (!m_enabled) {
            return false;
        }
        
        // Lower numeric value = higher priority
        // Fatal(1) > Error(2) > Warn(3) > Info(4) > Debug(5) > Verbose(6)
        using LevelType = typename std::underlying_type<LogLevel>::type;
        return static_cast<LevelType>(level) <= static_cast<LevelType>(m_minLevel);
    }
    
    const char* ConsoleSink::getLevelColor(LogLevelType level) const noexcept
    {
        switch (level) {
            case 0x01:  return ANSI_RED;     // Fatal
            case 0x02:  return ANSI_RED;     // Error
            case 0x03:  return ANSI_YELLOW;  // Warn
            case 0x04:  return ANSI_GREEN;   // Info
            case 0x05:  return ANSI_CYAN;    // Debug
            case 0x06:  return ANSI_WHITE;   // Verbose
            default:    return ANSI_WHITE;
        }
    }
    
    const char* ConsoleSink::getLevelName(LogLevelType level) const noexcept
    {
        switch (level) {
            case 0x01:  return "FATAL";  // Fatal
            case 0x02:  return "ERROR";  // Error
            case 0x03:  return "WARN ";  // Warn
            case 0x04:  return "INFO ";  // Info
            case 0x05:  return "DEBUG";  // Debug
            case 0x06:  return "VERB ";  // Verbose
            default:    return "UNKNW";
        }
    }
    
    core::Size ConsoleSink::formatTimestamp(core::UInt64 timestamp, char* buffer) const noexcept
    {
        // Convert microseconds to seconds and microseconds
        time_t seconds = timestamp / 1000000;
        core::UInt32 microseconds = timestamp % 1000000;
        core::UInt32 milliseconds = microseconds / 1000;
        
        // Convert to local time
        struct tm tmInfo;
        localtime_r(&seconds, &tmInfo);
        
        // Format as HH:MM:SS.mmm
        return static_cast<core::Size>(
            snprintf(buffer, 16, "%02d:%02d:%02d.%03u",
                    tmInfo.tm_hour,
                    tmInfo.tm_min,
                    tmInfo.tm_sec,
                    milliseconds)
        );
    }
    
} // namespace log
} // namespace lap

/**
 * @file        CSyslogSink.cpp
 * @author      ddkv587 ( ddkv587@gmail.com )
 * @brief       Syslog sink implementation
 * @date        2025-10-28
 */

#include "CSyslogSink.hpp"
#include <cstring>

namespace lap
{
namespace log
{
    SyslogSink::SyslogSink(core::StringView identity, core::Int32 facility, LogLevel minLevel) noexcept
        : m_identity(identity.data())
        , m_facility(facility)
        , m_enabled(true)
        , m_minLevel(minLevel)
        , m_opened(false)
    {
        // Open connection to syslog
        // LOG_PID: include PID in messages
        // LOG_CONS: write to console if syslog unavailable
        openlog(m_identity.c_str(), LOG_PID | LOG_CONS, m_facility);
        m_opened = true;
    }
    
    SyslogSink::~SyslogSink() noexcept
    {
        if (m_opened) {
            closelog();
            m_opened = false;
        }
    }
    
    void SyslogSink::write(
        core::UInt64 timestamp,
        core::UInt32 threadId,
        LogLevelType level,
        core::StringView contextId,
        core::StringView message
    ) noexcept
    {
        UNUSED(timestamp);
        UNUSED(threadId);
        
        if (!isEnabled()) {
            return;
        }
        
        // Get priority from log level
        core::Int32 priority = convertPriority(level);
        
        // Format: [CONTEXT] message
        // Note: syslog() adds timestamp automatically
        if (contextId.empty()) {
            syslog(priority, "%.*s", 
                   static_cast<int>(message.size()), 
                   message.data());
        } else {
            syslog(priority, "[%.*s] %.*s",
                   static_cast<int>(contextId.size()),
                   contextId.data(),
                   static_cast<int>(message.size()),
                   message.data());
        }
    }
    
    void SyslogSink::flush() noexcept
    {
        // Syslog flushes automatically, nothing to do
    }
    
    core::Bool SyslogSink::shouldLog(LogLevel level) const noexcept
    {
        if (!m_enabled) {
            return false;
        }
        
        // Lower numeric value = higher priority
        using LevelType = typename std::underlying_type<LogLevel>::type;
        return static_cast<LevelType>(level) <= static_cast<LevelType>(m_minLevel);
    }
    
    core::Int32 SyslogSink::convertPriority(LogLevelType level) const noexcept
    {
        // Map log levels to syslog priorities
        switch (level) {
            case 0x01:  return LOG_CRIT;     // Fatal -> Critical conditions
            case 0x02:  return LOG_ERR;      // Error -> Error conditions
            case 0x03:  return LOG_WARNING;  // Warn -> Warning conditions
            case 0x04:  return LOG_INFO;     // Info -> Informational messages
            case 0x05:  return LOG_DEBUG;    // Debug -> Debug-level messages
            case 0x06:  return LOG_DEBUG;    // Verbose -> Debug in syslog
            default:    return LOG_NOTICE;   // Normal but significant
        }
    }
    
} // namespace log
} // namespace lap

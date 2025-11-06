/**
 * @file        CSyslogSink.hpp
 * @author      ddkv587 ( ddkv587@gmail.com )
 * @brief       Syslog sink for system logging
 * @date        2025-10-28
 * @details     Outputs logs to system syslog daemon
 * @copyright   Copyright (c) 2025
 */

#ifndef LAP_LOG_SYSLOGSINK_HPP
#define LAP_LOG_SYSLOGSINK_HPP

#include "ISink.hpp"
#include <lap/core/CMemory.hpp>
#include <syslog.h>

namespace lap
{
namespace log
{
    /**
     * @brief Syslog sink for Unix/Linux system logging
     * 
     * Features:
     * - Integration with system syslog daemon
     * - Automatic priority mapping from DLT levels
     * - Configurable facility and options
     * - Thread-safe syslog() calls
     */
    class SyslogSink : public ISink
    {
    public:
        IMP_OPERATOR_NEW(SyslogSink)
        
        /**
         * @brief Constructor
         * @param identity Process identity string (shown in logs)
         * @param facility Syslog facility (LOG_USER, LOG_DAEMON, etc.)
         * @param minLevel Minimum log level to output
         */
        explicit SyslogSink(
            core::StringView identity = "LightAP",
            core::Int32 facility = LOG_USER,
            LogLevel minLevel = LogLevel::kVerbose
        ) noexcept;
        
        virtual ~SyslogSink() noexcept override;
        
        // ISink interface implementation
        virtual void write(
            core::UInt64 timestamp,
            core::UInt32 threadId,
            LogLevelType level,
            core::StringView contextId,
            core::StringView message
        ) noexcept override;
        
        virtual void flush() noexcept override;
        virtual core::Bool isEnabled() const noexcept override { return m_enabled; }
        virtual core::StringView getName() const noexcept override { return "Syslog"; }
        virtual void setLevel(LogLevel level) noexcept override { m_minLevel = level; }
        virtual core::Bool shouldLog(LogLevel level) const noexcept override;
        
        /**
         * @brief Enable/disable this sink
         * @param enabled Enable state
         */
        void setEnabled(core::Bool enabled) noexcept { m_enabled = enabled; }
        
    private:
        /**
         * @brief Convert LogLevel to syslog priority
         * @param level Log level
         * @return Syslog priority (LOG_ERR, LOG_WARNING, etc.)
         */
        core::Int32 convertPriority(LogLevelType level) const noexcept;
        
    private:
        core::String    m_identity;     ///< Process identity
        core::Int32     m_facility;     ///< Syslog facility
        core::Bool      m_enabled;      ///< Enable state
        LogLevel        m_minLevel;     ///< Minimum log level
        core::Bool      m_opened;       ///< Whether openlog() was called
    };
    
} // namespace log
} // namespace lap

#endif // LAP_LOG_SYSLOGSINK_HPP

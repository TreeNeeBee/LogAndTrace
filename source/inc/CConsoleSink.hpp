/**
 * @file        CConsoleSink.hpp
 * @author      ddkv587 ( ddkv587@gmail.com )
 * @brief       Console output sink with ANSI color support
 * @date        2025-10-28
 * @details     Outputs formatted logs to stderr with colors and timestamps
 * @copyright   Copyright (c) 2025
 */

#ifndef LAP_LOG_CONSOLESINK_HPP
#define LAP_LOG_CONSOLESINK_HPP

#include "ISink.hpp"
#include <core/CMemory.hpp>

namespace lap
{
namespace log
{
    /**
     * @brief Console sink for terminal output
     * 
     * Features:
     * - ANSI color codes for different log levels
     * - Formatted timestamp (HH:MM:SS.mmm)
     * - Thread-safe output to stderr
     */
    class ConsoleSink : public ISink
    {
    public:
        IMP_OPERATOR_NEW(ConsoleSink)
        
        /**
         * @brief Constructor
         * @param colorized Enable ANSI colors (default: true)
         * @param minLevel Minimum log level to output (default: Verbose)
         */
        explicit ConsoleSink(core::Bool colorized = true, LogLevel minLevel = LogLevel::kVerbose) noexcept;
        
        virtual ~ConsoleSink() noexcept override = default;
        
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
        virtual core::StringView getName() const noexcept override { return "Console"; }
        virtual void setLevel(LogLevel level) noexcept override { m_minLevel = level; }
        virtual core::Bool shouldLog(LogLevel level) const noexcept override;
        
        /**
         * @brief Enable/disable this sink
         * @param enabled Enable state
         */
        void setEnabled(core::Bool enabled) noexcept { m_enabled = enabled; }
        
        /**
         * @brief Enable/disable colorized output
         * @param colorized Colorized state
         */
        void setColorized(core::Bool colorized) noexcept { m_colorized = colorized; }
        
    private:
        /**
         * @brief Get ANSI color code for log level
         * @param level Log level
         * @return Color code string
         */
        const char* getLevelColor(LogLevelType level) const noexcept;
        
        /**
         * @brief Get level name string
         * @param level Log level
         * @return Level name (5 chars, padded)
         */
        const char* getLevelName(LogLevelType level) const noexcept;
        
        /**
         * @brief Format timestamp from microseconds
         * @param timestamp Microseconds since epoch
         * @param buffer Output buffer (at least 16 bytes)
         * @return Number of characters written
         */
        core::Size formatTimestamp(core::UInt64 timestamp, char* buffer) const noexcept;
        
    private:
        core::Bool  m_enabled;      ///< Enable state
        core::Bool  m_colorized;    ///< Use ANSI colors
        LogLevel    m_minLevel;     ///< Minimum log level
    };
    
} // namespace log
} // namespace lap

#endif // LAP_LOG_CONSOLESINK_HPP

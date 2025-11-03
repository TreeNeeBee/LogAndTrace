/**
 * @file        ISink.hpp
 * @author      ddkv587 ( ddkv587@gmail.com )
 * @brief       Log sink interface for multi-destination logging
 * @date        2025-10-28
 * @details     Provides abstract interface for various log outputs (Console/File/Syslog/DLT)
 * @copyright   Copyright (c) 2025
 * @note
 * sdk:
 * platform:
 * project:     LightAP
 * @version
 * <table>
 * <tr><th>Date        <th>Version  <th>Author          <th>Description
 * <tr><td>2025/10/28  <td>1.0      <td>ddkv587         <td>init version
 * </table>
 */

#ifndef LAP_LOG_ISINK_HPP
#define LAP_LOG_ISINK_HPP

#include <core/CTypedef.hpp>
#include <core/CMemory.hpp>
#include <core/CString.hpp>
#include "CCommon.hpp"

namespace lap
{
namespace log
{
    /**
     * @brief Log entry structure for zero-copy optimization
     * 
     * Memory layout: [LogEntry header][context_id string][message string]
     * Aligned to cache line boundary for performance
     */
    struct alignas(64) LogEntry
    {
        core::UInt64    timestamp;      ///< Microseconds since epoch
        core::UInt32    threadId;       ///< Thread ID
        LogLevelType    level;          ///< Log level
        core::UInt16    contextIdLen;   ///< Length of context ID
        core::UInt16    messageLen;     ///< Length of message
        
        /**
         * @brief Get context ID string from flexible array
         * @return StringView pointing to context ID
         */
        core::StringView getContextId() const noexcept
        {
            return core::StringView(reinterpret_cast<const char*>(this + 1), contextIdLen);
        }
        
        /**
         * @brief Get message string from flexible array
         * @return StringView pointing to message
         */
        core::StringView getMessage() const noexcept
        {
            const char* msgPtr = reinterpret_cast<const char*>(this + 1) + contextIdLen;
            return core::StringView(msgPtr, messageLen);
        }
        
        /**
         * @brief Calculate total size including flexible array
         * @return Total size in bytes
         */
        static core::Size calculateSize(core::Size contextLen, core::Size msgLen) noexcept
        {
            return sizeof(LogEntry) + contextLen + msgLen;
        }
    };
    
    /**
     * @brief Abstract log sink interface
     * 
     * All concrete sinks (Console, File, Syslog, DLT) implement this interface.
     * Thread-safety is guaranteed by the caller (SinkManager).
     */
    class ISink
    {
    public:
        virtual ~ISink() noexcept = default;
        
        /**
         * @brief Write a log message to the sink
         * @param timestamp Microseconds since epoch
         * @param threadId Thread ID
         * @param level Log level
         * @param contextId Context ID string
         * @param message Log message string
         * @note Should be fast and non-blocking if possible
         */
        virtual void write(
            core::UInt64 timestamp,
            core::UInt32 threadId,
            LogLevelType level,
            core::StringView contextId,
            core::StringView message
        ) noexcept = 0;
        
        /**
         * @brief Flush buffered data to underlying storage
         * @note Called periodically or on critical logs
         */
        virtual void flush() noexcept = 0;
        
        /**
         * @brief Check if this sink is enabled
         * @return true if enabled, false otherwise
         */
        virtual core::Bool isEnabled() const noexcept = 0;
        
        /**
         * @brief Get sink name for identification
         * @return Sink name
         */
        virtual core::StringView getName() const noexcept = 0;
        
        /**
         * @brief Set minimum log level for this sink
         * @param level Minimum level to output
         */
        virtual void setLevel(LogLevel level) noexcept = 0;
        
        /**
         * @brief Check if a log level should be output
         * @param level Log level to check
         * @return true if should output, false otherwise
         */
        virtual core::Bool shouldLog(LogLevel level) const noexcept = 0;
    };
    
} // namespace log
} // namespace lap

#endif // LAP_LOG_ISINK_HPP

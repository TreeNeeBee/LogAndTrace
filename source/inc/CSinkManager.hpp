/**
 * @file        CSinkManager.hpp
 * @author      ddkv587 ( ddkv587@gmail.com )
 * @brief       Sink manager for multi-destination logging
 * @date        2025-10-28
 * @details     Manages multiple log sinks and distributes log entries
 * @copyright   Copyright (c) 2025
 */

#ifndef LAP_LOG_SINKMANAGER_HPP
#define LAP_LOG_SINKMANAGER_HPP

#include "ISink.hpp"
#include <lap/core/CTypedef.hpp>
#include <lap/core/CMemory.hpp>
#include <lap/core/CString.hpp>
#include <lap/core/CSync.hpp>

namespace lap
{
namespace log
{
    /**
     * @brief Manager for multiple log sinks
     * 
     * Features:
     * - Dynamic sink registration
     * - Parallel writing to multiple destinations
     * - Thread-safe operation
     * - Centralized flush control
     * - Global minimum log level filtering
     */
    class SinkManager
    {
    public:
        IMP_OPERATOR_NEW(SinkManager)
        SinkManager() noexcept 
            : m_globalMinLevel(LogLevel::kVerbose) // Default: allow all levels (most permissive)
        {}
        ~SinkManager() noexcept = default;
        
        // Non-copyable, non-movable
        SinkManager(const SinkManager&) = delete;
        SinkManager& operator=(const SinkManager&) = delete;
        
        /**
         * @brief Set global minimum log level
         * @param level Minimum level to log (default: kWarn)
         * @details All logs below this level will be filtered out before reaching sinks
         */
        void setGlobalMinLevel(LogLevel level) noexcept
        {
            core::LockGuard lock(m_mutex);
            m_globalMinLevel = level;
        }
        
        /**
         * @brief Get current global minimum log level
         * @return Current minimum level
         */
        LogLevel getGlobalMinLevel() const noexcept
        {
            core::LockGuard lock(m_mutex);
            return m_globalMinLevel;
        }
        
        /**
         * @brief Add a sink to the manager
         * @param sink Unique pointer to sink
         */
        void addSink(core::UniqueHandle<ISink> sink) noexcept;
        
        /**
         * @brief Remove a sink by name
         * @param name Sink name
         * @return true if removed, false if not found
         */
        core::Bool removeSink(core::StringView name) noexcept;
        
        /**
         * @brief Get a sink by name
         * @param name Sink name
         * @return Pointer to sink or nullptr if not found
         */
        ISink* getSink(core::StringView name) noexcept;
        
        /**
         * @brief Write log from LogStream to all enabled sinks
         * @param stream LogStream containing the log data
         */
        void write(const class LogStream& stream) noexcept;
        
        /**
         * @brief Flush all sinks
         */
        void flushAll() noexcept;
        
        /**
         * @brief Get number of registered sinks
         * @return Sink count
         */
        core::Size getSinkCount() const noexcept
        {
            core::LockGuard lock(m_mutex);
            return m_sinks.size();
        }
        
        /**
         * @brief Clear all sinks
         */
        void clearAll() noexcept;
        
        /**
         * @brief Check if any sink should log this level
         * @param level Log level to check
         * @return true if at least one sink will output this level
         * @details First checks global minimum level, then checks individual sinks
         */
        core::Bool shouldLog(LogLevel level) const noexcept;
        
    private:
        mutable core::Mutex                     m_mutex;            ///< Mutex for thread safety
        core::Vector<core::UniqueHandle<ISink>> m_sinks;            ///< Registered sinks
        LogLevel                                m_globalMinLevel;   ///< Global minimum log level
    };
    
} // namespace log
} // namespace lap

#endif // LAP_LOG_SINKMANAGER_HPP

/**
 * @file        CSinkManager.cpp
 * @author      ddkv587 ( ddkv587@gmail.com )
 * @brief       Sink manager implementation
 * @date        2025-10-28
 */

#include "CSinkManager.hpp"
#include "CLogStream.hpp"
#include "CLogger.hpp"
#include <algorithm>
#include <chrono>

namespace lap
{
namespace log
{
    void SinkManager::addSink(core::UniqueHandle<ISink> sink) noexcept
    {
        if (!sink) {
            return;
        }
        
        std::lock_guard<std::mutex> lock(m_mutex);
        m_sinks.push_back(std::move(sink));
    }
    
    core::Bool SinkManager::removeSink(core::StringView name) noexcept
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = std::find_if(m_sinks.begin(), m_sinks.end(),
            [name](const core::UniqueHandle<ISink>& sink) {
                return sink && sink->getName() == name;
            });
        
        if (it != m_sinks.end()) {
            m_sinks.erase(it);
            return true;
        }
        
        return false;
    }
    
    ISink* SinkManager::getSink(core::StringView name) noexcept
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = std::find_if(m_sinks.begin(), m_sinks.end(),
            [name](const core::UniqueHandle<ISink>& sink) {
                return sink && sink->getName() == name;
            });
        
        return (it != m_sinks.end()) ? it->get() : nullptr;
    }
    
    void SinkManager::write(const LogStream& stream) noexcept
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Convert LogLevelType to LogLevel
        LogLevel level;
        LogLevelType levelValue = stream.getLevel();
        switch (levelValue) {
            case 0x01:  level = LogLevel::kFatal; break;    // Fatal
            case 0x02:  level = LogLevel::kError; break;    // Error
            case 0x03:  level = LogLevel::kWarn; break;     // Warn
            case 0x04:  level = LogLevel::kInfo; break;     // Info
            case 0x05:  level = LogLevel::kDebug; break;    // Debug
            case 0x06:  level = LogLevel::kVerbose; break;  // Verbose
            default:    level = LogLevel::kVerbose; break;
        }
        
        // Global level filter - early return if below global minimum
        if (level > m_globalMinLevel) {
            return;
        }
        
        // Get timestamp
        auto now = std::chrono::system_clock::now();
        core::UInt64 timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
            now.time_since_epoch()).count();
        core::UInt32 threadId = 0;  // TODO: Get actual thread ID
        
        // Get direct references (zero-copy from LogStream buffer)
        core::StringView contextId = stream.getLogger().getContextId();
        core::StringView message(stream.getBuffer(), stream.getBufferSize());
        
        // Write to all enabled sinks - pass LogStream buffer directly (zero-copy)
        // Each sink is responsible for its own formatting if needed
        for (auto& sink : m_sinks) {
            if (sink && sink->isEnabled() && sink->shouldLog(level)) {
                sink->write(timestamp, threadId, levelValue, contextId, message);
            }
        }
    }
    
    void SinkManager::flushAll() noexcept
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        for (auto& sink : m_sinks) {
            if (sink && sink->isEnabled()) {
                sink->flush();
            }
        }
    }
    
    void SinkManager::clearAll() noexcept
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_sinks.clear();
    }
    
    core::Bool SinkManager::shouldLog(LogLevel level) const noexcept
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Check global minimum level first
        if (level > m_globalMinLevel) {
            return false;
        }
        
        // Check if any sink will output this level
        for (const auto& sink : m_sinks) {
            if (sink && sink->isEnabled() && sink->shouldLog(level)) {
                return true;
            }
        }
        
        return false;
    }
    
} // namespace log
} // namespace lap

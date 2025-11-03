/**
 * @file        CDLTSink.cpp
 * @author      GitHub Copilot
 * @brief       DLT sink implementation
 * @date        2025-10-29
 */

#include "CDLTSink.hpp"
#include <cstring>
#include <ctime>

namespace lap
{
namespace log
{
    DLTSink::DLTSink(
        const DLTConfig& config,
        LogLevel minLevel
    ) noexcept
        : m_enabled(true)
        , m_minLevel(minLevel)
        , m_dltInitialized(false)
        , m_appRegistered(false)
        , m_appId(config.appId)
    {
        // Configure DLT options
        dlt_with_session_id(config.withSessionId);
        dlt_with_timestamp(config.withTimestamp);
        dlt_with_ecu_id(config.withEcuId);
        
        if (config.logMarker) {
            DLT_LOG_MARKER();
        }
        
        if (config.verboseMode) {
            DLT_VERBOSE_MODE();
        }
        
        // Check DLT library version
        auto ret = dlt_check_library_version(_DLT_PACKAGE_MAJOR_VERSION, _DLT_PACKAGE_MINOR_VERSION);
        if (DLT_RETURN_OK != ret) {
            fprintf(stderr, "[LightAP] DLTSink: dlt_check_library_version error %d\n", static_cast<int>(ret));
            return;
        }
        
        // Register DLT application
        ret = dlt_register_app(config.appId.c_str(), config.appDesc.c_str());
        if (DLT_RETURN_OK != ret) {
            fprintf(stderr, "[LightAP] DLTSink: dlt_register_app error %d\n", static_cast<int>(ret));
            return;
        }
        
        m_appRegistered = true;
        
        // Set application log level and trace status limits
        DLT_SET_APPLICATION_LL_TS_LIMIT(
            toDltLevel(config.defaultLogLevel),
            toDltTraceStatus(config.traceStatus)
        );
        
        // Register default context
        ret = dlt_register_context_ll_ts(
            &m_defaultContext,
            config.contextId.c_str(),
            config.contextDesc.c_str(),
            toDltLevel(config.defaultLogLevel),
            toDltTraceStatus(config.traceStatus)
        );
        
        if (DLT_RETURN_OK == ret) {
            m_dltInitialized = true;
        } else {
            fprintf(stderr, "[LightAP] DLTSink: dlt_register_context error %d\n", static_cast<int>(ret));
        }
    }
    
    DLTSink::~DLTSink() noexcept
    {
        if (m_dltInitialized) {
            dlt_unregister_context(&m_defaultContext);
        }
        
        if (m_appRegistered) {
            dlt_unregister_app();
        }
    }
    
    void DLTSink::write(
        core::UInt64 timestamp,
        core::UInt32 threadId,
        LogLevelType level,
        core::StringView contextId,
        core::StringView message
    ) noexcept
    {
        UNUSED(timestamp);
        UNUSED(threadId);
        
        if (!m_enabled || !m_dltInitialized) {
            return;
        }
        
        // Get DLT context
        DltContext* ctx = getContext(contextId);
        if (!ctx) {
            return;
        }
        
        // Convert to DLT level
        DltLogLevelType dltLevel = toDltLevel(level);
        
        // Write to DLT
        DltContextData contextData;
        int ret = dlt_user_log_write_start(ctx, &contextData, dltLevel);
        if (ret > 0) {
            // DLT has a maximum message size of 1390 bytes (DLT_USER_BUF_MAX_SIZE)
            // Our MAX_LOG_SIZE is 200, so we should be safe, but truncate just in case
            uint16_t msgLen = static_cast<uint16_t>(message.size());
            if (msgLen > 1300) {  // Leave some margin for DLT headers
                msgLen = 1300;
            }
            
            // Write message as string with explicit length (safe for non-null-terminated StringView)
            ret = dlt_user_log_write_sized_string(&contextData, message.data(), msgLen);
            
            // Finish log entry (must be called even if write failed)
            dlt_user_log_write_finish(&contextData);
            
            // Log error if write failed
            if (ret < 0) {
                fprintf(stderr, "[LightAP] DLTSink: dlt_user_log_write_sized_string failed with %d\n", ret);
            }
        }
    }
    
    void DLTSink::flush() noexcept
    {
        // DLT flushes automatically
    }
    
    core::Bool DLTSink::shouldLog(LogLevel level) const noexcept
    {
        // Check if level is enabled
        // Lower numeric value = higher priority
        using LevelType = typename std::underlying_type<LogLevel>::type;
        return static_cast<LevelType>(level) <= static_cast<LevelType>(m_minLevel);
    }
    
    core::Bool DLTSink::registerContext(
        core::StringView contextId, 
        core::StringView contextDesc,
        LogLevel level,
        TraceStatus status
    ) noexcept
    {
        // For now, we use the default context
        // In a complete implementation, this would manage a pool of contexts
        // and register new contexts dynamically
        UNUSED(contextId);
        UNUSED(contextDesc);
        UNUSED(level);
        UNUSED(status);
        return m_dltInitialized;
    }
    
    DltContext* DLTSink::getContext(core::StringView contextId) noexcept
    {
        // For now, always return default context
        // TODO: Implement context lookup/creation
        UNUSED(contextId);
        return &m_defaultContext;
    }

} // namespace log
} // namespace lap

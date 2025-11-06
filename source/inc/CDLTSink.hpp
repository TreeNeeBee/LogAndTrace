/**
 * @file        CDLTSink.hpp
 * @author      GitHub Copilot
 * @brief       DLT sink implementation
 * @date        2025-10-29
 * @details     Encapsulates all DLT API operations
 * @copyright   Copyright (c) 2025
 */

#ifndef LAP_LOG_DLTSINK_HPP
#define LAP_LOG_DLTSINK_HPP

#include "ISink.hpp"
#include <lap/core/CMemory.hpp>
#include <lap/core/CString.hpp>
#include "dlt/dlt.h"

namespace lap
{
namespace log
{
    /**
     * @brief DLT (Diagnostic Log and Trace) sink
     * 
     * Encapsulates all DLT API operations including:
     * - DLT application registration
     * - DLT context management
     * - Log level and trace status configuration
     * - DLT log output
     */
    class DLTSink : public ISink
    {
    public:
        IMP_OPERATOR_NEW(DLTSink)
        
        /**
         * @brief DLT configuration structure
         */
        struct DLTConfig {
            core::String    appId;              ///< Application ID (max 4 chars)
            core::String    appDesc;            ///< Application description
            core::String    contextId;          ///< Default context ID (max 4 chars)
            core::String    contextDesc;        ///< Default context description
            LogLevel        defaultLogLevel;    ///< Default log level
            TraceStatus     traceStatus;        ///< Trace status
            core::Int8      withSessionId;      ///< Include session ID
            core::Int8      withTimestamp;      ///< Include timestamp
            core::Int8      withEcuId;          ///< Include ECU ID
            core::Bool      logMarker;          ///< Enable log marker
            core::Bool      verboseMode;        ///< Enable verbose mode
            
            DLTConfig() noexcept
                : appId("LAPP")
                , appDesc("LightAP Application")
                , contextId("DCTX")
                , contextDesc("Default Context")
                , defaultLogLevel(LogLevel::kWarn)
                , traceStatus(TraceStatus::kDefault)
                , withSessionId(1)
                , withTimestamp(1)
                , withEcuId(1)
                , logMarker(false)
                , verboseMode(true)
            {}
        };
        
        /**
         * @brief Constructor
         * @param config DLT configuration
         * @param minLevel Minimum log level to output (default: Verbose)
         */
        explicit DLTSink(
            const DLTConfig& config,
            LogLevel minLevel = LogLevel::kVerbose
        ) noexcept;
        
        virtual ~DLTSink() noexcept override;
        
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
        virtual core::StringView getName() const noexcept override { return "DLT"; }
        virtual void setLevel(LogLevel level) noexcept override { m_minLevel = level; }
        virtual core::Bool shouldLog(LogLevel level) const noexcept override;
        
        /**
         * @brief Set minimum log level
         * @param level Minimum level to output
         */
        void setMinLevel(LogLevel level) noexcept { m_minLevel = level; }
        
        /**
         * @brief Enable or disable the sink
         * @param enabled true to enable, false to disable
         */
        void setEnabled(core::Bool enabled) noexcept { m_enabled = enabled; }
        
        /**
         * @brief Register a DLT context
         * @param contextId Context ID (max 4 chars)
         * @param contextDesc Context description
         * @param level Log level for this context
         * @param status Trace status for this context
         * @return true if registered successfully
         */
        core::Bool registerContext(
            core::StringView contextId, 
            core::StringView contextDesc,
            LogLevel level = LogLevel::kWarn,
            TraceStatus status = TraceStatus::kDefault
        ) noexcept;
        
    private:
        core::Bool          m_enabled;
        LogLevel            m_minLevel;
        DltContext          m_defaultContext;  // Default DLT context
        core::Bool          m_dltInitialized;
        core::Bool          m_appRegistered;   // DLT app registration status
        core::String        m_appId;           // Stored app ID
        
        // Helper to get or create DLT context (simplified: using default context for now)
        DltContext* getContext(core::StringView contextId) noexcept;
        
        /**
         * @brief Convert internal LogLevelType to DltLogLevelType
         * @param level Internal log level
         * @return DLT log level
         */
        static constexpr DltLogLevelType toDltLevel(LogLevelType level) noexcept {
            return static_cast<DltLogLevelType>(level);
        }
        
        /**
         * @brief Convert LogLevel to DltLogLevelType
         */
        static constexpr DltLogLevelType toDltLevel(LogLevel level) noexcept {
            return static_cast<DltLogLevelType>(static_cast<core::UInt8>(level));
        }
        
        /**
         * @brief Convert TraceStatus to DltTraceStatusType
         */
        static constexpr DltTraceStatusType toDltTraceStatus(TraceStatus status) noexcept {
            return static_cast<DltTraceStatusType>(static_cast<core::Int8>(status));
        }
    };

} // namespace log
} // namespace lap

#endif // LAP_LOG_DLTSINK_HPP

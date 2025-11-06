/**
 * @file        LogManager.hpp
 * @author      ddkv587 ( ddkv587@gmail.com )
 * @brief       LogManager class for AUTOSAR Adaptive Platform logging
 * @date        2025-10-27
 * @details     Provides centralized logging management and configuration
 * @copyright   Copyright (c) 2025
 * @note
 * sdk:
 * platform:
 * project:     LightAP
 * @version
 * <table>
 * <tr><th>Date        <th>Version  <th>Author          <th>Description
 * <tr><td>2024/02/02  <td>1.0      <td>ddkv587         <td>init version
 * <tr><td>2025/10/27  <td>1.1      <td>ddkv587         <td>update header format
 * </table>
 */

#ifndef LAP_LOG_LOGMANAGER_HPP
#define LAP_LOG_LOGMANAGER_HPP

#include <lap/core/CSync.hpp>
#include <lap/core/CConfig.hpp>
#include <lap/core/CTypedef.hpp>
#include "CCommon.hpp"
#include "CLogger.hpp"
#include "CSinkManager.hpp"
#include <lap/core/CInstanceSpecifier.hpp>
#include <nlohmann/json.hpp>

namespace lap
{
namespace log
{
    class LogManager final
    {
    public:
    private:
        #define DEF_LOG_CONFIG_INDICATE     "logConfig"

        using _LogContextMap = core::UnorderedMap< core::String, core::UniqueHandle< Logger > >;

        struct tagLogConfig
        {
            core::String             strApplicationId;
            core::String             strApplicationDescription;
            core::String             strDefaultContextId;
            core::String             strDefaultContextDescription;
            core::String             strLogTraceFilePath;

            LogLevel                 logTraceDefaultLogLevel;
            LogMode                  logTraceLogMode;
            TraceStatus              logTraceStatus;

            core::Int8               iWithSessionId;
            core::Int8               iWithTimeStamp;
            core::Int8               iWithEcuId;
            core::Bool               isLogMarker;
            core::Bool               isVerboseMode;
            
            // FileSink rotation configuration
            core::Size               logFileMaxSize;        // Max file size in bytes (default: 10MB)
            core::UInt32             logFileMaxBackups;     // Max backup files (default: 5)
        };

    public:
        IMP_OPERATOR_NEW(LogManager)

        static LogManager& getInstance() noexcept
        {
            static LogManager instance;

            return instance;
        }

        /** @fn         bool initialize() noexcept;
         *  @brief      Default initialize function
         *  @details    Call this function when app start, it will load default config file "config.json"
         *  @param[in]  none
         *  @return     core::Bool                  Initialize success or failed. When failed, call any other function will assert
         */
        core::Bool                          initialize() noexcept;

        /** @fn         bool initialize( core::InstanceSpecifier &strConfigFile ) noexcept;
         *  @brief      initialize function with specified config
         *  @details    Call this function when app start, it will load specified config file
         *  @param[in]  core::InstanceSpecifier     specified config file 
         *  @return     core::Bool                  Initialize success or failed. When failed, call any other function will assert
         */
        core::Bool                          initialize( const core::InstanceSpecifier &strConfigFile ) noexcept;
        void                                uninitialize() noexcept;
        inline core::Bool                   isInitialized() const noexcept                      { return m_bInitialized; }

        Logger&                             registerLogger( core::StringView ctxID, 
                                                                core::StringView ctxDesc,
                                                                LogLevel level = LogLevel::kFatal,
                                                                TraceStatus status = TraceStatus::kDefault ) noexcept;

        Logger&                             logger( core::StringView ctxID = "" ) noexcept;

        inline core::Bool                   isConsoleEnabled() const noexcept                           { return m_bInitialized && static_cast<bool>(static_cast<core::UInt8>(m_logConfig.logTraceLogMode) & static_cast<core::UInt8>(LogMode::kConsole)); }

        // Sink management
        inline SinkManager&                 getSinkManager() noexcept                                   { return m_sinkManager; }
        inline const SinkManager&           getSinkManager() const noexcept                             { return m_sinkManager; }

        // inline void                         setDefaultLogLevel( LogLevel level ) noexcept       { m_logConfig.logTraceDefaultLogLevel = level; }
        // inline LogLevel                     defaultLogLevel() noexcept                          { return m_logConfig.logTraceDefaultLogLevel; }

    protected:
        LogManager() noexcept;
        ~LogManager() noexcept;

        //inline tagLogConfig&              getLogConfig() noexcept                             { return m_logConfig; }

    private:
        void                                resetLogConfig() noexcept;
        core::Bool                          initWithLogConfig() noexcept;
        void                                initializeSinks() noexcept;
        // Load and parse logging config from Core::ConfigManager (module: "logConfig")
        core::Bool                          loadFromCoreConfig() noexcept;
        // Save current log config to Core::ConfigManager
        void                                saveToCoreConfig() noexcept;
        void                                createSinkFromConfig(const nlohmann::json& sinkConfig) noexcept;

        core::StringView                    formatId( core::StringView strId ) const noexcept;
        LogLevel                            formatLevel( core::StringView strLevel ) const noexcept;

    private:
        static LogManager                  *s_pInstance;

        core::Bool                          m_bInitialized{ false };
        tagLogConfig                        m_logConfig;
        
        // Store sink configurations from JSON for later initialization
        core::Vector<nlohmann::json>        m_sinkConfigs;

        core::Mutex                         m_mtxContextMap;
        _LogContextMap                      m_mapLogContext;

        core::UniqueHandle< Logger >        m_defaultLogCtx{ nullptr };
        
        SinkManager                         m_sinkManager;      // Sink manager for Console/File/Syslog outputs
    };
} // namespace log
} // namespace lap
#endif
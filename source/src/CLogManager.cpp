#include <syslog.h>
#include <nlohmann/json.hpp>
#include <lap/core/CConfig.hpp>
#include "CLogManager.hpp"
#include "CConsoleSink.hpp"
#include "CFileSink.hpp"
#include "CSyslogSink.hpp"
#include "CDLTSink.hpp"

namespace lap
{
namespace log
{
    const char* DEFAULT_LOG_CONFIG = "log";

    LogManager::LogManager() noexcept
    {
        resetLogConfig();

        initialize();
    }

    LogManager::~LogManager() noexcept
    {
        if ( m_bInitialized ) {
            // Save current configuration before uninitialize
            saveToCoreConfig();

            uninitialize();
        }
    }

    core::Bool LogManager::initialize() noexcept
    {
        // pre init set
        if ( m_bInitialized )   return m_bInitialized;

        // Load logging configuration from Core Config
        // ConfigManager should already be initialized by the application
        if ( !loadFromCoreConfig() ) {
            resetLogConfig();
        }

        m_bInitialized = initWithLogConfig();

        return m_bInitialized;
    }

    core::Bool LogManager::initialize( const lap::core::InstanceSpecifier &strConfigFile ) noexcept
    {
        if ( m_bInitialized )   return m_bInitialized;

        // For model specifier we rely on Core ConfigManager
        // ConfigManager should already be initialized by the application
        (void)strConfigFile; // parameter intentionally unused in Core Config flow
        
        if ( !loadFromCoreConfig() ) {
            resetLogConfig();
        }

        m_bInitialized = initWithLogConfig();

        return m_bInitialized;
    }

    void LogManager::uninitialize() noexcept
    {
        if ( !m_bInitialized )  return;

        core::LockGuard lock( m_mtxContextMap );
        m_mapLogContext.clear();

        // unregister default context
        m_defaultLogCtx.release();

        // DLT unregistration is now handled by CDLTSink destructor

        m_bInitialized = false;
    }

    Logger& LogManager::registerLogger( lap::core::StringView ctxID, lap::core::StringView ctxDesc, LogLevel level, TraceStatus status ) noexcept
    {
        // Make sure the initialization function has been called
        assert( m_bInitialized && "Make sure the initialization function has been called!!!"  );

        if ( ctxID.empty() ) {
            return *m_defaultLogCtx;
        }

        auto&& it = m_mapLogContext.find( { ctxID.data() } );

        if ( it != m_mapLogContext.end() ) {
            // already exist
            return *( it->second );
        } else {
            core::LockGuard lock( m_mtxContextMap );
            // create new logger
            auto&& _it = m_mapLogContext.emplace( ctxID, core::MakeUnique< Logger >( ctxID, ctxDesc, level, status ) );

            // return default logger
            return *( _it.first->second );
        }
    }

    Logger& LogManager::logger( lap::core::StringView ctxID ) noexcept
    {
        // Make sure the initialization function has been called
        assert( m_bInitialized && "Make sure the initialization function has been called!!!" );

        if ( ctxID.empty() ) {
            return *m_defaultLogCtx;
        }

        auto&& it = m_mapLogContext.find( { ctxID.data() } );

        if ( it != m_mapLogContext.end() ) {
            // already exist
            return *( it->second );
        } else {
            // return default logger
            return *m_defaultLogCtx;
        }
    }

    void LogManager::resetLogConfig() noexcept
    {
       // set default log config
        m_logConfig.strApplicationId                = "DEFT";
        m_logConfig.strApplicationDescription       = "Default App";
        m_logConfig.strDefaultContextId             = "DEFT";
        m_logConfig.strDefaultContextDescription    = "Default Context";
        m_logConfig.strLogTraceFilePath             = "";
        m_logConfig.logTraceDefaultLogLevel         = LogLevel::kWarn;
        m_logConfig.logTraceLogMode                 = LogMode::kConsole;  // Default to Console only
        m_logConfig.logTraceStatus                  = TraceStatus::kDefault;

        m_logConfig.iWithSessionId                  = 1;
        m_logConfig.iWithTimeStamp                  = 1;
        m_logConfig.iWithEcuId                      = 1;
        m_logConfig.isLogMarker                     = false;
        m_logConfig.isVerboseMode                   = true;
        
        // FileSink rotation defaults
        m_logConfig.logFileMaxSize                  = 10 * 1024 * 1024;  // 10MB
        m_logConfig.logFileMaxBackups               = 5;                 // 5 backup files
    }

    core::Bool LogManager::loadFromCoreConfig() noexcept
    {
        try {
            auto& cfgMgr = core::ConfigManager::getInstance();
            nlohmann::json logObj = cfgMgr.getModuleConfigJson(DEFAULT_LOG_CONFIG);
            if (!logObj.is_object()) {
                auto all = nlohmann::json::parse(cfgMgr.toJson(false), nullptr, false);
                if (all.is_object() && all.contains(DEFAULT_LOG_CONFIG) && all[DEFAULT_LOG_CONFIG].is_object()) {
                    logObj = all[DEFAULT_LOG_CONFIG];
                } else {
                    return true; // keep defaults
                }
            }

            auto getStr = [&]( const char* key, ::std::string& out ) {
                if (logObj.contains(key) && logObj[key].is_string()) { out = logObj[key].get< ::std::string >(); return true; } return false; };
            auto getInt = [&]( const char* key, ::std::int64_t& out ) {
                if (logObj.contains(key) && logObj[key].is_number_integer()) { out = logObj[key].get< ::std::int64_t >(); return true; } return false; };
            auto getUInt = [&]( const char* key, ::std::uint64_t& out ) {
                if (logObj.contains(key) && logObj[key].is_number_unsigned()) { out = logObj[key].get< ::std::uint64_t >(); return true; } return false; };
            auto getBool = [&]( const char* key, bool& out ) {
                if (logObj.contains(key) && logObj[key].is_boolean()) { out = logObj[key].get< bool >(); return true; } return false; };

            ::std::string s;
            if ( getStr( "applicationId", s ) ) {
                m_logConfig.strApplicationId = core::String{ formatId( core::StringView{ s.c_str() } ) };
            }
            if ( getStr( "applicationDescription", s ) ) {
                m_logConfig.strApplicationDescription = core::String{ s.c_str() };
            }
            if ( getStr( "contextId", s ) ) {
                m_logConfig.strDefaultContextId = core::String{ formatId( core::StringView{ s.c_str() } ) };
            }
            if ( getStr( "contextDescription", s ) ) {
                m_logConfig.strDefaultContextDescription = core::String{ s.c_str() };
            }
            if ( getStr( "logTraceDefaultLogLevel", s ) ) {
                m_logConfig.logTraceDefaultLogLevel = formatLevel( core::StringView{ s.c_str() } );
            }
            if ( getStr( "logTraceFilePath", s ) ) {
                m_logConfig.strLogTraceFilePath = core::String{ s.c_str() };
            }

            if (logObj.contains("logTraceLogMode") && logObj["logTraceLogMode"].is_array()) {
                m_logConfig.logTraceLogMode = LogMode::kOff;
                for (const auto& vj : logObj["logTraceLogMode"]) {
                    if (!vj.is_string()) continue;
                    auto v = vj.get< ::std::string >();
                    if ( v == "console" ) m_logConfig.logTraceLogMode |= LogMode::kConsole;
                    else if ( v == "file" ) m_logConfig.logTraceLogMode |= LogMode::kFile;
                    else if ( v == "dlt" ) m_logConfig.logTraceLogMode |= LogMode::kDlt;
                    else if ( v == "syslog" ) m_logConfig.logTraceLogMode |= LogMode::kSyslog;
                    else {
                        fprintf( stderr, "[LightAP] LogManager: Unknown log mode '%s' in config, ignored.\n", v.c_str() );
                    }
                }
            }

            ::std::int64_t iv = 0;
            ::std::uint64_t uv = 0;
            if ( getInt( "withSessionId", iv ) ) m_logConfig.iWithSessionId = static_cast<core::Int8>( iv );
            if ( getInt( "withTimeStamp", iv ) ) m_logConfig.iWithTimeStamp = static_cast<core::Int8>( iv );
            if ( getInt( "withEcuId", iv ) ) m_logConfig.iWithEcuId = static_cast<core::Int8>( iv );

            bool bv = false;
            if ( getBool( "logMarker", bv ) ) m_logConfig.isLogMarker = bv;
            if ( getBool( "verboseMode", bv ) ) m_logConfig.isVerboseMode = bv;

            if ( getUInt( "logFileMaxSize", uv ) && uv > 0 ) {
                m_logConfig.logFileMaxSize = static_cast<core::Size>( uv );
            }
            if ( getUInt( "logFileMaxBackups", uv ) && uv > 0 ) {
                m_logConfig.logFileMaxBackups = static_cast<core::UInt32>( uv );
            }

            if (logObj.contains("sinks") && logObj["sinks"].is_array()) {
                m_sinkConfigs.clear();
                for (const auto& sj : logObj["sinks"]) {
                    if (sj.is_object()) m_sinkConfigs.push_back(sj);
                }
            }

            return true;
        } catch (const ::std::exception& e) {
            fprintf(stderr, "[LightAP] LogManager: loadFromCoreConfig failed: %s\n", e.what());
            return false;
        }
    }

    void LogManager::saveToCoreConfig() noexcept
    {
        try {
            auto& cfgMgr = core::ConfigManager::getInstance();
            
            // Build JSON configuration object
            nlohmann::json logObj;
            
            // Save basic log config
            logObj["applicationId"] = std::string(m_logConfig.strApplicationId);
            logObj["applicationDescription"] = std::string(m_logConfig.strApplicationDescription);
            logObj["contextId"] = std::string(m_logConfig.strDefaultContextId);
            logObj["contextDescription"] = std::string(m_logConfig.strDefaultContextDescription);
            logObj["logTraceFilePath"] = std::string(m_logConfig.strLogTraceFilePath);
            
            // Save log level
            const char* levelStr = "WARN";
            switch (m_logConfig.logTraceDefaultLogLevel) {
                case LogLevel::kOff:     levelStr = "OFF"; break;
                case LogLevel::kFatal:   levelStr = "FATAL"; break;
                case LogLevel::kError:   levelStr = "ERROR"; break;
                case LogLevel::kWarn:    levelStr = "WARN"; break;
                case LogLevel::kInfo:    levelStr = "INFO"; break;
                case LogLevel::kDebug:   levelStr = "DEBUG"; break;
                case LogLevel::kVerbose: levelStr = "VERBOSE"; break;
                case LogLevel::kLogLevelMax: levelStr = "WARN"; break;  // Default fallback
            }
            logObj["logTraceDefaultLogLevel"] = levelStr;
            
            // Save log modes
            nlohmann::json modesArray = nlohmann::json::array();
            auto logMode = m_logConfig.logTraceLogMode;
            if (static_cast<bool>(static_cast<core::UInt8>(logMode) & static_cast<core::UInt8>(LogMode::kConsole))) {
                modesArray.push_back("console");
            }
            if (static_cast<bool>(static_cast<core::UInt8>(logMode) & static_cast<core::UInt8>(LogMode::kFile))) {
                modesArray.push_back("file");
            }
            if (static_cast<bool>(static_cast<core::UInt8>(logMode) & static_cast<core::UInt8>(LogMode::kDlt))) {
                modesArray.push_back("dlt");
            }
            if (static_cast<bool>(static_cast<core::UInt8>(logMode) & static_cast<core::UInt8>(LogMode::kSyslog))) {
                modesArray.push_back("syslog");
            }
            logObj["logTraceLogMode"] = modesArray;
            
            // Save DLT-related options
            logObj["withSessionId"] = static_cast<int>(m_logConfig.iWithSessionId);
            logObj["withTimeStamp"] = static_cast<int>(m_logConfig.iWithTimeStamp);
            logObj["withEcuId"] = static_cast<int>(m_logConfig.iWithEcuId);
            logObj["logMarker"] = m_logConfig.isLogMarker;
            logObj["verboseMode"] = m_logConfig.isVerboseMode;
            
            // Save file rotation config
            logObj["logFileMaxSize"] = m_logConfig.logFileMaxSize;
            logObj["logFileMaxBackups"] = m_logConfig.logFileMaxBackups;
            
            // Save sink configurations if any
            if (!m_sinkConfigs.empty()) {
                logObj["sinks"] = m_sinkConfigs;
            }
            
            // Write to ConfigManager
            cfgMgr.setModuleConfigJson(DEFAULT_LOG_CONFIG, logObj);
            
        } catch (const ::std::exception& e) {
            fprintf(stderr, "[LightAP] LogManager: saveToCoreConfig failed: %s\n", e.what());
        }
    }

    core::Bool LogManager::initWithLogConfig() noexcept
    {
        // DLT initialization is now handled by CDLTSink
        
        // register default logger
        m_defaultLogCtx = core::MakeUnique< Logger >( 
            core::StringView( m_logConfig.strDefaultContextId ), 
            core::StringView( m_logConfig.strDefaultContextDescription ), 
            m_logConfig.logTraceDefaultLogLevel 
        );

        assert( m_defaultLogCtx != nullptr && "The default log context creation failed!!!" );

        // Initialize SinkManager based on log mode configuration
        initializeSinks();

        return true;
    }

    void LogManager::initializeSinks() noexcept
    {
        // Get default log level as fallback
        auto defaultMinLevel = m_logConfig.logTraceDefaultLogLevel;
        
        // Set global minimum level for SinkManager
        m_sinkManager.setGlobalMinLevel(defaultMinLevel);
        
        // Check if we have sink configurations from JSON
        if (!m_sinkConfigs.empty()) {
            // Use sink configurations from JSON "sinks" array
            for (const auto& sinkConfig : m_sinkConfigs) {
                createSinkFromConfig(sinkConfig);
            }
        } else {
            // Fallback to legacy logTraceLogMode configuration
            auto logMode = m_logConfig.logTraceLogMode;
            
            // Add Console sink if enabled
            if (static_cast<bool>(static_cast<core::UInt8>(logMode) & static_cast<core::UInt8>(LogMode::kConsole))) {
                auto consoleSink = core::MakeUnique<ConsoleSink>(true, defaultMinLevel);
                m_sinkManager.addSink(core::Move(consoleSink));
            }
            
            // Add File sink if enabled
            if (static_cast<bool>(static_cast<core::UInt8>(logMode) & static_cast<core::UInt8>(LogMode::kFile))) {
                if (!m_logConfig.strLogTraceFilePath.empty()) {
                    auto fileSink = core::MakeUnique<FileSink>(
                        core::StringView(m_logConfig.strLogTraceFilePath),
                        m_logConfig.logFileMaxSize,
                        m_logConfig.logFileMaxBackups,
                        defaultMinLevel,
                        core::StringView(m_logConfig.strApplicationId)
                    );
                    m_sinkManager.addSink(core::Move(fileSink));
                }
            }
            
            // Add Syslog sink if enabled
            if (static_cast<bool>(static_cast<core::UInt8>(logMode) & static_cast<core::UInt8>(LogMode::kSyslog))) {
                auto syslogSink = core::MakeUnique<SyslogSink>("LightAP", LOG_USER, defaultMinLevel);
                m_sinkManager.addSink(core::Move(syslogSink));
            }
            
            // Add DLT sink if enabled
            if (static_cast<bool>(static_cast<core::UInt8>(logMode) & static_cast<core::UInt8>(LogMode::kDlt))) {
                DLTSink::DLTConfig dltConfig;
                dltConfig.appId = m_logConfig.strApplicationId;
                dltConfig.appDesc = m_logConfig.strApplicationDescription;
                dltConfig.contextId = m_logConfig.strDefaultContextId;
                dltConfig.contextDesc = m_logConfig.strDefaultContextDescription;
                dltConfig.defaultLogLevel = m_logConfig.logTraceDefaultLogLevel;
                dltConfig.traceStatus = m_logConfig.logTraceStatus;
                dltConfig.withSessionId = m_logConfig.iWithSessionId;
                dltConfig.withTimestamp = m_logConfig.iWithTimeStamp;
                dltConfig.withEcuId = m_logConfig.iWithEcuId;
                dltConfig.logMarker = m_logConfig.isLogMarker;
                dltConfig.verboseMode = m_logConfig.isVerboseMode;
                
                auto dltSink = core::MakeUnique<DLTSink>(dltConfig, defaultMinLevel);
                m_sinkManager.addSink(core::Move(dltSink));
            }
        }
    }

    void LogManager::createSinkFromConfig(const nlohmann::json& sinkConfig) noexcept
    {
        try {
            if (!sinkConfig.contains("type") || !sinkConfig["type"].is_string()) {
                fprintf(stderr, "[LightAP] LogManager: Sink missing 'type' field, skipped\n");
                return;
            }
            std::string type = sinkConfig["type"].get<std::string>();

            // Parse level (optional, use default if not specified)
            LogLevel sinkLevel = m_logConfig.logTraceDefaultLogLevel;
            if (sinkConfig.contains("level") && sinkConfig["level"].is_string()) {
                auto lv = sinkConfig["level"].get<std::string>();
                sinkLevel = formatLevel(core::StringView(lv.c_str()));
            }
            
            if (type == "file") {
                // File sink configuration
                if (!sinkConfig.contains("path") || !sinkConfig["path"].is_string() || sinkConfig["path"].get<std::string>().empty()) {
                    fprintf(stderr, "[LightAP] LogManager: File sink missing 'path', skipped\n");
                    return;
                }
                auto pathStr = sinkConfig["path"].get<std::string>();
                size_t maxSize = sinkConfig.contains("maxSize") && sinkConfig["maxSize"].is_number_unsigned() ? sinkConfig["maxSize"].get<size_t>() : m_logConfig.logFileMaxSize;
                core::UInt32 backupCount = sinkConfig.contains("backupCount") && sinkConfig["backupCount"].is_number_unsigned() ? sinkConfig["backupCount"].get<core::UInt32>() : m_logConfig.logFileMaxBackups;
                
                auto fileSink = core::MakeUnique<FileSink>(
                    core::StringView(pathStr.c_str()),
                    maxSize,
                    backupCount,
                    sinkLevel,
                    core::StringView(m_logConfig.strApplicationId)
                );
                m_sinkManager.addSink(core::Move(fileSink));
                
            } else if (type == "console") {
                // Console sink configuration
                bool colorized = sinkConfig.contains("colorized") && sinkConfig["colorized"].is_boolean() ? sinkConfig["colorized"].get<bool>() : true;
                auto consoleSink = core::MakeUnique<ConsoleSink>(colorized, sinkLevel);
                m_sinkManager.addSink(core::Move(consoleSink));
                
            } else if (type == "syslog") {
                // Syslog sink configuration
                // Use applicationId from logConfig as ident
                std::string ident = m_logConfig.strApplicationId;
                int facility = sinkConfig.contains("facility") && sinkConfig["facility"].is_number_integer() ? sinkConfig["facility"].get<int>() : LOG_USER;
                auto syslogSink = core::MakeUnique<SyslogSink>(ident, facility, sinkLevel);
                m_sinkManager.addSink(core::Move(syslogSink));
                
            } else if (type == "dlt") {
                // DLT sink configuration - inherit from logConfig
                DLTSink::DLTConfig dltConfig;
                
                // Use logConfig values (no override from sink config)
                dltConfig.appId = m_logConfig.strApplicationId;
                dltConfig.appDesc = m_logConfig.strApplicationDescription;
                dltConfig.contextId = m_logConfig.strDefaultContextId;
                dltConfig.contextDesc = m_logConfig.strDefaultContextDescription;
                dltConfig.defaultLogLevel = sinkLevel;
                dltConfig.traceStatus = m_logConfig.logTraceStatus;
                dltConfig.withSessionId = m_logConfig.iWithSessionId;
                dltConfig.withTimestamp = m_logConfig.iWithTimeStamp;
                dltConfig.withEcuId = m_logConfig.iWithEcuId;
                dltConfig.logMarker = m_logConfig.isLogMarker;
                dltConfig.verboseMode = m_logConfig.isVerboseMode;
                
                auto dltSink = core::MakeUnique<DLTSink>(dltConfig, sinkLevel);
                m_sinkManager.addSink(core::Move(dltSink));
                
            } else {
                fprintf(stderr, "[LightAP] LogManager: Unknown sink type '%s', skipped\n", type.c_str());
            }
            
        } catch (const std::exception& e) {
            fprintf(stderr, "[LightAP] LogManager: Error creating sink: %s\n", e.what());
        }
    }

    core::StringView LogManager::formatId( core::StringView strId ) const noexcept
    {
        if ( strId.empty() )        return "XXXX";

        if ( strId.size() <= 4 )    return strId;

        return core::StringView{ strId.data(), 4 };
    }

    LogLevel LogManager::formatLevel( core::StringView strLevel ) const noexcept
    {
        if ( strLevel == "Off" || strLevel == "OFF" ) { 
            return LogLevel::kOff;
        } else if ( strLevel == "Fatal" || strLevel == "FATAL" ) { 
            return LogLevel::kFatal;
        } else if ( strLevel == "Error" || strLevel == "ERROR" ) { 
            return LogLevel::kError;
        } else if ( strLevel == "Warn" || strLevel == "WARN" ) { 
            return LogLevel::kWarn;
        } else if ( strLevel == "Info" || strLevel == "INFO" ) { 
            return LogLevel::kInfo;
        } else if ( strLevel == "Debug" || strLevel == "DEBUG" ) { 
            return LogLevel::kDebug;
        } else if ( strLevel == "Verbose" || strLevel == "VERBOSE" ) { 
            return LogLevel::kVerbose;
        } else { 
            return LogLevel::kFatal;
        }
    }
} // namespace log
} // namespace lap

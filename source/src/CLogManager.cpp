#include <dlt/dlt.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <fstream>
#include "CPath.hpp"
#include "CLogManager.hpp"

#define DEFAULT_CONFIG_FILE     "config.json"
namespace lap
{
namespace log
{
    CLogManager::CLogManager() noexcept
    {
        resetLogConfig();
    }

    CLogManager::~CLogManager() noexcept
    {
        if ( m_bInitialized ) {
            uninitialize();
        }
    }

    core::Bool CLogManager::initialize() noexcept
    {
        // pre init set
        if ( m_bInitialized )   return m_bInitialized;

        INNER_LOG_DEBUG( "CLogManager::initialize...\n" );

        core::StringView strConfigFile = core::Path::append( core::Path::getApplicationFolder(), DEFAULT_CONFIG_FILE );

        std::ifstream ifs( strConfigFile.data() );
        if ( ifs.good() ) {
            // try to use default config
            if ( !parseLogConfig( strConfigFile ) ) {
                // reset log config if parse failed
                resetLogConfig();
            }
        }

        m_bInitialized = initWithLogConfig();

        return m_bInitialized;
    }

    core::Bool CLogManager::initialize( const lap::core::InstanceSpecifier &strConfigFile ) noexcept
    {
        if ( m_bInitialized )   return m_bInitialized;

        INNER_LOG_DEBUG( "CLogManager::initialize with specified file %s...\n", strConfigFile.ToString().data() );

        // parse from specified config file
        if ( !parseLogConfig( strConfigFile.ToString() ) ) {
            resetLogConfig();
        }

        m_bInitialized = initWithLogConfig();

        return m_bInitialized;
    }

    void CLogManager::uninitialize() noexcept
    {
        if ( !m_bInitialized )  return;

        INNER_LOG_DEBUG( "CLogManager::uninitialize...\n" );

        ::std::lock_guard< ::std::mutex > lock( m_mtxContextMap );
        m_mapLogContext.clear();

        // unregister default context
        m_defaultLogCtx.release();

        // unregister app
        DLT_UNREGISTER_APP();

        m_bInitialized = false;
    }

    Logger& CLogManager::registerLogger( lap::core::StringView ctxID, lap::core::StringView ctxDesc, LogLevel level, TraceStatus status ) noexcept
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
            ::std::lock_guard< ::std::mutex > lock( m_mtxContextMap );
            // create new logger
            auto&& _it = m_mapLogContext.emplace( ctxID, ::std::make_unique< Logger >( ctxID, ctxDesc, level, status ) );

            // return default logger
            return *( _it.first->second );
        }
    }

    Logger& CLogManager::logger( lap::core::StringView ctxID ) noexcept
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

    void CLogManager::resetLogConfig() noexcept
    {
       // set default log config
        m_logConfig.strApplicationId                = "DEFT";
        m_logConfig.strApplicationDescription       = "Default App";
        m_logConfig.strDefaultContextId             = "DEFT";
        m_logConfig.strDefaultContextDescription    = "Default Context";
        m_logConfig.strLogTraceFilePath             = "";
        m_logConfig.logTraceDefaultLogLevel         = LogLevel::kFatal;
        m_logConfig.logTraceLogMode                 = LogMode::kRemote;
        m_logConfig.logTraceStatus                  = TraceStatus::kDefault;

        m_logConfig.iWithSessionId                  = 1;
        m_logConfig.iWithTimeStamp                  = 1;
        m_logConfig.iWithEcuId                      = 1;
        m_logConfig.isLogMarker                     = false;
        m_logConfig.isVerboseMode                   = true;
    }

    core::Bool CLogManager::parseLogConfig( core::StringView strConfigFile ) noexcept
    {
        std::ifstream ifs{ strConfigFile.data() };
        if ( !ifs.good() ) {
            INNER_LOG_WARNING( "CLogManager::parseLogConfig cannot open %s\n", strConfigFile.data() );
            return false;
        }

        boost::property_tree::ptree root;
        try {
            ifs.clear();
            ifs.seekg( 0 );
            boost::property_tree::read_json( ifs, root );
        } catch ( const ::std::exception& e ) {
            INNER_LOG_WARNING( "CLogManager::parseLogConfig %s failed: %s\n", strConfigFile.data(), e.what() );
            return false;
        }

        auto logOpt = root.get_child_optional( "logConfig" );
        if ( !logOpt ) {
            return true; // keep defaults
        }
        const auto& logObj = *logOpt;

        auto getStr = [&]( const char* key, ::std::string& out ) {
            auto opt = logObj.get_optional< ::std::string >( key );
            if ( opt ) { out = *opt; return true; } return false; };
        auto getInt = [&]( const char* key, ::std::int64_t& out ) {
            auto opt = logObj.get_optional< ::std::int64_t >( key );
            if ( opt ) { out = *opt; return true; } return false; };
        auto getBool = [&]( const char* key, bool& out ) {
            auto opt = logObj.get_optional< bool >( key );
            if ( opt ) { out = *opt; return true; } return false; };

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

        auto modesOpt = logObj.get_child_optional( "logTraceLogMode" );
        if ( modesOpt ) {
            m_logConfig.logTraceLogMode = LogMode::kOff;
            for ( const auto& child : *modesOpt ) {
                const auto& v = child.second.get_value< ::std::string >();
                if ( v == "console" ) m_logConfig.logTraceLogMode |= LogMode::kConsole;
                else if ( v == "file" ) m_logConfig.logTraceLogMode |= LogMode::kFile;
                else if ( v == "network" ) m_logConfig.logTraceLogMode |= LogMode::kRemote;
            }
        }

        ::std::int64_t iv = 0;
        if ( getInt( "withSessionId", iv ) ) m_logConfig.iWithSessionId = static_cast<core::Int8>( iv );
        if ( getInt( "withTimeStamp", iv ) ) m_logConfig.iWithTimeStamp = static_cast<core::Int8>( iv );
        if ( getInt( "withEcuId", iv ) ) m_logConfig.iWithEcuId = static_cast<core::Int8>( iv );

        bool bv = false;
        if ( getBool( "logMarker", bv ) ) m_logConfig.isLogMarker = bv;
        if ( getBool( "verboseMode", bv ) ) m_logConfig.isVerboseMode = bv;

        return true;
    }

    core::Bool CLogManager::initWithLogConfig() noexcept
    {
        INNER_LOG_DEBUG( "apid: %s, ctid: %s, level: %d\n", \
                            m_logConfig.strApplicationId.c_str(), \
                            m_logConfig.strDefaultContextId.c_str(), \
                            static_cast< core::UInt8 >( m_logConfig.logTraceDefaultLogLevel ) );

        dlt_with_session_id( m_logConfig.iWithSessionId );
        dlt_with_timestamp( m_logConfig.iWithTimeStamp );
        dlt_with_ecu_id( m_logConfig.iWithEcuId );
        if ( m_logConfig.isLogMarker )      DLT_LOG_MARKER();
        if ( m_logConfig.isVerboseMode )    DLT_VERBOSE_MODE();

        // register app
        auto ret = dlt_check_library_version(_DLT_PACKAGE_MAJOR_VERSION, _DLT_PACKAGE_MINOR_VERSION);
        if ( DLT_RETURN_OK != ret ) {
            INNER_LOG_WARNING( "CLogManager::initWithLogConfig dlt_check_library_version error %d\n", static_cast< core::Int32 >( ret ) );
            // continue without hard failing; DLT operations may be no-ops
        }

        ret = dlt_register_app( m_logConfig.strApplicationId.c_str(), m_logConfig.strApplicationDescription.c_str() );
        if ( DLT_RETURN_OK != ret ) {
            INNER_LOG_WARNING( "CLogManager::initWithLogConfig dlt_register_app error %d\n", static_cast< core::Int32 >( ret ) );
            // continue; subsequent context registration may fail gracefully
        }

        // set app limit
        DLT_SET_APPLICATION_LL_TS_LIMIT( convert( m_logConfig.logTraceDefaultLogLevel ),
                                            convert( m_logConfig.logTraceStatus ) );

        // register default logger
        m_defaultLogCtx = ::std::make_unique< Logger >( core::StringView( m_logConfig.strDefaultContextId ), core::StringView( m_logConfig.strDefaultContextDescription ), m_logConfig.logTraceDefaultLogLevel );

        assert( m_defaultLogCtx != nullptr && "The default log context creation failed!!!" );

    return true;
    }

    core::StringView CLogManager::formatId( core::StringView strId ) const noexcept
    {
        if ( strId.empty() )        return "XXXX";

        if ( strId.size() <= 4 )    return strId;

        return core::StringView{ strId.data(), 4 };
    }

    LogLevel CLogManager::formatLevel( core::StringView strLevel ) const noexcept
    {
        if ( strLevel == "Off" ) { 
            return LogLevel::kOff;
        } else if ( strLevel == "Fatal" ) { 
            return LogLevel::kFatal;
        } else if ( strLevel == "Warn" ) { 
            return LogLevel::kWarn;
        } else if ( strLevel == "Info" ) { 
            return LogLevel::kInfo;
        } else if ( strLevel == "Debug" ) { 
            return LogLevel::kDebug;
        } else if ( strLevel == "Verbose" ) { 
            return LogLevel::kVerbose;
        } else { 
            return LogLevel::kFatal;
        }
    }
} // namespace core
} // namespace lap
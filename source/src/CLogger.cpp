/*
 * @Author: Daokuan.deng
 * @Date: 2023-05-24 15:40:10
 * @LastEditors: Daokuan.deng
 * @LastEditTime: 2023-07-12 15:26:33
 * @Description: 
 */
#include "CLogger.hpp"
#include "CLogStream.hpp"
#include "CLogManager.hpp"

namespace lap
{
namespace log
{
    LogStream Logger::LogFatal() const noexcept
    {
        return { LogLevel::kFatal, *this };
    }

    LogStream Logger::LogError() const noexcept
    {
        return { LogLevel::kError, *this };
    }

    LogStream Logger::LogWarn() const noexcept
    {
        return { LogLevel::kWarn, *this };
    }

    LogStream Logger::LogInfo() const noexcept
    {
        return { LogLevel::kInfo, *this };
    }

    LogStream Logger::LogDebug() const noexcept
    {
        return { LogLevel::kDebug, *this };
    }

    LogStream Logger::LogVerbose() const noexcept
    {
        return { LogLevel::kVerbose, *this };
    }

    LogStream Logger::LogOff() const noexcept
    {
        return { LogLevel::kOff, *this };
    }

    bool Logger::IsEnabled( LogLevel logLevel ) const noexcept
    {
        // Simple level check - no DLT dependency
        return static_cast<core::UInt8>(logLevel) <= static_cast<core::UInt8>(m_logLevel);
    }

    LogStream Logger::WithLevel( LogLevel logLevel ) const noexcept
    {
        return { logLevel, *this };
    }

    Logger::Logger( core::StringView ctxId, core::StringView ctxDesc, LogLevel level, TraceStatus status ) noexcept
        : m_strContextID( ctxId.data() )
        , m_strContextDesc( ctxDesc.data() )
        , m_logLevel( level )
        , m_traceStatus( status )
    {
        // No DLT registration here - handled by CDLTSink
    }

    Logger::~Logger() noexcept
    {
        // No DLT unregistration needed
    }

    Logger& CreateLogger( lap::core::StringView ctxId, lap::core::StringView ctxDescription, LogLevel ctxDefLogLevel ) noexcept
    {
        return LogManager::getInstance().registerLogger( ctxId, ctxDescription, ctxDefLogLevel );
    }

    ClientState remoteClientState() noexcept
    {
        // DLT client state is no longer directly accessible
        // Return connected state by default
        return ClientState::kConnected;
    }

    // template <typename T>
    // Argument< T > Arg( T &&arg, const char *name=nullptr, const char *unit = nullptr ) noexcept
    // {
    //     ;
    // }

    // no-verbose
    template <typename MsgId, typename... Params>
    void Log( const MsgId &id, const Params &... args ) noexcept
    {
        ;
    }
} // namespace core
} // namespace lap
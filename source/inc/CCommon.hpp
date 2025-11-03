/**
 * @file        CCommon.hpp
 * @author      ddkv587 ( ddkv587@gmail.com )
 * @brief       Common definitions for AUTOSAR Adaptive Platform logging
 * @date        2025-10-27
 * @details     Provides common types, enums, and macros for logging functionality
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

#ifndef LAP_LOG_COMMON_HPP
#define LAP_LOG_COMMON_HPP

#include <core/CTypedef.hpp>
#include <core/CString.hpp>

namespace lap
{
namespace log
{
    // ANSI Color codes for console output
    constexpr const char* ANSI_RESET   = "\033[0m";
    constexpr const char* ANSI_RED     = "\033[31m";
    constexpr const char* ANSI_GREEN   = "\033[32m";
    constexpr const char* ANSI_YELLOW  = "\033[33m";
    constexpr const char* ANSI_BLUE    = "\033[34m";
    constexpr const char* ANSI_MAGENTA = "\033[35m";
    constexpr const char* ANSI_CYAN    = "\033[36m";
    constexpr const char* ANSI_WHITE   = "\033[37m";
    constexpr const char* ANSI_BOLD    = "\033[1m";
    constexpr const char* ANSI_DIM     = "\033[2m";

    // Internal log level type - independent of third-party libraries
    using LogLevelType = core::UInt8;

    enum class LogLevel : LogLevelType
    {
        kOff        = 0x00,     // No logging.
        kFatal      = 0x01,     // Fatal error, not recoverable.
        kError      = 0x02,     // Error with impact to correct functionality.
        kWarn       = 0x03,     // Warning if correct behavior cannot be ensured.
        kInfo       = 0x04,     // Informational, providing high level understanding.
        kDebug      = 0x05,     // Detailed information for programmers.
        kVerbose    = 0x06,     // Extra-verbose debug messages (highest grade of information)
        kLogLevelMax
    };

    constexpr inline core::StringView toString( const LogLevel& level ) noexcept
    {
        switch( level )
        {
        case LogLevel::kOff:            return "Off";
        case LogLevel::kFatal:          return "Fatal";
        case LogLevel::kError:          return "Error";
        case LogLevel::kWarn:           return "Warn";
        case LogLevel::kInfo:           return "Info";
        case LogLevel::kDebug:          return "Debug";
        case LogLevel::kVerbose:        return "Verbose";
        default:                        return "Unknow";
        }
    }

    enum class LogMode : core::UInt8
    {
        kOff        = 0x00,     // No log mode.
        kDlt        = 0x01,     // DLT (Diagnostic Log and Trace) - sent remotely.
        kFile       = 0x02,     // Save to file.
        kConsole    = 0x04,     // Forward to console.
        kSyslog     = 0x08,     // Send to system syslog.
        kLogModeMax
    };

    inline constexpr LogMode operator|( LogMode lhs, LogMode rhs ) noexcept
    {
        return static_cast< LogMode >( static_cast< typename std::underlying_type_t< LogMode > >( lhs ) |
                                        static_cast< typename std::underlying_type_t< LogMode > >( rhs ) );
    }

    inline constexpr LogMode& operator|=( LogMode& lhs, LogMode rhs ) noexcept
    {
        lhs = static_cast< LogMode >( static_cast< typename std::underlying_type_t< LogMode > >( lhs ) |
                                        static_cast< typename std::underlying_type_t< LogMode > >( rhs ) );
        return lhs;
    }

    inline constexpr typename std::underlying_type_t< LogMode > operator&( LogMode lhs, LogMode rhs ) noexcept
    {
        return ( static_cast< typename std::underlying_type_t< LogMode > >( lhs ) &
                    static_cast< typename std::underlying_type_t< LogMode > >( rhs ) );
    }

    enum class TraceStatus : core::Int8
    {
        kDefault    = -1,       // default.
        kOn         = 0,        // On.
        kOff        = 1,        // Off.
        kTraceStatusMax
    };

    enum class ClientState : core::Int8
    {
        kUnknown    = -1,
        kNotConnected,
        kConnected,
        kMAX
    };

    // MSTP
    // enum class MessageType : core::UInt8
    // {
    //     DLT_TYPE_LOG        = 0x00,              // Dlt Log Message
    //     DLT_TYPE_APP_TRACE  = 0x01,              // Dlt Trace Message
    //     DLT_TYPE_NW_TRACE   = 0x02,              // Dlt Network Message
    //     DLT_TYPE_CONTROL    = 0x03,              // Dlt Control Message
    // };

    // MTIN(Dlt Log Message)
    // enum class MTINLogMessage : core::UInt8
    // {   
    //     DLT_LOG_FATAL       = 0x1,              // Fatal system error
    //     DLT_LOG_DLT_ERROR   = 0x2,              // Application error)
    //     DLT_LOG_WARN        = 0x3,              // Correct behavior cannot be ensured
    //     DLT_LOGINFO         = 0x4,              // Message of LogLevel type “Information”
    //     DLT_LOG_DEBUG       = 0x5,              // Message of LogLevel type “Debug”
    //     DLT_LOG_VERBOSE     = 0x6,              // Message of LogLevel type “Verbose”
    //     // 0x7 – 0xF: Reserved
    // };

    // MTIN(Dlt Trace Message)
    // enum class MTINTraceMessage : core::UInt8
    // {
    //     DLT_TRACE_VARIABLE          = 0x1,      // (Value of variable)
    //     DLT_TRACE_FUNCTION_IN       = 0x2,      // (Call of a function)
    //     DLT_TRACE_FUNCTION_OUT      = 0x3,      // (Return of a function)
    //     DLT_TRACE_STATE             = 0x4,      // (State of a State Machine)
    //     DLT_TRACE_VFB               = 0x5,      // (RTE events)
    //     // 0x6 – 0xF: Reserved
    // };

    // MTIN(Dlt Network Message)
    // enum class MTINNetworkMessage : core::UInt8
    // {
    //     DLT_NW_TRACE_IPC            = 0x1,      // (Inter-Process-Communication)
    //     DLT_NW_TRACE_CAN            = 0x2,      // (CAN Communications bus)
    //     DLT_NW_TRACE_FLEXRAY        = 0x3,      // (FlexRay Communications bus)
    //     DLT_NW_TRACE_MOST           = 0x4,      // (Most Communications bus)
    //     DLT_NW_TRACE_ETHERNET       = 0x5,      // (Ethernet Communications bus)
    //     DLT_NW_TRACE_SOMEIP         = 0x6,      // (Inter-SOME/IP Communication)
    //     // 0x7-0xF: User Defined (User defined settings)
    // };

    // MTIN(Dlt Control Message)
    // enum class MTINControlMessage : core::UInt8
    // {
    //     DLT_CONTROL_REQUEST         = 0x1,      // (Request Control Message)
    //     DLT_CONTROL_RESPONSE        = 0x2,      // (Respond Control Message)
    //     // 0x3-0xF: Reserved
    // };

    // enum class ServiceID : core::UInt32
    // {
    //     DLT_SERVICE_ID                              = 0x00u,
    //     DLT_SERVICE_ID_SET_LOG_LEVEL                = 0x01u,
    //     DLT_SERVICE_ID_SET_TRACE_STATUS             = 0x02u,
    //     DLT_SERVICE_ID_GET_LOG_INFO                 = 0x03u,
    //     DLT_SERVICE_ID_GET_DEFAULT_LOG_LEVEL        = 0x04u,
    //     DLT_SERVICE_ID_STORE_CONFIG                 = 0x05u,
    //     DLT_SERVICE_ID_RESET_TO_FACTORY_DEFAULT     = 0x06u,
    //     DLT_SERVICE_ID_SET_MESSAGE_FILTERING        = 0x0Au,
    //     DLT_SERVICE_ID_SET_DEFAULT_LOG_LEVEL        = 0x11u,
    //     DLT_SERVICE_ID_SET_DEFAULT_TRACE_STATUS     = 0x12u,
    //     DLT_SERVICE_ID_GET_SOFTWARE_VERSION         = 0x13u,
    //     DLT_SERVICE_ID_GET_DEFAULT_TRACE_STATUS     = 0x15u,
    //     DLT_SERVICE_ID_GET_LOG_CHANNEL_NAME         = 0x17u,
    //     DLT_SERVICE_ID_GET_TRACE_STATUS             = 0x1Fu,
    //     DLT_SERVICE_ID_SET_LOG_CHANNEL_ASSIGNMENT   = 0x20u,
    //     DLT_SERVICE_ID_SET_LOG_CHANNEL_THRESHOLD    = 0x21u,
    //     DLT_SERVICE_ID_GET_LOG_CHANNEL_THRESHOLD    = 0x22u,
    //     DLT_SERVICE_ID_BUFFER_OVERFLOW_NOTIFICATION = 0x23u,
    //     DLT_SERVICE_ID_SYNC_TIMESTAMP               = 0x24u,
    //     DLT_SERVICE_ID_LAST_ENTRY
    // };
} // namespace log
} // namespace lap
#endif

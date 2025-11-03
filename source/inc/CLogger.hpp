/**
 * @file        CLogger.hpp
 * @author      ddkv587 ( ddkv587@gmail.com )
 * @brief       Logger class for AUTOSAR Adaptive Platform logging
 * @date        2025-10-27
 * @details     Provides logger instances for context-based logging with multiple log levels
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

#ifndef LAP_LOG_LOGGER_HPP
#define LAP_LOG_LOGGER_HPP

#include "CCommon.hpp"
#include "CLogStream.hpp"

namespace lap
{
namespace log
{
    //class CLogManager;
    class Logger final
    {
    public:
        LogStream           LogFatal() const noexcept;
        LogStream           LogError() const noexcept;
        LogStream           LogWarn() const noexcept;
        LogStream           LogInfo() const noexcept;
        LogStream           LogDebug() const noexcept;
        LogStream           LogVerbose() const noexcept;
        LogStream           LogOff() const noexcept;

        /** @fn         bool IsEnabled (LogLevel logLevel) const noexcept;
         *  @brief      Check current configured log reporting level.
         *  @param[in]  logLevel        The to be checked log level.
         *  @return     bool            True if desired log level satisfies the configured reporting level.
         */
        bool                IsEnabled( LogLevel logLevel ) const noexcept;

        /** @fn         LogStream WithLevel (LogLevel logLevel) const noexcept;
         *  @brief      Log message with a programmatically determined log level can be written.
         *  @param[in]  logLevel        the log level to use for this LogStream instance
         *  @return     LogStream       a new LogStream instance with the given log level
         */
        LogStream           WithLevel( LogLevel logLevel ) const noexcept;

        inline core::StringView getContextId() const noexcept { return m_strContextID; }

        explicit Logger( core::StringView ctxId, 
                            core::StringView ctxDesc, 
                            LogLevel level = LogLevel::kWarn,
                            TraceStatus status = TraceStatus::kDefault ) noexcept;
        ~Logger() noexcept;

    protected:
        friend class CLogManager;
        friend class LogStream;

        Logger() = delete;
        Logger( const Logger& ) = delete;
        Logger& operator=( const Logger& ) = delete;
        Logger( Logger&& ) = delete;
        Logger& operator=( Logger&& ) = delete;

    private:
        core::String                m_strContextID;
        core::String                m_strContextDesc;
        LogLevel                    m_logLevel;
        TraceStatus                 m_traceStatus;
    };

    /** @fn         Logger& CreateLogger (ara::core::StringView ctxId, lap::core::StringView ctxDescription, LogLevel ctxDefLogLevel=LogLevel::kWarn) noexcept;
     *  @brief      Creates a Logger object, holding the context which is registered in the Logging framework. 
     *  @param[in]  ctxId            The context ID.
     *  @param[in]  ctxDescription   The description of the provided context ID.
     *  @param[in]  ctxDefLogLevel   The default log level, set to Warning severity if not explicitly specified.
     *  @return     Logger&          Reference to the internal managed instance of a Logger object. Ownership stays within the Logging framework.
     */
    Logger&             CreateLogger( lap::core::StringView ctxId = "", lap::core::StringView ctxDescription = "", LogLevel ctxDefLogLevel = LogLevel::kWarn ) noexcept;

    /** @fn         ClientState remoteClientState () noexcept;
     *  @brief      Fetches the connection state from the DLT back-end of a possibly available remote client.
     *  @param      None
     *  @return     ClientState     The current client state.
     */
    ClientState         remoteClientState() noexcept;

    // /** @fn         template <typename T> Argument<T> Arg (T &&arg, const char *name=nullptr, const char *unit=nullptr) noexcept;
    //  *  @brief      Create a wrapper object for the given arguments. 
    //  *  @param[in]  arg             an argument payload object
    //  *  @param[in]  name            an optional "name" attribute for arg
    //  *  @param[in]  unit            an optional "unit" attribute for arg
    //  *  @return     Argument<T>     a wrapper object holding the supplied arguments
    //  */
    // template < typename T >
    // Argument< T >       Arg(T &&arg, const char *name = nullptr, const char *unit = nullptr) noexcept;

    /** @fn         template <typename MsgId, typename... Params> void Log (const MsgId &id, const Params &... args) noexcept;
     *  @brief      Log a modeled message.
     *  @param      MsgId           the type of the id parameter
     *  @param      arg             the types of the args parameters
     *  @param[in]  id              an implementation-defined type identifying the message object
     *  @param[in]  args            the arguments to add to the message
     *  @return     None
     */
    template < typename MsgId, typename... Params >
    void                Log ( const MsgId &id, const Params &... args ) noexcept;

    constexpr LogHex8   HexFormat ( uint8_t value ) noexcept        { return LogHex8{ value }; }
    constexpr LogHex8   HexFormat ( int8_t value ) noexcept         { return LogHex8{ static_cast< core::UInt8 >( value ) }; }
    constexpr LogHex16  HexFormat ( uint16_t value ) noexcept       { return LogHex16{ value }; }
    constexpr LogHex16  HexFormat ( int16_t value ) noexcept        { return LogHex16{ static_cast< core::UInt16 >( value ) }; }
    constexpr LogHex32  HexFormat ( uint32_t value ) noexcept       { return LogHex32{ value }; }
    constexpr LogHex32  HexFormat ( int32_t value ) noexcept        { return LogHex32{ static_cast< core::UInt32 >( value ) }; }
    constexpr LogHex64  HexFormat ( uint64_t value ) noexcept       { return LogHex64{ value }; }
    constexpr LogHex64  HexFormat ( int64_t value ) noexcept        { return LogHex64{ static_cast< core::UInt64 >( value ) }; }

    constexpr LogBin8   BinFormat ( uint8_t value ) noexcept        { return LogBin8{ value }; }
    constexpr LogBin8   BinFormat ( int8_t value ) noexcept         { return LogBin8{ static_cast< core::UInt8 >( value ) }; }
    constexpr LogBin16  BinFormat ( uint16_t value ) noexcept       { return LogBin16{ value }; }
    constexpr LogBin16  BinFormat ( int16_t value ) noexcept        { return LogBin16{ static_cast< core::UInt16 >( value ) }; }
    constexpr LogBin32  BinFormat ( uint32_t value ) noexcept       { return LogBin32{ value }; }
    constexpr LogBin32  BinFormat ( int32_t value ) noexcept        { return LogBin32{ static_cast< core::UInt32 >( value ) }; }
    constexpr LogBin64  BinFormat ( uint64_t value ) noexcept       { return LogBin64{ value }; }
    constexpr LogBin64  BinFormat ( int64_t value ) noexcept        { return LogBin64{ static_cast< core::UInt64 >( value ) }; }
} // namespace core
} // namespace lap
#endif
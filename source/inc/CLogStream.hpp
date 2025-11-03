/**
 * @file        CLogStream.hpp
 * @author      ddkv587 ( ddkv587@gmail.com )
 * @brief       LogStream class for AUTOSAR Adaptive Platform logging
 * @date        2025-10-27
 * @details     Provides stream-based logging interface with various data type support
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

#ifndef LAP_LOG_LOGSTREAM_HPP
#define LAP_LOG_LOGSTREAM_HPP

#include <core/CErrorCode.hpp>
#include <core/CInstanceSpecifier.hpp>
#include <core/CMemory.hpp>
#include <core/CSpan.hpp>

#include "CCommon.hpp"

namespace lap
{
namespace log
{
    struct LogHex8  { core::UInt8 value; };
    struct LogHex16 { core::UInt16 value; };
    struct LogHex32 { core::UInt32 value; };
    struct LogHex64 { core::UInt64 value; };
    struct LogBin8  { core::UInt8 value; };
    struct LogBin16 { core::UInt16 value; };
    struct LogBin32 { core::UInt32 value; };
    struct LogBin64 { core::UInt64 value; };

    class Logger;
    class LogStream final
    {
    public:
        static constexpr size_t MAX_LOG_SIZE = 200;  // Fixed log message size
        
        IMP_OPERATOR_NEW(LogStream)  // Use Core Memory allocator (256-byte block)

        void        Flush () noexcept;
        LogStream&  WithLocation ( core::StringView file, core::Int32 line ) noexcept;

        LogStream&  operator<< ( core::Bool value ) noexcept;
        LogStream&  operator<< ( core::UInt8 value ) noexcept;
        LogStream&  operator<< ( core::UInt16 value ) noexcept;
        LogStream&  operator<< ( core::UInt32 value ) noexcept;
        LogStream&  operator<< ( core::UInt64 value ) noexcept;

        LogStream&  operator<< ( core::Int8 value ) noexcept;
        LogStream&  operator<< ( core::Int16 value ) noexcept;
        LogStream&  operator<< ( core::Int32 value ) noexcept;
        LogStream&  operator<< ( core::Int64 value ) noexcept;
        LogStream&  operator<< ( core::Float value ) noexcept;
        LogStream&  operator<< ( core::Double value ) noexcept;

        LogStream&  operator<< ( const LogHex8 &value ) noexcept;
        LogStream&  operator<< ( const LogHex16 &value ) noexcept;
        LogStream&  operator<< ( const LogHex32 &value ) noexcept;
        LogStream&  operator<< ( const LogHex64 &value ) noexcept;

        LogStream&  operator<< ( const LogBin8 &value ) noexcept;
        LogStream&  operator<< ( const LogBin16 &value ) noexcept;
        LogStream&  operator<< ( const LogBin32 &value ) noexcept;
        LogStream&  operator<< ( const LogBin64 &value ) noexcept;

        LogStream&  operator<< ( const core::StringView value ) noexcept;
        LogStream&  operator<< ( const char *const value ) noexcept;

        LogStream&  operator<< ( core::Span< const core::Byte > data ) noexcept;

    /** @fn         void LogFormat( LogLevel logLevel, ... ) const noexcept;
         *  @brief      Log message with format value.
         *  @param[in]  logLevel        the log level to use for this log message
         *  @param[in]  ...             the format value list. eg: "string: %s, int: %d", STRING_VALUE, INT_VALUE 
         *  @return     LogStream       a new LogStream instance with the given log level
         */
    LogStream&  logFormat ( const core::Char *fmt, ... ) noexcept;

        ~LogStream() noexcept;

    protected:
        friend class Logger;
        friend class SinkManager;  // Allow SinkManager to access m_logBuffer directly

        LogStream( LogLevel, const Logger& ) noexcept;

        // Explicitly delete all copy and move operations (zero-copy only)
        LogStream() = delete;
        LogStream( const LogStream& ) = delete;
        LogStream& operator=( const LogStream& ) = delete;
        LogStream( LogStream&& ) = delete;
        LogStream& operator=( LogStream&& ) = delete;

    private:
        // Inline helper functions to estimate size needed
        static constexpr inline size_t estimateSize(core::Bool) noexcept { return 1; }
        static constexpr inline size_t estimateSize(core::UInt8) noexcept { return 3; }
        static constexpr inline size_t estimateSize(core::UInt16) noexcept { return 5; }
        static constexpr inline size_t estimateSize(core::UInt32) noexcept { return 10; }
        static constexpr inline size_t estimateSize(core::UInt64) noexcept { return 20; }
        static constexpr inline size_t estimateSize(core::Int8) noexcept { return 4; }
        static constexpr inline size_t estimateSize(core::Int16) noexcept { return 6; }
        static constexpr inline size_t estimateSize(core::Int32) noexcept { return 11; }
        static constexpr inline size_t estimateSize(core::Int64) noexcept { return 20; }
        static constexpr inline size_t estimateSize(core::Float) noexcept { return 16; }
        static constexpr inline size_t estimateSize(core::Double) noexcept { return 24; }
        
        void                    flushBuffer() noexcept;  // Flush current buffer to sinks
        void                    checkAndFlush(size_t additionalSize) noexcept;  // Check if flush needed

    public:
        // Direct access methods (no copy, for friend classes)
        inline const char* getBuffer() const noexcept { return m_logBuffer; }
        inline size_t getBufferSize() const noexcept { return m_bufferPos; }
        inline LogLevelType getLevel() const noexcept { return m_logLevel; }
        inline const Logger& getLogger() const noexcept { return m_logger; }

    private:
        LogLevelType            m_logLevel;
        const Logger&           m_logger;
        char                    m_logBuffer[MAX_LOG_SIZE];  // Fixed-size log buffer
        size_t                  m_bufferPos;  // Current position in buffer
    };

    LogStream& operator<< ( LogStream &out, LogLevel value ) noexcept;
    LogStream& operator<< ( LogStream &out, const lap::core::ErrorCode &ec ) noexcept;

    template <typename Rep, typename Period>
    LogStream& operator<< ( LogStream &out, const std::chrono::duration< Rep, Period > &value ) noexcept;

    /** @fn         template <typename T> LogStream& operator<< (const Argument< T > &arg) noexcept;
     *  @brief      Log an argument with attributes 
     *  @param      T               the argument payload type
     *  @param[in]  arg             the argument wrapper object
     *  @return     LogStream &     *this
     */
    // template < typename T >
    // LogStream& operator<<(const Argument< T > &arg) noexcept;

    LogStream& operator<< ( LogStream &out, const core::InstanceSpecifier &value ) noexcept;
} // namespace core
} // namespace lap
#endif
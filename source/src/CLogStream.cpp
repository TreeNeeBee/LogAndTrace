#include <stdarg.h>
#include <ctime>
#include <lap/core/CPath.hpp>
#include <lap/core/CTime.hpp>
#include <lap/core/CCrypto.hpp>
#include "CLogStream.hpp"
#include "CLogger.hpp"
#include "CLogManager.hpp"
#include "CSinkManager.hpp"

namespace lap
{
namespace log
{
    LogStream::LogStream( LogLevel level, const Logger& logger ) noexcept
        : m_logLevel( static_cast< LogLevelType >( level ) )
        , m_logger( logger )
        , m_bufferPos( 0 )
        , m_encodeEnabled( false )
    {
        // Initialize buffer to empty
        m_logBuffer[0] = '\0';
    }

    LogStream::~LogStream() noexcept
    {
        // Flush any remaining content
        if ( m_bufferPos > 0 ) {
            flushBuffer();
        }
    }

    void LogStream::Flush() noexcept
    {
        if ( m_bufferPos > 0 ) {
            flushBuffer();
            // Reset buffer for next message
            m_bufferPos = 0;
            m_logBuffer[0] = '\0';
        }
    }

    void LogStream::checkAndFlush(size_t additionalSize) noexcept
    {
        // If adding this would exceed buffer, flush now
        if ( m_bufferPos + additionalSize >= MAX_LOG_SIZE ) {
            flushBuffer();
            m_bufferPos = 0;
            m_logBuffer[0] = '\0';
        }
    }

    void LogStream::flushBuffer() noexcept
    {
        if ( m_bufferPos == 0 ) {
            return;
        }
        
        // Get SinkManager from LogManager
        auto& logMgr = LogManager::getInstance();
        if ( !logMgr.isInitialized() ) {
            return;
        }
        
        auto& sinkMgr = logMgr.getSinkManager();
        
        // Check if any sink needs this log level
        if ( !sinkMgr.shouldLog(static_cast<LogLevel>(m_logLevel)) ) {
            return;
        }
        
        // Check if base64 encoding is enabled for this LogStream
        if ( m_encodeEnabled ) {
            // Encode the log message using Core's base64 encoder
            core::String encoded = core::Crypto::Util::base64Encode(
                reinterpret_cast<const core::UInt8*>(m_logBuffer), 
                m_bufferPos
            );
            
            // Create a temporary buffer to hold the encoded message
            char encodedBuffer[MAX_LOG_SIZE * 2];  // Base64 can expand up to 4/3 of original
            size_t encodedLen = encoded.size();
            if ( encodedLen >= sizeof(encodedBuffer) ) {
                encodedLen = sizeof(encodedBuffer) - 1;
            }
            std::memcpy(encodedBuffer, encoded.data(), encodedLen);
            encodedBuffer[encodedLen] = '\0';
            
            // Temporarily replace buffer for sink writing
            char* originalBuffer = const_cast<char*>(m_logBuffer);
            size_t originalPos = m_bufferPos;
            
            // Copy encoded data back to m_logBuffer for SinkManager
            std::memcpy(originalBuffer, encodedBuffer, encodedLen);
            const_cast<size_t&>(m_bufferPos) = encodedLen;
            originalBuffer[encodedLen] = '\0';
            
            // Write to sinks (SinkManager will access m_logBuffer as friend)
            sinkMgr.write(*this);
            
            // Restore original state (though buffer will be cleared after this)
            const_cast<size_t&>(m_bufferPos) = originalPos;
        } else {
            // Write to sinks without encoding (SinkManager will access m_logBuffer as friend)
            sinkMgr.write(*this);
        }
    }

    LogStream& LogStream::WithLocation( core::StringView file, core::Int32 line ) noexcept
    {
        char temp[64];
        // Use Core Path utility to get basename
        core::StringView basename = core::Path::getBaseName(file);
        
        int len = std::snprintf( temp, sizeof(temp), "[%.*s:%d] ", 
                                static_cast<int>(basename.size()), basename.data(), line );
        if ( len > 0 ) {
            checkAndFlush(len);
            std::memcpy( m_logBuffer + m_bufferPos, temp, len );
            m_bufferPos += len;
            m_logBuffer[m_bufferPos] = '\0';
        }
        return *this;
    }

    LogStream& LogStream::WithEncode( bool enable ) noexcept
    {
        m_encodeEnabled = enable;
        return *this;
    }

    //=============================================================================
    // Stream Operators - Direct write to logBuffer
    //=============================================================================

    LogStream& LogStream::operator<< ( core::Bool value ) noexcept
    {
        checkAndFlush(estimateSize(value));
        m_logBuffer[m_bufferPos++] = value ? '1' : '0';
        m_logBuffer[m_bufferPos] = '\0';
        return *this;
    }

    LogStream& LogStream::operator<< ( core::UInt8 value ) noexcept
    {
        checkAndFlush(estimateSize(value));
        int written = std::snprintf( m_logBuffer + m_bufferPos, MAX_LOG_SIZE - m_bufferPos, "%u", static_cast<unsigned>(value) );
        if ( written > 0 ) m_bufferPos += written;
        return *this;
    }

    LogStream& LogStream::operator<< ( core::UInt16 value ) noexcept
    {
        checkAndFlush(estimateSize(value));
        int written = std::snprintf( m_logBuffer + m_bufferPos, MAX_LOG_SIZE - m_bufferPos, "%u", value );
        if ( written > 0 ) m_bufferPos += written;
        return *this;
    }

    LogStream& LogStream::operator<< ( core::UInt32 value ) noexcept
    {
        checkAndFlush(estimateSize(value));
        int written = std::snprintf( m_logBuffer + m_bufferPos, MAX_LOG_SIZE - m_bufferPos, "%u", value );
        if ( written > 0 ) m_bufferPos += written;
        return *this;
    }

    LogStream& LogStream::operator<< ( core::UInt64 value ) noexcept
    {
        checkAndFlush(estimateSize(value));
        int written = std::snprintf( m_logBuffer + m_bufferPos, MAX_LOG_SIZE - m_bufferPos, "%lu", value );
        if ( written > 0 ) m_bufferPos += written;
        return *this;
    }

    LogStream& LogStream::operator<< ( core::Int8 value ) noexcept
    {
        checkAndFlush(estimateSize(value));
        int written = std::snprintf( m_logBuffer + m_bufferPos, MAX_LOG_SIZE - m_bufferPos, "%d", static_cast<int>(value) );
        if ( written > 0 ) m_bufferPos += written;
        return *this;
    }

    LogStream& LogStream::operator<< ( core::Int16 value ) noexcept
    {
        checkAndFlush(estimateSize(value));
        int written = std::snprintf( m_logBuffer + m_bufferPos, MAX_LOG_SIZE - m_bufferPos, "%d", value );
        if ( written > 0 ) m_bufferPos += written;
        return *this;
    }

    LogStream& LogStream::operator<< ( core::Int32 value ) noexcept
    {
        checkAndFlush(estimateSize(value));
        int written = std::snprintf( m_logBuffer + m_bufferPos, MAX_LOG_SIZE - m_bufferPos, "%d", value );
        if ( written > 0 ) m_bufferPos += written;
        return *this;
    }

    LogStream& LogStream::operator<< ( core::Int64 value ) noexcept
    {
        checkAndFlush(estimateSize(value));
        int written = std::snprintf( m_logBuffer + m_bufferPos, MAX_LOG_SIZE - m_bufferPos, "%ld", value );
        if ( written > 0 ) m_bufferPos += written;
        return *this;
    }

    LogStream& LogStream::operator<< ( core::Float value ) noexcept
    {
        checkAndFlush(estimateSize(value));
        int written = std::snprintf( m_logBuffer + m_bufferPos, MAX_LOG_SIZE - m_bufferPos, "%.6f", value );
        if ( written > 0 ) m_bufferPos += written;
        return *this;
    }

    LogStream& LogStream::operator<< ( core::Double value ) noexcept
    {
        checkAndFlush(estimateSize(value));
        int written = std::snprintf( m_logBuffer + m_bufferPos, MAX_LOG_SIZE - m_bufferPos, "%.12f", value );
        if ( written > 0 ) m_bufferPos += written;
        return *this;
    }

    LogStream& LogStream::operator<< ( const LogHex8 &value ) noexcept
    {
        checkAndFlush(8);
        int written = std::snprintf( m_logBuffer + m_bufferPos, MAX_LOG_SIZE - m_bufferPos, "0x%02X", value.value );
        if ( written > 0 ) m_bufferPos += written;
        return *this;
    }

    LogStream& LogStream::operator<< ( const LogHex16 &value ) noexcept
    {
        checkAndFlush(16);
        int written = std::snprintf( m_logBuffer + m_bufferPos, MAX_LOG_SIZE - m_bufferPos, "0x%04X", value.value );
        if ( written > 0 ) m_bufferPos += written;
        return *this;
    }

    LogStream& LogStream::operator<< ( const LogHex32 &value ) noexcept
    {
        checkAndFlush(16);
        int written = std::snprintf( m_logBuffer + m_bufferPos, MAX_LOG_SIZE - m_bufferPos, "0x%08X", value.value );
        if ( written > 0 ) m_bufferPos += written;
        return *this;
    }

    LogStream& LogStream::operator<< ( const LogHex64 &value ) noexcept
    {
        checkAndFlush(24);
        int written = std::snprintf( m_logBuffer + m_bufferPos, MAX_LOG_SIZE - m_bufferPos, "0x%016lX", value.value );
        if ( written > 0 ) m_bufferPos += written;
        return *this;
    }

    LogStream& LogStream::operator<< ( const LogBin8 &value ) noexcept
    {
        checkAndFlush(16);
        m_logBuffer[m_bufferPos++] = '0';
        m_logBuffer[m_bufferPos++] = 'b';
        for (int i = 7; i >= 0; i--) {
            m_logBuffer[m_bufferPos++] = ((value.value >> i) & 1) ? '1' : '0';
        }
        m_logBuffer[m_bufferPos] = '\0';
        return *this;
    }

    LogStream& LogStream::operator<< ( const LogBin16 &value ) noexcept
    {
        UNUSED( value );
        checkAndFlush(9);
        const char* str = "0b(bin16)";
        std::memcpy( m_logBuffer + m_bufferPos, str, 9 );
        m_bufferPos += 9;
        m_logBuffer[m_bufferPos] = '\0';
        return *this;
    }

    LogStream& LogStream::operator<< ( const LogBin32 &value ) noexcept
    {
        UNUSED( value );
        checkAndFlush(9);
        const char* str = "0b(bin32)";
        std::memcpy( m_logBuffer + m_bufferPos, str, 9 );
        m_bufferPos += 9;
        m_logBuffer[m_bufferPos] = '\0';
        return *this;
    }

    LogStream& LogStream::operator<< ( const LogBin64 &value ) noexcept
    {
        UNUSED( value );
        checkAndFlush(9);
        const char* str = "0b(bin64)";
        std::memcpy( m_logBuffer + m_bufferPos, str, 9 );
        m_bufferPos += 9;
        m_logBuffer[m_bufferPos] = '\0';
        return *this;
    }

    LogStream& LogStream::operator<< ( const core::StringView value ) noexcept
    {
        size_t len = value.size();
        if ( len > 0 ) {
            checkAndFlush(len);
            // Truncate if still too large
            if ( m_bufferPos + len >= MAX_LOG_SIZE ) {
                len = MAX_LOG_SIZE - m_bufferPos - 1;
            }
            std::memcpy( m_logBuffer + m_bufferPos, value.data(), len );
            m_bufferPos += len;
            m_logBuffer[m_bufferPos] = '\0';
        }
        return *this;
    }

    LogStream& LogStream::operator<< ( const core::String& value ) noexcept
    {
        return *this << core::StringView(value);
    }

    LogStream& LogStream::operator<< ( const char *const value ) noexcept
    {
        if ( value ) {
            size_t len = std::strlen(value);
            if ( len > 0 ) {
                checkAndFlush(len);
                // Truncate if still too large
                if ( m_bufferPos + len >= MAX_LOG_SIZE ) {
                    len = MAX_LOG_SIZE - m_bufferPos - 1;
                }
                std::memcpy( m_logBuffer + m_bufferPos, value, len );
                m_bufferPos += len;
                m_logBuffer[m_bufferPos] = '\0';
            }
        }
        return *this;
    }

    LogStream& LogStream::operator<< ( core::Span< const core::Byte > data ) noexcept
    {
        checkAndFlush(20);
        int written = std::snprintf( m_logBuffer + m_bufferPos, MAX_LOG_SIZE - m_bufferPos, "[binary:%zu]", data.size() );
        if ( written > 0 ) m_bufferPos += written;
        return *this;
    }

    LogStream& LogStream::logFormat( const core::Char *fmt, ... ) noexcept
    {
#ifdef LAP_DEBUG
        va_list args;
        va_start( args, fmt );
        
        checkAndFlush(100);  // Estimate
        int written = std::vsnprintf( m_logBuffer + m_bufferPos, MAX_LOG_SIZE - m_bufferPos, fmt, args );
        
        va_end( args );
        
        if ( written > 0 ) {
            m_bufferPos += written;
        }
#else
        UNUSED( fmt );
#endif
        return *this;
    }

    //=============================================================================
    // Non-member Stream Operators
    //=============================================================================

    LogStream& operator<< ( LogStream &out, LogLevel value ) noexcept
    {
        out << toString( value );
        return out;
    }

    LogStream& operator<< ( LogStream &out, const lap::core::ErrorCode &ec ) noexcept
    {
        out << ec.Domain().Name() << ":" << ec.Value();
        return out;
    }

    template <typename Rep, typename Period>
    LogStream& operator<< ( LogStream &out, const std::chrono::duration< Rep, Period > &value ) noexcept
    {
        char buffer[32];
        std::snprintf( buffer, sizeof(buffer), "%.12e", static_cast<double>(value.count()) );
        out << buffer;
        return out;
    }

    LogStream& operator<< ( LogStream &out, const lap::core::InstanceSpecifier &value ) noexcept
    {
        out << value.ToString();
        return out;
    }

} // namespace log
} // namespace lap

/**
 * @file        CFileSink.hpp
 * @author      ddkv587 ( ddkv587@gmail.com )
 * @brief       File output sink with rotation support
 * @date        2025-10-28
 * @details     Writes logs to file with optional size-based rotation
 * @copyright   Copyright (c) 2025
 */

#ifndef LAP_LOG_FILESINK_HPP
#define LAP_LOG_FILESINK_HPP

#include "ISink.hpp"
#include <core/CMemory.hpp>
#include <core/CFile.hpp>

namespace lap
{
namespace log
{
    /**
     * @brief File sink for persistent log storage
     * 
     * Features:
     * - Buffered file I/O for performance
     * - Size-based log rotation
     * - Automatic backup file management
     * - Configurable flush policy
     */
    class FileSink : public ISink
    {
    public:
        IMP_OPERATOR_NEW(FileSink)
        
        /**
         * @brief Constructor
         * @param filePath Log file path
         * @param maxSize Maximum file size before rotation (0 = no rotation)
         * @param maxFiles Maximum number of backup files to keep
         * @param minLevel Minimum log level to output
         * @param appId Application ID (max 4 bytes)
         */
        explicit FileSink(
            core::StringView filePath,
            core::Size maxSize = 10 * 1024 * 1024,  // 10MB default
            core::UInt32 maxFiles = 5,
            LogLevel minLevel = LogLevel::kVerbose,
            core::StringView appId = ""
        ) noexcept;
        
        virtual ~FileSink() noexcept override;
        
        // ISink interface implementation
        virtual void write(
            core::UInt64 timestamp,
            core::UInt32 threadId,
            LogLevelType level,
            core::StringView contextId,
            core::StringView message
        ) noexcept override;
        
        virtual void flush() noexcept override;
        virtual core::Bool isEnabled() const noexcept override { return m_enabled && m_file.isOpen(); }
        virtual core::StringView getName() const noexcept override { return "File"; }
        virtual void setLevel(LogLevel level) noexcept override { m_minLevel = level; }
        virtual core::Bool shouldLog(LogLevel level) const noexcept override;
        
        /**
         * @brief Enable/disable this sink
         * @param enabled Enable state
         */
        void setEnabled(core::Bool enabled) noexcept { m_enabled = enabled; }
        
        /**
         * @brief Get current file size
         * @return File size in bytes
         */
        core::Size getCurrentSize() const noexcept { return m_currentSize; }
        
        /**
         * @brief Manually trigger log rotation
         * @return true if rotation succeeded, false otherwise
         */
        core::Bool rotate() noexcept;
        
    private:
        /**
         * @brief Open log file for writing (append mode with O_APPEND for atomicity)
         * @return true if succeeded, false otherwise
         */
        core::Bool openFile() noexcept;
        
        /**
         * @brief Close current log file
         */
        void closeFile() noexcept;
        
        /**
         * @brief Check if rotation is needed and perform it
         */
        void checkRotation() noexcept;
        
    private:
        core::String    m_filePath;     ///< Log file path
        core::File      m_file;         ///< File instance (RAII fd wrapper)
        core::Size      m_maxSize;      ///< Max size before rotation
        core::UInt32    m_maxFiles;     ///< Max backup files
        core::Size      m_currentSize;  ///< Current file size
        core::Bool      m_enabled;      ///< Enable state
        LogLevel        m_minLevel;     ///< Minimum log level
        char            m_appId[5];     ///< Application ID (4 bytes + null)
    };
    
} // namespace log
} // namespace lap

#endif // LAP_LOG_FILESINK_HPP

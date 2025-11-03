/**
 * @file        CLog.hpp
 * @author      ddkv587 ( ddkv587@gmail.com )
 * @brief       Main header file for AUTOSAR Adaptive Platform Log module
 * @date        2025-10-27
 * @details     Aggregates all logging functionality headers and provides logging macros
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

#ifndef LAP_LOG_LOG_HPP
#define LAP_LOG_LOG_HPP

#include "CLogStream.hpp"
#include "CLogger.hpp"
#include "CLogManager.hpp"

namespace lap
{
namespace log
{
    #define LAP_LOG( ... )                                      ::lap::log::CreateLogger( __VA_ARGS__ )

    #define LAP_LOG_VERBOSE( ... )                              LAP_LOG( __VA_ARGS__ ).LogVerbose()
    #define LAP_LOG_DEBUG( ... )                                LAP_LOG( __VA_ARGS__ ).LogDebug()
    #define LAP_LOG_INFO( ... )                                 LAP_LOG( __VA_ARGS__ ).LogInfo()
    #define LAP_LOG_WARN( ... )                                 LAP_LOG( __VA_ARGS__ ).LogWarn()
    #define LAP_LOG_ERROR( ... )                                LAP_LOG( __VA_ARGS__ ).LogError()
    #define LAP_LOG_FATAL( ... )                                LAP_LOG( __VA_ARGS__ ).LogFatal()
    #define LAP_LOG_OFF( ... )                                  LAP_LOG( __VA_ARGS__ ).LogOff()

    #define LAP_LOG_VERBOSE_WITH_FILE_LINE( ... )               LAP_LOG( __VA_ARGS__ ).LogVerbose().WithLocation( __FILE__, __LINE__ )
    #define LAP_LOG_DEBUG_WITH_FILE_LINE( ... )                 LAP_LOG( __VA_ARGS__ ).LogDebug().WithLocation( __FILE__, __LINE__ )
    #define LAP_LOG_INFO_WITH_FILE_LINE( ... )                  LAP_LOG( __VA_ARGS__ ).LogInfo().WithLocation( __FILE__, __LINE__ )
    #define LAP_LOG_WARN_WITH_FILE_LINE( ... )                  LAP_LOG( __VA_ARGS__ ).LogWarn().WithLocation( __FILE__, __LINE__ )
    #define LAP_LOG_ERROR_WITH_FILE_LINE( ... )                 LAP_LOG( __VA_ARGS__ ).LogError().WithLocation( __FILE__, __LINE__ )
    #define LAP_LOG_FATAL_WITH_FILE_LINE( ... )                 LAP_LOG( __VA_ARGS__ ).LogFatal().WithLocation( __FILE__, __LINE__ )
    #define LAP_LOG_OFF_WITH_FILE_LINE( ... )                   LAP_LOG( __VA_ARGS__ ).LogOff().WithLocation( __FILE__, __LINE__ )
} // namespace log
} // namespace lap
#endif
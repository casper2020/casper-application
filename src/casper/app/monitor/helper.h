/**
 * @file helper.h
 *
 * Copyright (c) 2011-2022 Cloudware S.A. All rights reserved.
 *
 * This file is part of casper-app.
 *
 * casper-app is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * casper-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with casper.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CASPER_APP_MONITOR_HELPER_H_
#define CASPER_APP_MONITOR_HELPER_H_
#pragma once

#include "osal/osal_singleton.h"

#include <unistd.h> // getppid
#include <mutex>    // std::mutex

#include "sys/error.h"
#include "sys/process.h"

#include "casper/app/logger.h"

#ifdef CASPER_APP_MONITOR_SET_ERROR
    #undef CASPER_APP_MONITOR_SET_ERROR
#endif
#define CASPER_APP_MONITOR_SET_ERROR(a_process, o_error, a_errno, a_format, ...) \
    casper::app::monitor::Helper::GetInstance().SetError(a_process, o_error, __FILE__, __FUNCTION__, __LINE__, a_errno, a_format, __VA_ARGS__);

#ifdef CASPER_APP_MONITOR_RESET_ERROR
    #undef CASPER_APP_MONITOR_RESET_ERROR
#endif
#define CASPER_APP_MONITOR_RESET_ERROR(o_error) \
    o_error.Reset();

#ifdef CASPER_APP_MONITOR_LOG_ERROR
    #undef CASPER_APP_MONITOR_LOG_ERROR
#endif
#define CASPER_APP_MONITOR_LOG_ERROR(a_token, a_error) [&] () { \
    CASPER_APP_LOG(a_token, "------ [B] %s ------", "ERROR"); \
    CASPER_APP_LOG(a_token, "%s:%d", a_error.function(), a_error.line()); \
    CASPER_APP_LOG(a_token, "message: %s", a_error.message().c_str()); \
    if ( sys::Error::k_no_error_ != a_error.no() ) { \
        CASPER_APP_LOG(a_token, "system: %s", a_error.str().c_str()); \
    } \
    CASPER_APP_LOG(a_token, "------ [E] %s ------", "ERROR"); \
} ()

#ifdef CASPER_APP_MONITOR_FATAL_ERROR
    #undef CASPER_APP_MONITOR_FATAL_ERROR
#endif

#define CASPER_APP_MONITOR_FATAL_ERROR(a_format, ...)[&] () { \
    ::ev::LoggerV2::GetInstance().Log(::casper::app::Logger::GetInstance().client(), "status", a_format, __VA_ARGS__); \
    exit(-1); \
} ()

namespace casper
{
    
    namespace app
    {
        
        namespace monitor
        {
            
            // ---- //
            class Helper;
            class HelperInitializer final : public ::osal::Initializer<Helper>
            {
                
            public: // Constructor(s) / Destructor
                
                HelperInitializer (Helper& a_instance)
                : ::osal::Initializer<Helper>(a_instance)
                {
                    /* empty */
                }
                
                virtual ~HelperInitializer ()
                {
                    /* empty */
                }
                
            };
            
            // ---- //
            class Helper final : public osal::Singleton<Helper, HelperInitializer>
            {
                
            private: // Threading
                
                std::mutex mutex_;
                                
            public: // Method(s) / Function(s)
                
                void SetError (const ::sys::Process* a_process, ::sys::Error& o_error,
                               const char* const a_file, const char* const a_function, const int a_line,
                               const errno_t a_errno, const char* const a_format, ...)  __attribute__((format(printf, 8, 9))
                );
                
            }; // end of namespace 'helper'
            
        } // end of namespace 'monitor'
        
    } // end of namespace 'app'
    
} // end of namespace 'casper'

#endif // CASPER_APP_MONITOR_HELPER_H_

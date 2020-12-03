/**
 * @file logger.h
 *
 * Based on code originally developed for NDrive S.A.
 *
 * Copyright (c) 2011-2018 Cloudware S.A. All rights reserved.
 *
 * This file is part of casper.
 *
 * casper is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * casper is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with osal.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CASPER_APP_LOGGER_H_
#define CASPER_APP_LOGGER_H_
#pragma once

#include <string>

#include "cc/singleton.h"

#include "ev/logger_v2.h"

#ifdef CASPER_APP_LOG
    #undef CASPER_APP_LOG
#endif
#define CASPER_APP_LOG(a_token, a_format, ...) \
    if ( ::ev::LoggerV2::GetInstance().IsRegistered(::casper::app::Logger::GetInstance().client(), a_token) ) { \
        ::ev::LoggerV2::GetInstance().Log(::casper::app::Logger::GetInstance().client(), a_token, "[%-16.16s] " a_format, __FUNCTION__, __VA_ARGS__); \
    } else { \
        fprintf(stderr, a_format, __VA_ARGS__); \
        fflush(stderr); \
    }

#ifdef CASPER_APP_DEBUG_LOG
    #undef CASPER_APP_DEBUG_LOG
#endif
#define CASPER_APP_DEBUG_LOG(a_token, a_format, ...) \
    ::ev::LoggerV2::GetInstance().Log(::casper::app::Logger::GetInstance().client(), a_token, "[%-16.16s] " a_format, __FUNCTION__, __VA_ARGS__);

#ifdef CASPER_APP_LOG_OWNS_FD
    #undef CASPER_APP_LOG_OWNS_FD
#endif
#define CASPER_APP_LOG_OWNS_FD(a_fd) [&] () -> bool { \
    return ::ev::LoggerV2::GetInstance().Using(nullptr, a_fd); \
} ()

namespace casper
{

    namespace app
    {
        
        // ---- //
        
        class Logger;
        
        class Initializer final : public ::cc::Initializer<Logger>
        {
            
        public: // Constructor(s) / Destructor
            
            Initializer (Logger& a_instance);
            virtual ~Initializer ();
            
        };
        
        // ---- //
        class Logger final : public ::cc::Singleton<Logger, Initializer>
        {
            
            friend class Initializer;
            
        private: //
            
            ::ev::Loggable::Data*   loggable_data_;
            ::ev::LoggerV2::Client* client_;
            std::string             path_;
            
        public: // Method(s) / Function(s)
            
            void Startup (const std::string& a_path, const std::string& a_module, const std::string& a_tag);
            void Restart (const std::string& a_module, const std::string& a_tag);
            
        private: // Method(s) / Fucntion(s)
            
            void InnerStartup (const std::string& a_path,
                               const std::string& a_module, const std::string& a_tag,
                               const char* const a_caller_func);
            void InnerShutdown (const bool a_complete);
            
        public: // Inline Method(s) / Function(s)
            
            ::ev::LoggerV2::Client* client        () const;
            ::ev::Loggable::Data&   loggable_data () const;

        }; // end of class 'Logger'

        inline ::ev::LoggerV2::Client* Logger::client () const
        {
            return client_;
        }

        inline ::ev::Loggable::Data& Logger::loggable_data () const
        {
            return *loggable_data_;
        }

    } // end of namespace 'app'
    
} // end of namespace 'casper'


#endif // CASPER_APP_LOGGER_H_

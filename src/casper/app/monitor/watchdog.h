/**
 * @file watchdog.h
 *
 * Copyright (c) 2011-2019 Cloudware S.A. All rights reserved.
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

#ifndef CASPER_APP_MONITOR_WATCHDOG_H_
#define CASPER_APP_MONITOR_WATCHDOG_H_
#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>
#include <stdlib.h> // malloc, free
#include <string.h> // strdup
#include <thread>
#include <mutex>
#include <functional>
#include <atomic>  // std::atomic
#include <fstream>  // std::ifstream

#include "cc/singleton.h"
#include "casper/app/logger.h"

#include "osal/condition_variable.h"

#include "sys/darwin/process.h"

#include "cc/exception.h"

#include "json/json.h"

#ifdef CASPER_APP_WATCHDOG_LOCK
    #undef CASPER_APP_WATCHDOG_LOCK
#endif
#define CASPER_APP_WATCHDOG_LOCK(a_function, a_line) [&] () { \
    if ( true == detached_ ) { \
        CASPER_APP_LOG("status", "%s:%d %s", a_function, a_line, "lock"); \
        if ( false ) assert(0 == locks_.fetch_add(1)); \
        mutex_.lock(); \
    } \
} ()

#ifdef CASPER_APP_WATCHDOG_UNLOCK
    #undef CASPER_APP_WATCHDOG_UNLOCK
#endif
#define CASPER_APP_WATCHDOG_UNLOCK(a_function, a_line) [&]() { \
    if ( true == detached_ ) { \
        CASPER_APP_LOG("status", "%s:%d %s", a_function, a_line, "unlock"); \
        if ( false ) assert(1 == locks_.fetch_sub(1)); \
        mutex_.unlock(); \
    }\
} ()

namespace casper
{
    
    namespace app
    {
        
        namespace monitor
        {
                        
            // ---- //
            
            class Watchdog;
            
            class Initializer final : public ::cc::Initializer<Watchdog>
            {
                
            public: // Constructor(s) / Destructor
                
                Initializer (Watchdog& a_watchdog);
                virtual ~Initializer ();
                
            };
            
            // ---- //
            class Watchdog final : public cc::Singleton<Watchdog, Initializer>
            {
                
                friend class Initializer;
                
            public: // Data Type(s)

                class Listener
                {
                    
                public: // Constructor(s) / Destructor
                    
                    /**
                     * @brief Destructor.
                     */
                    virtual ~Listener ()
                    {
                        /* empty */
                    }
                    
                public: // API Pure Virtual Method(s) / Function(s)
                    
                    virtual void OnRunningProcessesUpdated (const std::list<::sys::Process*>& a_list       ) = 0;
                    virtual void OnError                   (const ::sys::Error& a_error, const bool a_fatal) = 0;
                    virtual void OnTerminated              ()                                                = 0;
                    
                }; // end of class 'Listener'
                
            private: // Ptrs

                Listener* listener_ptr_; //!< NOT MANAGED BY THIS CLASS
                
            private: // Data
                
                std::list<::sys::Process*> list_;
                ::sys::Error               last_error_;
                
            private: // Threading
                
                std::thread*            thread_;
                std::mutex              mutex_;
                std::atomic<bool>       detached_;
                std::atomic<int>        locks_;
                bool volatile*          abort_flag_;
                osal::ConditionVariable thread_cv_;
                pid_t                   main_pid_;
                
            public: // Method(s) / Function(s)
                
                void        Start     (const Json::Value& a_config, const bool a_detached, Listener& a_listener,
                                       bool volatile* a_abort_flag);
                void        Stop      ();
                void        Quit      ();
                void        Refresh   ();
            
            public: // Inline Method(s) / Function(s)
                
                bool        IsErrorSet ();
                void        GetError   (const std::function<void(const ::sys::Error& a_last_error)>& a_callback);
                
            private: // Method(s) / Function(s)

                bool Start             (const std::list<::sys::Process::Info>& a_list, const bool a_detached, Listener& a_listener,
                                        bool volatile* a_abort_flag);
                void Loop              ();
                
                bool KillAll           (const bool& a_optional, const pid_t a_parent_pid = 1);
                bool TerminateAll      (const bool& a_optional, const pid_t a_parent_pid = 1);
                bool SignalAll         (const int a_no, const bool& a_optional, const pid_t a_parent_pid = 1);
                
                bool Spawn             (::sys::Process& a_process, const bool& a_block_sigchld);
                bool Exec              (::sys::Process& a_process);
                
                bool MKDIR              (const ::sys::Process* a_process, const std::string& a_directory);
                bool EnsureRequirements (const ::sys::Process& a_process);
                bool Redirect           (const ::sys::Process& a_process,
                                         const std::list<std::pair<FILE*, std::string>>& a_list);

            private: // Static Method(s) / Function(s)
                
                static void OnSignal (int a_signal_no);
                
            private:
                
                bool IsErrorSetUnsafe   () const;
                void Bite               (const bool a_fatal = false);
                void BiteUnsafe         (const bool a_fatal = false);
                void Bark               (const sys::Error& a_error, FILE* a_stream = nullptr) const;
                void Notify             (const int a_sigal_no);
                
            public: // Static Method(s) / Function(s)
                
                static int KillSignalForProcess (const std::string& a_name);
                static int ReadPIDFromFile      (const std::string& a_uri, pid_t& o_pid);
                
            }; // end of class 'Watchdog'
            
            /**
             * @return True if an error is set, false otherwise.
             */
            inline bool Watchdog::IsErrorSet ()
            {
                CASPER_APP_WATCHDOG_LOCK(__PRETTY_FUNCTION__, __LINE__);
                const bool rv = IsErrorSetUnsafe();
                CASPER_APP_WATCHDOG_UNLOCK(__PRETTY_FUNCTION__, __LINE__);
                return rv;
            }
            
            /**
             * @brief Call a function to access RO error data.
             *
             * @param a_callback The function to call.
             */
            inline void Watchdog::GetError (const std::function<void(const ::sys::Error& a_last_error)>& a_callback)
            {
                CASPER_APP_WATCHDOG_LOCK(__PRETTY_FUNCTION__, __LINE__);
                try {
                    a_callback(last_error_);
                    CASPER_APP_WATCHDOG_UNLOCK(__PRETTY_FUNCTION__, __LINE__);
                } catch (...) {
                    CASPER_APP_WATCHDOG_UNLOCK(__PRETTY_FUNCTION__, __LINE__);
                    std::rethrow_exception(std::current_exception());
                }
            }
            
            /**
             * @return True if an error is set, false otherwise.
             */
            inline bool Watchdog::IsErrorSetUnsafe () const
            {
                return ( 0 != last_error_.message().length() );
            }
            
            /**
             * @brief Throw an exception if an error is set.
             *
             * @param a_fatal When true, a callback ( if set ) will be called instead of throwing an exception.
             *                If no callback is set, the error is logged and exit is called.
             */
            inline void Watchdog::Bite (const bool a_fatal)
            {
                CASPER_APP_WATCHDOG_LOCK(__PRETTY_FUNCTION__, __LINE__);
                try {
                    BiteUnsafe(a_fatal);
                    CASPER_APP_WATCHDOG_UNLOCK(__PRETTY_FUNCTION__, __LINE__);
                } catch (...) {
                    CASPER_APP_WATCHDOG_UNLOCK(__PRETTY_FUNCTION__, __LINE__);
                    std::rethrow_exception(std::current_exception());
                }
            }
            
            /**
             * @brief Throw an exception if an error is set.
             *
             * @param a_fatal When true, a callback ( if set ) will be called instead of throwing an exception.
             *                If no callback is set, the error is logged and exit is called.
             */
            inline void Watchdog::BiteUnsafe (const bool a_fatal) {
                if ( 0 == last_error_.message().length() ) {
                    return;
                }
                Bark(last_error_);
                if ( nullptr != listener_ptr_ ) {
                    listener_ptr_->OnError(last_error_, a_fatal);
                } else {
                    if ( 0 != last_error_.message().length() ) {
                        throw ::cc::Exception("%s", last_error_.message().c_str());
                    }
                }
            }
            
            /**
             * @brief Log an error message.
             *
             * @param a_error  The error to log.
             * @param a_stream The stream to write to.
             */
            inline void Watchdog::Bark (const sys::Error& a_error, FILE* a_stream) const
            {
                if ( nullptr != a_stream ) {
                    fprintf(a_stream, "------ [B] %s ------\n", "ERROR");
                    fprintf(a_stream, "%s:%d\n", a_error.function(), a_error.line());
                    fprintf(a_stream, "message: %s\n", a_error.message().c_str());
                    if ( sys::Error::k_no_error_ != a_error.no() ) {
                        fprintf(a_stream, "system: %s\n", a_error.str().c_str());
                    }
                    fprintf(a_stream, "------ [E] %s ------\n", "ERROR");
                } else {
                    CASPER_APP_LOG("error", "------ [B] %s ------", "ERROR");
                    CASPER_APP_LOG("error", "%s:%d", a_error.function(), a_error.line());
                    CASPER_APP_LOG("error", "message: %s", a_error.message().c_str());
                    if ( sys::Error::k_no_error_ != a_error.no() ) {
                        CASPER_APP_LOG("error", "system: %s", a_error.str().c_str());
                    }
                    CASPER_APP_LOG("error", "------ [E] %s ------", "ERROR");
                }
            }

            /**
             * @brief Notify listener about processes list changes.
             *
             * @param a_signal_no The signal number.
             */
            inline void casper::app::monitor::Watchdog::Notify (const int a_signal_no)
            {
                CASPER_APP_WATCHDOG_LOCK(__PRETTY_FUNCTION__, __LINE__);
                try {
                    if ( nullptr != listener_ptr_ ) {
                        if ( SIGTERM == a_signal_no ) {
                            listener_ptr_->OnTerminated();
                        } else if ( SIGUSR2 == a_signal_no ) {
                            listener_ptr_->OnRunningProcessesUpdated(list_);
                        }
                    }
                    CASPER_APP_WATCHDOG_UNLOCK(__PRETTY_FUNCTION__, __LINE__);
                } catch (...) {
                    CASPER_APP_WATCHDOG_UNLOCK(__PRETTY_FUNCTION__, __LINE__);
                    std::rethrow_exception(std::current_exception());
                }
            }
                
            inline int Watchdog::KillSignalForProcess (const std::string& a_name)
            {
                if ( nullptr != strcasestr(a_name.c_str(), "postgres") ) {
                    return SIGTERM;
                } else {
                    return SIGKILL;
                }
            }

            inline int Watchdog::ReadPIDFromFile (const std::string& a_uri, pid_t& o_pid)
            {
                std::ifstream stream(a_uri);
                if ( true == stream.is_open() ) {
                    int pid = 0;
                    while ( stream >> pid ) {
                        o_pid = static_cast<pid_t>(pid);
                        break;
                    }
                    stream.close();
                } else {
                    return EPERM;
                }
                return 0;
            }

            
        } // end of namespace 'monitor'
        
    } // end of namespace 'app'
    
} // end of namespace 'casper'

#endif // CASPER_APP_MONITOR_WATCHDOG_H_

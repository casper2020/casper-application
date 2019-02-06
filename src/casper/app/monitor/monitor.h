/**
 * @file monitor.h
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

#ifndef CASPER_APP_MONITOR_H_
#define CASPER_APP_MONITOR_H_
#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>
#include <stdlib.h> // malloc, free
#include <string.h> // strdup
#include <thread>
#include <mutex>

#include "casper/app/monitor/singleton.h"
#include "osal/condition_variable.h"

#include "casper/app/monitor/process.h"

namespace casper
{
    
    namespace app
    {
        
        namespace monitor
        {
                        
            // ---- //
            
            class Minimalist;
            
            class Initializer final : public ::cc::Initializer<Minimalist>
            {
                
            public: // Constructor(s) / Destructor
                
                Initializer (Minimalist& a_minimalist);
                virtual ~Initializer ();
                
            };
            
            // ---- //
            class Minimalist final : public cc::Singleton<Minimalist, Initializer>
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
                    
                    virtual void OnRunningProcessesUpdated (const Process::List& a_list) = 0;
                    
                }; // end of class 'Listener'
                
            public: // Data Type(s)
                
                typedef struct  {
                    const std::string& data_directory_;
                    const std::string& arguments_;
                } PostgreSQL;
                
                typedef struct {
                    const std::string& runtime_directory_;
                    const std::string& resources_directory_;
                    const std::string& working_directory_;
                    const std::string& logs_directory_;
                    const std::string& app_directory_prefix_;
                    const std::string& config_directory_prefix_;
                    const PostgreSQL&  postgresql_;
                } Settings;
                
            private: // Ptrs

                Listener* listener_ptr_; //!< NOT MANAGED BY THIS CLASS
                
            private: // Data
                
                Process::List list_;
                Error         last_error_;
                
            private: // Threading
                
                std::thread*            thread_;
                std::mutex              mutex_;
                osal::ConditionVariable thread_cv_;
                
            public: // Method(s) / Function(s)
                
                bool        Start     (Listener* a_listener, const Settings& a_settings);
                void        Stop      ();
                void        Notify    ();
                
            private: // Method(s) / Function(s)
                
                bool Start             (const std::list<const Process::Info>& a_list, Listener* a_listener);
                void Loop              ();
                
                bool KillAll           (const bool& a_optional);
                bool TerminateAll      (const bool& a_optional);
                bool SignalAll         (const int a_no, const bool& a_optional);
                
                bool Spawn             (Process& a_process, const bool& a_block_sigchld);
                bool Exec              (Process& a_process, sigset_t& a_sigmask);
                
                bool MKDIR              (const Process* a_process, const std::string& a_directory);
                bool EnsureRequirements (const Process& a_process);
                bool Redirect           (const Process& a_process,
                                         const std::list<std::pair<FILE*, std::string>>& a_list);

            private: // Static Method(s) / Function(s)
                
                static void Log (const Error& a_error);
                
                static void USR1 (int a_signal);
                static void USR2 (int a_signal);
                
            public:
                
                bool         IsErrorSet () const;
                const Error* LastError  () const;
                
            }; // end of class 'Minimalist'
            
            /**
             * return True if an error is set, false otherwise.
             */
            inline bool Minimalist::IsErrorSet () const
            {
                return ( 0 != last_error_.message().length() );
            }
            
            /**
             * @return \link Error \link, null if no error is set.
             */
            inline const Error* Minimalist::LastError () const
            {
                if ( true == IsErrorSet() ) {
                    return &last_error_;
                }
                return nullptr;
            }
            
        } // end of namespace 'monitor'
        
    } // end of namespace 'app'
    
} // end of namespace 'casper'

#endif // CASPER_APP_MONITOR_H_

/**
 * @file monitor.h
 *
 * Copyright (c) 2010-2017 Neto Ranito & Seabra LDA. All rights reserved.
 *
 * This file is part of casper.
 *
 * casper is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * casper  is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with casper.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CASPER_APP_MAC_MONITOR_H_
#define CASPER_APP_MAC_MONITOR_H_

#pragma once

#include "cc/singleton.h"

#include "sys/process.h"

#import "casper/mac/app_delegate.h"

#include "json/json.h"

#include "osal/condition_variable.h"


#include <mutex>
#include <list>

namespace casper
{
    
    namespace app
    {
        
        namespace mac
        {
            
            // ---- //
            class Monitor;
            class MonitorInitializer final : public ::cc::Initializer<Monitor>
            {
                
            public: // Constructor(s) / Destructor
                
                MonitorInitializer (Monitor& a_instance);
                virtual ~MonitorInitializer ();
                
            };
            
            // ---- //
            class Monitor final : public cc::Singleton<Monitor, MonitorInitializer>
            {
                
                friend class MonitorInitializer;
                
            public: // Data Type(s)
                
                typedef std::function<void()>                         DispatchCallback;
                typedef std::function<AppDelegate*(DispatchCallback)> BindCallback;
                typedef std::function<void(const Json::Value&)>       QuitCallback;
                
            private: // Threading
                
                std::mutex              mutex_;
                std::list<Json::Value>  messages_;
                sys::Process*           process_;

            private: // Data
                
                AppDelegate*     app_delegate_;
                DispatchCallback main_thread_dispatcher_;
                Json::Value      rc_object_;
                QuitCallback     quit_callback_;
                
            public: // Method(s) / Function(s)
                
                void Start (const Json::Value& a_config, BindCallback a_bind_callback, DispatchCallback a_dispatch_callback,
                            QuitCallback a_quit_callback);
                void Stop  (bool a_soft);
                
            private: // Method(s) / Function(s)
                
                void ProcessReceivedMessages ();
                
            }; // end of class 'Monitor'
            
        } // end of namespace 'mac'
            
    } // end of namespace 'app'
    
} // end of namespace 'casper'

#endif // CASPER_APP_MAC_MONITOR_H_

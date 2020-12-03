/**
 * @file monitor.cc
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

#include <unistd.h> // getopt

#include "casper/app/monitor/version.h"

#include <string>
#include <map>

#include "casper/app/monitor/watchdog.h"
#include "cc/sockets/dgram/ipc/client.h"
#include "cc/sockets/dgram/ipc/server.h"

#include "cc/exception.h"

#include "json/json.h"

#include "cc/b64.h"

#include "ev/signals.h"

#include "casper/app/logger.h"

#include <signal.h>

/**
 * @brief Show version.
 *
 * @param a_name Tool name.
 */
void show_version (const char* a_name)
{
    fprintf(stderr, "%s\n", CASPER_MONITOR_INFO);
}

/**
 * @brief Show help.
 *
 * @param a_name Tool name.
 */
static void show_help (const char* a_name)
{
    fprintf(stderr, "usage: %s -c <configuration file>\n", a_name);
    fprintf(stderr, "       -%c: %s\n", 'c' , "configuration file.");
    fprintf(stderr, "       -%c: %s\n", 'd' , "register debug token");
    fprintf(stderr, "       -%c: %s\n", 'h' , "show help.");
    fprintf(stderr, "       -%c: %s\n", 'v' , "show version.");
}

/**
 * @brief 'monitor' process entry point
 *
 * @param a_argc
 * @parma a_arvg
 */
int main (int a_argc, char* a_argv[])
{
    //
    // -c B64 of a JSON string with required configuration
    // -d register debug token
    // -h display help
    // -v display version
    //
    CASPER_APP_LOG("status", "%s", "Starting 'monitor'...");
    
    Json::Value config;

    // ... parse arguments ...
    char opt;
    while ( -1 != ( opt = getopt(a_argc, a_argv, "hvc:") ) ) {
        switch (opt) {
            case 'h':
                show_help(a_argv[0]);
                return 0;
            case 'v':
                show_version(a_argv[0]);
                return 0;
            case 'c':
            {
                Json::Reader reader;
                
                const std::string b64_config  = optarg;
                const std::string json_config = cc::base64_url_unpadded::decode<std::string>(b64_config.c_str(), b64_config.length());
                
                if ( false == reader.parse(json_config, config) ) {
                    const auto errors = reader.getStructuredErrors();
                    if ( errors.size() > 0 ) {
                        fprintf(stderr, "An error occurred while parsing configuration JSON: %s!",
                                errors[0].message.c_str()
                        );
                    } else {
                        fprintf(stderr, "An error occurred while parsing configuration JSON!");
                    }
                    return -1;
                }
            }
                break;
            default:
                fprintf(stderr, "llegal option %s:\n", optarg);
                show_help(a_argv[0]);
                return -1;
        }
    }
    
    a_argc -= optind;
    a_argv += optind;
    
    if ( true == config.isNull() || false == config.isObject() ) {
        fprintf(stderr, "missing or invalid argument value for -c option!\n");
        return -1;
    }
    
    class Listener final : public casper::app::monitor::Watchdog::Listener
    {
        
    private: // Data
        
        Json::Value   data_;
        size_t        sigterm_count_;
        
    public: // Flags
        
        volatile bool abort_flag_;
        
    public: // Constructor(s) / Destructor
        
        /**
         * @brief Default constructor.
         */
        Listener ()
        {
            sigterm_count_ = 0;
            abort_flag_    = false;
        }
        
        /**
         * @brief Destructor.
         */
        virtual ~Listener ()
        {
            /* empty */
        }
        
    public: // API Inherited Pure Virtual Method(s) / Function(s)
        
        virtual void OnRunningProcessesUpdated (const std::list<::sys::Process*>& a_list)
        {
            data_         = Json::Value(Json::ValueType::objectValue);
            data_["type"] = "list";
            
            Json::Value array = Json::Value(Json::ValueType::arrayValue);
            
            Json::Value& element = array.append(Json::Value(Json::ValueType::objectValue));
            element["id" ] = "monitor";
            element["pid"] = getpid();

            for ( auto process : a_list ) {
                Json::Value& element = array.append(Json::Value(Json::ValueType::objectValue));
                element["id" ] = process->info().id_;
                element["pid"] = process->pid();
            }
            
            data_["list"] = array;
            
            try {
                cc::sockets::dgram::ipc::Client::GetInstance().Send(data_);
            } catch (const ::cc::Exception& a_cc_exception) {
                CASPER_APP_LOG("error", "%s", a_cc_exception.what());
            }
        }
        
        virtual void OnError (const sys::Error& a_error, const bool a_fatal)
        {
            data_         = Json::Value(Json::ValueType::objectValue);
            data_["type"] = "error";
            
            data_["error"]["no"]    = a_error.no();
            data_["error"]["str"]   = a_error.str();
            data_["error"]["msg"]   = a_error.message();
            data_["error"]["fnc"]   = a_error.function();
            data_["error"]["ln"]    = a_error.line();
            data_["error"]["fatal"] = a_fatal;
            
            cc::sockets::dgram::ipc::Client::GetInstance().Send(data_);
        }
        
        virtual void OnTerminated ()
        {
            fprintf(stdout, "monitor: sigterm_count_=%d\n", (int)sigterm_count_);
            fflush(stdout);
//            if ( 0 == sigterm_count_++ ) {
//                data_           = Json::Value(Json::ValueType::objectValue);
//                data_["type"]   = "status";
//                data_["status"] = "terminated";
//                cc::sockets::dgram::ipc::Client::GetInstance().Send(data_);
//            } else {
//                abort_flag_ = true;
//            }
            abort_flag_ = true;
        }
        
    }; // end of class 'Listener'
    
    int rv = -1;

    // {
    //     "directories": {
    //         "runtime": "",
    //         "working": "",
    //         "config" : "",
    //         "logs": ""
    //     },
    //     "variables": {
    //         "common": {
    //              "<key>": "<value>"
    //         },
    //         "postgresql": {
    //             "<key>": "<value>"
    //         }
    //     }
    // }
    
#ifdef __APPLE__
    signal(SIGPIPE, SIG_IGN);
#endif
    
    try {
        
        const Json::Value& directories = config["directories"];

        // ... start logger ...
        ::casper::app::Logger::GetInstance().Startup(directories["logs"].asString(), "monitor", CASPER_MONITOR_VERSION);
                
        Listener                listener;
        osal::ConditionVariable start_cv;
        
        const auto on_fatal_exception = [&start_cv] (const ::cc::Exception& a_cc_exception) {
            // ... failure ...
            CASPER_APP_LOG("error", "%s", a_cc_exception.what());
            // ... release start condition ( if any ) ...
            start_cv.Wake();
        };
        
        class Dummy {
            
        };
        Dummy owner;
        
        ::ev::Loggable::Data* loggable_data_ = new ::ev::Loggable::Data(
                /* owner_ptr_ */ &owner,
                /* ip_addr_   */ "127.0.0.1",
                /* module_    */ "monitor",
                /* tag_       */ ""
        );
        ::ev::Signals::GetInstance().WarmUp(*loggable_data_);

        //
        // ... signal handing ...
        //
        ::ev::Signals::GetInstance().Startup(
               /* a_signals */
               { SIGUSR1, SIGTERM, SIGQUIT, SIGTTIN },
               /* a_callbacks */
               {
                   /* on_signal_           */ [](const int a_sig_no) {
                       // ... is a 'shutdown' signal?
                       switch(a_sig_no) {
                           case SIGQUIT:
                           case SIGTERM:
                           {
                               casper::app::monitor::Watchdog::GetInstance().Quit();
                           }
                               return true;
                           default:
                               return false;
                       }
                   },
                   /* on_fatal_exception_  */ on_fatal_exception,
                   /* call_on_main_thread_ */ [] (std::function<void()> a_callback) {
                       a_callback();
                   }
               }
        ); 
        
        // ( on error, an exception will be thrown )
        cc::sockets::dgram::ipc::Server::GetInstance().Start("monitor", directories["runtime"].asString(),
                                                               {
                                                                   /* on_message_received_ */
                                                                   [&start_cv] (const Json::Value& a_value) {
                                                                       try {
                                                                           const Json::Value& type       = a_value["type"];
                                                                           const char* const  type_c_str = type.asCString();
                                                                           if ( 0 == strcasecmp("control", type_c_str) ) {
                                                                               const char* const control_c_str = a_value[type_c_str].asCString();
                                                                               if ( 0 == strcasecmp("start", control_c_str) ) {
                                                                                   start_cv.Wake();
                                                                               } else if ( 0 == strcasecmp("refresh", control_c_str) ) {
                                                                                   casper::app::monitor::Watchdog::GetInstance().Refresh();
                                                                               } else if ( 0 == strcasecmp("stop", control_c_str) ) {
                                                                                   casper::app::monitor::Watchdog::GetInstance().Stop();
                                                                               }
                                                                           }
                                                                           
                                                                       } catch (const Json::Exception& a_json_exception) {
                                                                           // ... failure ...
                                                                           CASPER_APP_LOG("error", "%s", a_json_exception.what());
                                                                       }
                                                                   },
                                                                   /* on_terminated_       */ [&start_cv] () {
                                                                       // ... release start condition ( if any ) ...
                                                                       start_cv.Wake();
                                                                   },
                                                                   /* on_fatal_exception_  */ on_fatal_exception
                                                               }
        );

        Json::Value status = Json::Value(Json::ValueType::objectValue);
        status["type"]   = "status";

        // ... start a unidirectional message channel to send messages to parent process ...
        // ( on error, an exception will be thrown )
        cc::sockets::dgram::ipc::Client::GetInstance().Start("casper", directories["runtime"].asString());
        
        status["status"] = "started";
        cc::sockets::dgram::ipc::Client::GetInstance().Send(status);

        // ... wait for parent process order ...
        CASPER_APP_LOG("status", "%s", "Waiting for monitor's parent...");
        start_cv.Wait();
        
        // ... start monitoring process(es) ...
        // ( on error, an exception will be thrown )
        
        casper::app::monitor::Watchdog& watchdog = casper::app::monitor::Watchdog::GetInstance();
        
        CASPER_APP_LOG("status", "%s", "Resuming monitor...");
        watchdog.Start(config, /* a_detached */ false, listener, &listener.abort_flag_);
        
        status["status"] = "terminated";
        if ( true == watchdog.IsErrorSet() ) {
            watchdog.GetError([&status] (const ::sys::Error& a_error) {
                status["error"]        = Json::Value(Json::ValueType::objectValue);
                status["error"]["no"]  = a_error.no();
                status["error"]["str"] = a_error.str();
                status["error"]["msg"] = a_error.message();
                status["error"]["fnc"] = a_error.function();
                status["error"]["ln"]  = a_error.line();
            });
        }
        
        cc::sockets::dgram::ipc::Client::GetInstance().Send(status);

        CASPER_APP_LOG("status", "%s", "Cleanup...");

        usleep(2000*1000);

        CASPER_APP_LOG("status", "%s", "Monitor stopped...");
        
        // ... success ...
        rv = 0;
        
    } catch (const ::cc::Exception& a_cc_exception) {
        // ... failure ...
        CASPER_APP_LOG("error", "%s", a_cc_exception.what());
        fprintf(stderr, "%s\n", a_cc_exception.what());
        fflush(stderr);
    } catch (const Json::Exception& a_json_exception) {
        // ... failure ...
        CASPER_APP_LOG("error", "%s", a_json_exception.what());
    }
    
    // ... done ...
    return rv;
}

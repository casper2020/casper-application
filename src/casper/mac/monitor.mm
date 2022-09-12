/**
 * @file Monitor.mm
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

#include "casper/mac/monitor.h"

#include "cc/sockets/dgram/ipc/client.h"
#include "cc/sockets/dgram/ipc/server.h"

#include "casper/app/monitor/watchdog.h"

#include "cc/utc_time.h"

#include "sys/darwin/process.h"

#include "osal/osal_file.h"

#ifdef __APPLE__
#pragma mark - MonitorInitializer
#endif

/**
 * @brief This method will be called when it's time to initialize this singleton.
 *
 * @param a_instance A referece to the owner of this class.
 */
casper::app::mac::MonitorInitializer::MonitorInitializer (casper::app::mac::Monitor& a_instance)
    : ::cc::Initializer<casper::app::mac::Monitor>(a_instance)
{
    instance_.app_delegate_           = nullptr;
    instance_.main_thread_dispatcher_ = nullptr;
    instance_.process_                = nullptr;
    instance_.rc_object_              = Json::Value(Json::ValueType::objectValue);
    instance_.rc_object_["type"]      = "control";
    instance_.rc_object_["control"]   = "refresh";
}

/**
 * @brief Destructor.
 */
casper::app::mac::MonitorInitializer::~MonitorInitializer ()
{
    instance_.Stop(/* a_soft */ false);
    if ( nullptr != instance_.process_ ) {
        delete instance_.process_;
    }
    instance_.app_delegate_           = nullptr;
    instance_.main_thread_dispatcher_ = nullptr;
}

#ifdef __APPLE__
#pragma mark - Monitor
#endif

/**
 * @brief Start this singleton inter process comunication.
 *
 * @param a_config
 * @param a_bind_callback
 * @param a_dispatch_callback
 * @param a_quit_callback
 */
void casper::app::mac::Monitor::Start (const Json::Value& a_config,
                                       casper::app::mac::Monitor::BindCallback a_bind_callback,
                                       casper::app::mac::Monitor::DispatchCallback a_dispatch_callback,
                                       casper::app::mac::Monitor::QuitCallback a_quit_callback)
{
    Stop(/* a_soft */ false);
    
    main_thread_dispatcher_ = a_dispatch_callback;
    app_delegate_           = a_bind_callback(std::bind(&casper::app::mac::Monitor::ProcessReceivedMessages, this));
    quit_callback_          = a_quit_callback;
    
    const Json::Value& directories = a_config["directories"];
    
    const std::string runtime_dir = directories["runtime"].asString();
    const std::string logs_dir    = directories["logs"].asString();

    // ... start a unidirectional message channel to send messages to parent process ...
    // ( on error, an exception will be thrown )
    cc::sockets::dgram::ipc::Server::GetInstance().Start("casper", runtime_dir,
                                                           {
                                                               /* on_message_received_ */
                                                               [this] (const Json::Value& a_value) {
                                                                   // ... on this callback message must be handled ...
                                                                   std::lock_guard<std::mutex> lock(mutex_);
                                                                   messages_.push_back(a_value);
                                                                   main_thread_dispatcher_();
                                                               },
                                                               /* on_terminated_ */
                                                               [] () {
                                                                   // ... just an 'info' callback, no action required ...
                                                                   fprintf(stderr, "casper-application: monitor terminated\n");
                                                                   fflush(stderr);
                                                               },
                                                               /* on_fatal_exception_ */
                                                               [] (const ::cc::Exception& a_cc_exception) {
                                                                   // ... just an 'info' callback, no action required ...
                                                                   fprintf(stderr, "casper-application: %s\n", a_cc_exception.what());
                                                                   fflush(stderr);
                                                               }
                                                           }
    );
    
    
    // ... start a unidirectional message channel to send messages to 'monitor' process ...
    // ( on error, an exception will be thrown )
    cc::sockets::dgram::ipc::Client::GetInstance().Start("monitor", runtime_dir, /* a_standalone */ false);

    process_ = new ::sys::darwin::Process(::sys::Process::Info({
        /* id_          */ "monitor",
        /* owner_       */ "casper",
        /* path_        */ "",
        /* executable_  */ a_config["monitor"]["path"].asString(),
        /* arguments_   */ "",
        /* user_        */ "",
        /* group_       */ "",
        /* working_dir_ */ "",
        /* log_dir_     */ "",
        /* pid_file_    */ runtime_dir + "monitor.pid",
        /* depends_on_  */ {}
    }));
    
    // ... first kill previous 'monitor' processs ( if any ) ...
    if ( true == process_->LoadPIDFromFile(/* a_optional */ true) ) {
        bool is_zombie, is_running = false;
        if ( ( true == process_->IsRunning(/* a_optional */ true, /* a_parent_pid */ 1, is_running) && true == is_running )
              ||
             ( true == process_->IsZombie(/* a_optional */ true, is_zombie) && true == is_zombie )
        ) {
            if ( false == process_->Kill(/* a_optional */ false) ) {
                throw ::cc::Exception(process_->error().message());
            }
        }
        process_->UnlinkPID(/* a_optional */ false);
    }

    // ... start new 'monitor' process ..
    [app_delegate_ startProcess: a_config
         notifyWhenStarted:^(pid_t a_pid) {
             
             (*process_) = a_pid;
             process_->WritePID();
             
             cc::sockets::dgram::ipc::Server::GetInstance().Schedule([this]() {
                 std::lock_guard<std::mutex> lock(mutex_);
                 messages_.push_back(rc_object_);
                 main_thread_dispatcher_();
             }, /* a_timeout_ms */ 5000, /* a_recurrent */ true);
             
         }
        andWhenFinished:^(int a_code, Json::Value a_error) {
            if ( not ( YES == [app_delegate_ shouldQuit] || YES == [app_delegate_ shouldRelaunch ] ) ) {
                // ... possible crash?
                if ( EXIT_SUCCESS == a_code ) {
                    a_code = EXIT_FAILURE;
                    if ( true == a_error.isNull() ) {
                        a_error              = Json::Value(Json::ValueType::objectValue);
                        a_error["msg"]       = "Abnormal 'monitor' process termination - killed by external signal sent illegally?";
                        a_error["terminate"] = true;
                    }
                }
                
                class FindCallback : public ::osal::File::FindCallback
                {
                    
                private: // Const Data
                    
                    const std::string logs_dir_;
                    FILE*             log_file_;
                    
                public: // Constructor(s) / Destructor
                    
                    FindCallback () = delete;
                    FindCallback (const std::string& a_error, const std::string& a_logs_dir)
                     : logs_dir_(a_logs_dir)
                    {
                        log_file_ = fopen((logs_dir_ + "/monitor.log").c_str(), "w+");
                        if ( nullptr != log_file_ ) {
                            fprintf(log_file_, "--- casper-monitor: %s ---\n", ::cc::UTCTime::NowISO8601DateTime().c_str());
                            fprintf(log_file_, "%s\n", a_error.c_str());
                            fflush(log_file_);
                        }
                    }
                    
                    virtual ~FindCallback ()
                    {
                        if ( nullptr != log_file_ ) {
                            fflush(log_file_);
                            fclose(log_file_);
                        }
                    }
                    
                public: // Inherited Vrtual Method(s) / Funciton(s)
                    
                    virtual bool OnNewFileEntry (const char* a_dir_name, const char* a_file_name)
                    {
                        pid_t pid = 0;
                        try {
                            const std::string uri = a_dir_name + std::string(a_file_name);
                            if ( false == osal::File::Exists(uri.c_str()) ) {
                                // ... next ...
                                return true;
                            }
                            const std::string name = std::string(a_file_name, strlen(a_file_name) - 4);
                            const int         signo = casper::app::monitor::Watchdog::KillSignalForProcess(name);
                            // ... try to kill it ...
                            const int eno = casper::app::monitor::Watchdog::ReadPIDFromFile(uri, pid);
                            if ( 0 == eno ) {
                                if ( 0 != kill(pid, signo) && nullptr != log_file_ ) {
                                    if ( nullptr == strcasestr(a_file_name, "monitor") ) {
                                        fprintf(log_file_, "%-26s: %-7s: unable to send signal %d to process with pid %d - (%d) : %s\n", name.c_str(), "WARNING", signo, pid, eno, strerror(eno));
                                    }
                                } else if ( nullptr != log_file_ ) {
                                    fprintf(log_file_, "%-26s: %-7s: signal %d sent to process with pid %d\n", name.c_str(), "WARNING", signo, pid);
                                }
                            } else {
                                fprintf(log_file_, "%-26s: %-7s: unable to obtain process pid %d - (%d) : %s\n", name.c_str(), "WARNING", pid, eno, strerror(eno));
                            }
                            fflush(log_file_);
                            (void)osal::File::Delete(uri.c_str());
                        } catch (...) {
                            try {
                                ::cc::Exception::Rethrow(/* a_unhandled */ false, __FILE__, __LINE__, __FUNCTION__);
                            } catch (const ::cc::Exception& a_cc_exception) {
                                if ( nullptr != log_file_ ) {
                                    fprintf(log_file_, "%-26s: %-7s: unable to kill this child process - %s\n", a_file_name, "WARNING", a_cc_exception.what());
                                    fflush(log_file_);
                                }
                            }
                        }
                        // ... next ...
                        return true;
                    }
                    
                };
                
                if ( EXIT_SUCCESS != a_code ) {
                    // ... search for all child processes ( via pid file ) ...
                    FindCallback c(a_error["msg"].asString(), logs_dir);
                    // ... kill all child processes ...
                    ::osal::File::FindRecursive(runtime_dir.c_str(), {"*.pid"}, &c);
                    // ... stop monitoring ...
                    cc::sockets::dgram::ipc::Server::GetInstance().Stop(SIGQUIT);
                }
            }

            if ( nullptr != process_ ) {
                (void)process_->UnlinkPID(/* a_optional */ true);
                delete process_;
                process_ = nullptr;
            }

            if ( EXIT_SUCCESS != a_code ) {
                
                Json::Value message = Json::Value(Json::ValueType::objectValue);
                message["type"]  = "error";
                message["error"] = a_error;
                
                std::lock_guard<std::mutex> lock(mutex_);
                messages_.push_back(message);
                main_thread_dispatcher_();
                                
            }
         }
    ];
    
}

/**
 * @brief Stop this singleton inter process comunication.
 *
 * @param a_soft When false, it's a full stop.
 */
void casper::app::mac::Monitor::Stop (bool a_soft)
{
    if ( nullptr != process_ ) {
        process_->Terminate(/* a_optional */ false);
    }
    if ( false == a_soft ) {
        cc::sockets::dgram::ipc::Server::GetInstance().Stop(SIGQUIT);
    } else {
        cc::sockets::dgram::ipc::Client::GetInstance().Stop(SIGQUIT);
    }
}

/**
 * @brief Process received messages;
 */
void casper::app::mac::Monitor::ProcessReceivedMessages ()
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    cc::sockets::dgram::ipc::Client& client = cc::sockets::dgram::ipc::Client::GetInstance();
    
    while ( messages_.size() > 0 ) {

        const Json::Value message     = messages_.front();
        const Json::Value& type       = message["type"];
        const char* const  type_c_str = type.asCString();
        const Json::Value& data       = message[type_c_str];
        
        if ( 0 == strcasecmp("list", type_c_str) ) {
            
            [app_delegate_ setRunningProcesses: data];
            
        } else if ( 0 == strcasecmp("error", type_c_str) ) {
            
            [app_delegate_ showError: data andRelaunch: YES];
            
        } else if ( 0 == strcasecmp("status", type_c_str) ) {

            if ( 0 == strcasecmp("started", data.asCString()) ) {
                Json::Value msg = Json::Value(Json::ValueType::objectValue);
                msg["type"]    = "control";
                msg["control"] = "start";
                client.Send(msg);
            } else if ( 0 == strcasecmp("terminated", data.asCString()) ) {
                client.Stop(SIGQUIT);
                quit_callback_(message.get("error", Json::Value::null));
            }
            
        } else if ( 0 == strcasecmp("control", type_c_str) ) {
            try {
                if ( true == client.IsReady() ) {
                    client.Send(rc_object_);
                }
            } catch (...) {
                // ... process is shutting down ...
            }
        }
    
        messages_.pop_front();
    }
    
}

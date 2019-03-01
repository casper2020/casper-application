/**
 * @file Watchdog.cc
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

#include "casper/app/monitor/watchdog.h"

#include "casper/app/monitor/helper.h"

#include <unistd.h> // access, pid_t, getppid
#include <errno.h>  // errno
#include <signal.h> // sigemptyset, sigaddset, pthread_sigmask, etc
#include <sstream>  // stringstream
#include <inttypes.h> // PRId32
#include <sys/stat.h> //fstat

#include <sys/event.h> // kevent
#include <sys/select.h> // select

#include <grp.h> // getgrgid

#include <set>     // std::set

#include <cstdarg> // va_start, va_end, std::va_list

#include <spawn.h>

#include "json/json.h"
#include <iostream> // std::istream, std::ios
#include <fstream>  // std::filebuf

#include <fcntl.h>

#ifdef CASPER_APP_WATCHDOG_LOCK
    #undef CASPER_APP_WATCHDOG_LOCK
#endif
#define CASPER_APP_WATCHDOG_LOCK() [&] () { \
    assert(0 == locks_.fetch_add(1)); \
    mutex_.lock(); \
} ()

#ifdef CASPER_APP_WATCHDOG_UNLOCK
    #undef CASPER_APP_WATCHDOG_UNLOCK
#endif
#define CASPER_APP_WATCHDOG_UNLOCK() [&]() { \
    assert(1 == locks_.fetch_sub(1)); \
    mutex_.unlock(); \
} ()

#ifdef CASPER_APP_WATCHDOG_BARK_ONCE_UNSAFE
    #undef CASPER_APP_WATCHDOG_BARK_ONCE_UNSAFE
#endif
#define CASPER_APP_WATCHDOG_BARK_ONCE_UNSAFE() [&] () { \
    /* ... dump error to log ... */ \
    Bark(last_error_); \
    /* ... not critical ... */ \
    CASPER_APP_MONITOR_RESET_ERROR(last_error_); \
} ()

#ifdef CASPER_APP_WATCHDOG_FATAL_BITE
    #undef CASPER_APP_WATCHDOG_FATAL_BITE
#endif
#define CASPER_APP_WATCHDOG_FATAL_BITE() [&] () { \
    Bite(/* a_fatal */ true); \
} ()

#ifdef CASPER_APP_WATCHDOG_FATAL_BITE_UNSAFE
    #undef CASPER_APP_WATCHDOG_FATAL_BITE_UNSAFE
#endif
#define CASPER_APP_WATCHDOG_FATAL_BITE_UNSAFE() [&] () { \
    BiteUnsafe(/* a_fatal */ true); \
} ()

#ifdef CASPER_APP_WATCHDOG_BITE_UNSAFE
    #undef CASPER_APP_WATCHDOG_BITE_UNSAFE
#endif
#define CASPER_APP_WATCHDOG_BITE_UNSAFE() [&] () { \
    BiteUnsafe(/* a_fatal */ false); \
} ()

/**
 * @brief This method will be called when it's time to initialize this singleton.
 *
 * @param a_instance A reference to this initializer owner.
 */
casper::app::monitor::Initializer::Initializer (casper::app::monitor::Watchdog& a_instance)
    : ::cc::Initializer<casper::app::monitor::Watchdog>(a_instance)
{
    instance_.listener_ptr_ = nullptr;
    instance_.thread_       = nullptr;
    instance_.detached_     = false;
    instance_.locks_        = 0;
    instance_.abort_flag_   = nullptr;
    instance_.main_pid_     = 0;
}

/**
 * @brief Destructor.
 */
casper::app::monitor::Initializer::~Initializer ()
{
    instance_.Stop();
}

/**
 * @brief Spawn and monitor a list of processes.
 *
 * @param a_config     A JSON with configuration options.
 * @param a_listener   A listener to be notified when process(es) list is modified.
 * @param a_detached   True when a new thread must be started, false it will run in current thread.
 * @param a_abort_flag External abort flag.
 */
void casper::app::monitor::Watchdog::Start (const Json::Value& a_config, const bool a_detached,
                                            casper::app::monitor::Watchdog::Listener& a_listener,
                                            bool volatile* a_abort_flag)
{
    //
    // a_config:
    //
    // {
    //     "directories": {
    //         "runtime": "",
    //         "working": "",
    //         "config": "",
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
    
    const Json::Value& variables        = a_config["variables"];
    const Json::Value& common_variables = a_config["variables"]["common"];
    
    const std::string logs_dir          = a_config["directories"]["logs"].asString();
    const std::string runtime_dir       = a_config["directories"]["runtime"].asString();
    
    const std::string config_file_uri = a_config["directories"]["config"].asString() + "monitor.json";
   
    std::string uri;

    struct stat stat_info;
    if ( 0 == stat(config_file_uri.c_str(), &stat_info) ) {
        if ( 0 != S_ISLNK(stat_info.st_mode) ) {
            char tmp[PATH_MAX];
            tmp[0] = 0;
            const ssize_t count = readlink(config_file_uri.c_str(), tmp, ( sizeof(tmp) / sizeof(tmp[0]) ));
            if ( -1 == count ) {
                CASPER_APP_MONITOR_SET_ERROR(nullptr, last_error_,
                                             errno,
                                             "an error occurred while trying to follow symbolic link for configuration file '%s'", config_file_uri.c_str()
                );
            }
            uri = std::string(tmp, 0, static_cast<size_t>(count));
        } else if ( 0 != S_ISREG(stat_info.st_mode) ) {
            uri = config_file_uri;
        }
    } else {
        CASPER_APP_MONITOR_SET_ERROR(nullptr, last_error_,
                                     sys::Error::k_no_error_,
                                     "an error occurred while trying to open configuration file '%s'", config_file_uri.c_str()
        );
    }
    
    
    // ... notify fatal error ( if any ) ...
    CASPER_APP_WATCHDOG_FATAL_BITE();
    
    Json::Reader reader;
    Json::Value  config;
    
    std::ifstream stream(uri);
    if ( false == stream.is_open() ) {
        CASPER_APP_MONITOR_SET_ERROR(nullptr, last_error_,
                                     sys::Error::k_no_error_,
                                     "an error occurred while trying to open configuration file '%s'", config_file_uri.c_str()
        );
    } else if ( false == reader.parse(stream, config, false)  ) {
        CASPER_APP_MONITOR_SET_ERROR(nullptr, last_error_,
                                     sys::Error::k_no_error_,
                                     "an error occurred while parsing configuration file '%s'", config_file_uri.c_str()
        );
    }
    
    // ... notify fatal error ( if any ) ...
    CASPER_APP_WATCHDOG_FATAL_BITE();

    const Json::Value& children = config["children"];
    
    const auto replace_variables = [] (const std::string& a_string, const Json::Value& a_variables, bool a_is_path) -> std::string {
        
        if ( 0 == a_string.length() ) {
            return a_string;
        }
        
        std::string rv  = a_string;
        
        for ( auto key : a_variables.getMemberNames() ) {
            
            const Json::Value& value = a_variables[key];
            if ( true == value.isNull() || false == value.isString() || nullptr == strcasestr(rv.c_str(), key.c_str()) ) {
                continue;
            }
            
            const std::string v  = value.asString();
            const std::string to = ( true == a_is_path && rv.length() >= 2 && '@' == rv.c_str()[0]  && '@' == rv.c_str()[1] && '/' == v[v.length() -1]  ) ? v.substr(0, v.length() - 1) : v;
            
            size_t pos = 0;
            while ( std::string::npos != ( pos = rv.find(key, pos) ) ) {
                rv.replace(pos, key.length(), to);
                pos += rv.length();
            }
            
        }
        
        return rv;
    };
    
    //
    // ... load processes to launch and monitor ...
    //
   
    std::vector<const ::sys::Process::Info> vector;
    
    for ( Json::ArrayIndex idx = 0 ; idx < children.size() ; ++idx ) {
        
        const Json::Value& entry = children[idx];

        std::string arguments = replace_variables(entry.get("arguments", "").asString(), common_variables, /* a_is_path */ false);
        
        const Json::Value& process_variables = variables[entry["id"].asString()];
        if ( false == process_variables.isNull() && true == process_variables.isObject() ) {
            arguments = replace_variables(arguments, process_variables, /* a_is_path */ false);
        }

        std::list<std::string> precedents;

        const Json::Value& depends_on = entry["depends_on"];
        if ( false == depends_on.isNull() && true == depends_on.isArray() && depends_on.size() > 0 ) {
            for ( Json::ArrayIndex idx = 0 ; idx < depends_on.size() ; ++idx ) {
                precedents.push_back(depends_on[idx].asString());
            }
        }

        vector.push_back({
            /* id_          */ entry["id"].asString(),
            /* owner_       */ "",
            /* path_        */ replace_variables(entry["path"].asString(),common_variables, /* a_is_path */ true),
            /* executable_  */ entry["executable"].asString(),
            /* arguments_   */ arguments,
            /* user_        */ "",
            /* group_       */ "",
            /* working_dir_ */ replace_variables(entry.get("working_dir", "").asString(), common_variables, /* a_is_path */ true),
            /* log_dir      */ logs_dir,
            /* pid_file_    */ entry.get("pid_file", ( runtime_dir + entry["executable"].asString() + ".pid" ) ).asString(),
            /* depends_on_  */ precedents
        });
        
    }
 
    std::list<const ::sys::Process::Info> sorted;
    
    // ... solve dependencies ...
    sys::Process::Sort(vector, sorted);

    // ... try to launch and start monitoring them ...
    if ( false == Start(sorted, a_detached, a_listener, a_abort_flag) ) {
        // ... notify fatal error ...
        CASPER_APP_WATCHDOG_FATAL_BITE();
    }
}

/**
 * @brief Spawn and monitor a list of processes.
 *
 * @param a_list       List of processes to start.
 * @param a_listener   A listener to be notified when process(es) list is modified.
 * @param a_detached   True when a new thread must be started, false it will run in current thread.
 * @param a_abort_flag External abort flag.
 *
 * @return True on success, false otherwise.
 */
bool casper::app::monitor::Watchdog::Start (const std::list<const ::sys::Process::Info>& a_list, const bool a_detached,
                                            casper::app::monitor::Watchdog::Listener& a_listener,
                                            bool volatile* a_abort_flag)
{
    // ... cleanup, if required ...
    Stop();
    
    CASPER_APP_WATCHDOG_LOCK();
    
    listener_ptr_ = &a_listener;
    detached_     = a_detached;
    main_pid_     = getpid();
    
    // ... keep track of new process(es) to spawn ...
    for ( auto info : a_list ) {
        // ... create a new process ...
        ::sys::darwin::Process* process = new ::sys::darwin::Process(::sys::darwin::Process::Info(info));
        // ... check if it's an executable and ensure directories are created and can be accessed ...
        if ( false == EnsureRequirements(*process) ) {
            // ... forget process ...
            delete process;
            // ... failure, unlock mutex ...
            CASPER_APP_WATCHDOG_UNLOCK();
            // ... done ...
            return false;
        }
        // ... load previous executed pid from file ( if any ) ...
        (void) process->LoadPIDFromFile(/* a_optional */ true);
        // ... keep track of it ...
        list_.push_back(process);
    }
    
    // ... now try to terminate all running processes, launched by this app ...
    if ( false == TerminateAll(/* a_optional */ true) ) {
        CASPER_APP_WATCHDOG_BITE_UNSAFE();
    }
    
    CASPER_APP_WATCHDOG_UNLOCK();
    
    // ... install signal(s) handler(s) ...
    signal(SIGUSR2, casper::app::monitor::Watchdog::OnSignal);
    signal(SIGCHLD, casper::app::monitor::Watchdog::OnSignal);
    signal(SIGTERM, casper::app::monitor::Watchdog::OnSignal);
    
    // .. keep track of abort flag ...
    abort_flag_ = a_abort_flag;
    
    // ... start a new thread? ....
    if ( true == a_detached ) {
        // ... start a new thread ....
        thread_ = new std::thread(&casper::app::monitor::Watchdog::Loop, this);
        thread_->detach();
        // ... wait for thread to be running ...
        thread_cv_.Wait();
    } else {
        Loop();
    }
    
    // ... we're done ...
    return true;
}

/**
 * @brief Stop monitoring processes.
 */
void casper::app::monitor::Watchdog::Stop ()
{
    // ... first release thread ...
    if ( nullptr != thread_ ) {
        delete thread_;
        thread_ = nullptr;
    }
    
    CASPER_APP_WATCHDOG_LOCK();
    
    // ... now try to terminate all running processes, launched by this app ...
    if ( false == TerminateAll(/* a_optional */ true) ) {
        CASPER_APP_WATCHDOG_BARK_ONCE_UNSAFE();
    }
    
    // ... then release them ...
    for ( auto it : list_ ) {
        delete it;
    }
    list_.clear();
    last_error_.Reset();
    
    // ... forget all other data ...
    listener_ptr_ = nullptr;
    detached_     = false;
    main_pid_     = 0;
    
    CASPER_APP_WATCHDOG_UNLOCK();
}

/**
 * @brief Quit loop.
 */
void casper::app::monitor::Watchdog::Quit ()
{
    Stop();
}

/**
 * @brief Quit loop.
 */
void casper::app::monitor::Watchdog::Refresh ()
{
    if ( true == detached_ ) {
        // ... signal parent ...
        kill(getppid(), SIGUSR2);
    } else {
        // ... just call signal handler ...
        casper::app::monitor::Watchdog::OnSignal(SIGUSR2);
    }
}

#ifdef __APPLE__
#pragma mark -
#endif

/**
 * @brief Thread function where the 'loop' will run.
 */
void casper::app::monitor::Watchdog::Loop ()
{
    // ... log ...
    CASPER_APP_DEBUG_LOG("status", "%s", "Starting...");

    // ... signal thread is running ...
    thread_cv_.Wake();
    
    sigset_t sigmask;
    sigset_t saved_sigmask;
    
    // ... block SIGCHLD ...
    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGCHLD);
    pthread_sigmask(SIG_BLOCK, &sigmask, &saved_sigmask);
    
    CASPER_APP_WATCHDOG_LOCK();

    // ... first try to terminate all running processes, launched by this app ...
    if ( false == TerminateAll(/* a_optional */ true) ) {
        CASPER_APP_WATCHDOG_BARK_ONCE_UNSAFE();
    }
    
    // ... now spawn new processes ...
    for ( auto internal : list_ ) {
        // ... try to fork and exec for this process ...
        if ( false == Spawn(*internal, /* a_block_sigchld */ false) ) {
            CASPER_APP_WATCHDOG_FATAL_BITE_UNSAFE();
        }
        
        // TODO CW: fix this?
        // ... give some time to process start ...
        CASPER_APP_WATCHDOG_UNLOCK();
        usleep(1000*1000);
        CASPER_APP_WATCHDOG_LOCK();
        
    }

    CASPER_APP_WATCHDOG_UNLOCK();

    // ... restore the signal mask ...
    pthread_sigmask(SIG_SETMASK, &saved_sigmask, NULL);
    
    typedef struct  {
        pid_t                 pid_;
        const ::sys::Process* process_;
        std::string           reason_;
        bool                  terminated_;
        int                   status_;
        bool                  signalled_;
        int                   signal_;
        bool                  stopped_;
    } Child;
    
#if 0
    const int wait_options = WUNTRACED | WCONTINUED;
#else
    const int wait_options = WNOHANG;

    struct timeval timeout;

    timeout.tv_sec  = 1;
    timeout.tv_usec = 0;
    
#endif
    
    // ... monitor children ...
    while ( false == (*abort_flag_) ) {

        Child child = { 0, nullptr, "", false, INT_MIN, false, INT_MIN, false };
        
#if 0
        // ... log ...
        CASPER_APP_DEBUG_LOG("status", "%s", "Waiting...");
        
        child.pid_ = waitpid(-1, &child.status_, wait_options);
        
#else

        // ... log ...
//        CASPER_APP_DEBUG_LOG("status", "Waiting, timeout in %d second(s)...", static_cast<int>(timeout.tv_sec));

        bool polling_timeout = false;
        
        while ( false == (*abort_flag_) ) {
            
            const int rv = select(0, nullptr, nullptr, nullptr, &timeout);
            if ( rv == -1 && errno == EINTR ) {
                child.pid_ = waitpid(-1, &child.status_, wait_options);
                if ( -1 == child.pid_ || child.pid_ > 0 ) {
                    break;
                }
            } else if ( 0 == rv ) {
                // ... timeout ..
                polling_timeout = true;
                break;
            } else {
                // ... error ...
                break;
            }
        }

        if ( true == polling_timeout ) {
            continue;
        }
        
        if ( true == (*abort_flag_ ) ) {
            break;
        }
        
#endif
        
        if ( -1 == child.pid_ ) {
            const auto err_no = errno;
            if ( SIGILL == err_no ) { // 4 - Interrupted system call
                // ... log ...
                CASPER_APP_DEBUG_LOG("status", "%s", "4 - Interrupted system call");
                continue;
                
            }
            CASPER_APP_MONITOR_SET_ERROR(nullptr, last_error_,
                                         errno,
                                         "%s", "an error occurred while monitoring children"
            );
            break;
        }
        
        // ... log ...
        CASPER_APP_DEBUG_LOG("status", "%s", "Received signal...");
        
        CASPER_APP_WATCHDOG_LOCK();

        for ( auto process : list_ ) {
            if ( process->pid() == child.pid_ ) {
                child.process_ = process;
                break;
            }
        }
 
        if ( nullptr == child.process_ ) {
            CASPER_APP_MONITOR_SET_ERROR(child.process_, last_error_,
                                         errno,
                                         "an error occurred while monitoring child with pid %d: A signal was sent to a child that no longer exists?", child.pid_
            );
        }
        
        CASPER_APP_WATCHDOG_UNLOCK();
        
        // ... if an error is set, we're in a invalid state!
        if ( true == IsErrorSet() ) {
            break;
        }
        
        CASPER_APP_WATCHDOG_LOCK();

        // ... log ...
        CASPER_APP_DEBUG_LOG("status", "... for child %s ( %d )...",
                             child.process_->info().id_.c_str(), child.pid_
        );
        
        // ... check child status ...
        if ( true == WIFEXITED(child.status_) ) {
            // ... child terminated normally ...
            child.terminated_ = true;
            // ... grab exit status of the child ...
            child.status_     = WEXITSTATUS(child.status_);
            child.reason_     = "terminated normally";
        } else if ( true == WIFSIGNALED(child.status_) ) {
            // ... child process was signaled by a signal ...
            child.signalled_ = true;
            child.reason_    = "received signal ";
            if ( true == WCOREDUMP(child.status_) ) {
                // ... child produced a core dump ...
                child.reason_ += " and produced a core dump";
            } else {
                //  ... grab number of the signal that caused the child process to terminate ...
                child.signal_ = WTERMSIG(child.status_);
                if ( SIGTRAP == child.signal_ ) {
                    child.signalled_ = false;
                }
                child.reason_ += std::to_string(child.signal_);
            }
        } else if ( wait_options & WUNTRACED ) {
            if ( true == WIFSTOPPED(child.status_) ) {
                // ... child process was stopped by delivery of a signal ...
                child.stopped_ = true;
                child.reason_  = " was stopped by delivery of a signal";
                // ... grab number of the signal which caused the child to stop ...
                child.signal_ = WSTOPSIG(child.status_);
                child.reason_ += " ( " + std::to_string(child.signal_) + " )";
            }
        } else if ( wait_options & WCONTINUED ) {
            if ( true == WIFCONTINUED(child.status_) ) {
                // ... child process was resumed by delivery of SIGCONT ...
                child.reason_ += "resumed by delivery of SIGCONT";
            }
        }
        
        // ... log ...
        CASPER_APP_DEBUG_LOG("status", "... %s...",
                             child.reason_.c_str()
        );
        
        if ( true == child.terminated_ || true == child.signalled_ ) {
            

            if ( true == child.terminated_ || SIGTERM == child.signal_ || SIGQUIT == child.signal_ || SIGKILL == child.signal_  ) {
                
                CASPER_APP_MONITOR_SET_ERROR(child.process_, last_error_,
                                             ::sys::Error::k_no_error_,
                                             "%s ( %d ) %s", child.process_->info().id_.c_str(), child.process_->pid(), child.reason_.c_str()
                );

            }
            
        }

        CASPER_APP_WATCHDOG_UNLOCK();
        
        if ( true == IsErrorSet() ) {
            break;
        }

    }

    // ... log ...
    CASPER_APP_DEBUG_LOG("status", "%s", "Cleanup...");
    
    // ... uninstall signal(s) handler(s) ...
    signal(SIGUSR2, SIG_DFL);
    signal(SIGCHLD, SIG_DFL);
    signal(SIGTERM, SIG_DFL);

    CASPER_APP_WATCHDOG_LOCK();
    
    const pid_t my_pid = getpid();
    
    TerminateAll(/* a_optional */ true, my_pid);

    std::list<const sys::Process*> interest;
    for ( auto it : list_ ) {
        interest.push_back(it);
    }

    std::list<const sys::Process*> pending;
    if ( true == sys::Process::Filter(interest, pending) ) {
        
        while ( pending.size() > 0 ) {

            CASPER_APP_DEBUG_LOG("status", "%lu processe(s) remaining...", pending.size());

            for ( std::list<const ::sys::Process*>::reverse_iterator it = pending.rbegin() ; pending.rend() != it ; ++it ) {
                ::sys::Process* process = const_cast<::sys::Process*>(*it);
                
                bool is_running, is_zombie, erased = false;
                
                if ( ( true == process->IsRunning(/* a_optional */ true, /* a_parent_pid */ my_pid, is_running) && false == is_running )
                    ||
                    ( true == process->IsZombie(/* a_optional */ true, is_zombie) && true == is_zombie )
                )
                {
                    for ( std::list<const sys::Process*>::iterator it2 = interest.begin(); interest.end() != it2 ; ++it2 ) {
                        const ::sys::Process* p2 = (*it2);
                        if ( p2 == process ) {
                            erased = true;
                            interest.erase(it2);
                            break;
                        }
                    }
                }
                
                CASPER_APP_DEBUG_LOG("status", "%s ( %d ) %s running...",
                                     process->info().id_.c_str(), process->pid(), true == erased ? "no longer" : "still"
                );
                
                if ( true == erased ) {
                    (*process) = static_cast<pid_t>(0);
                }
                
            }

            CASPER_APP_WATCHDOG_UNLOCK();

            Notify(SIGUSR2);
            usleep(500*1000);

            CASPER_APP_WATCHDOG_LOCK();

            pending.clear();
            if ( false == sys::Process::Filter(interest, pending) ) {
                break;
            }
            
        }
    }
    
    CASPER_APP_WATCHDOG_UNLOCK();

    Notify(SIGUSR2);

    // ... log ...
    CASPER_APP_DEBUG_LOG("status", "%s", "Shutting down...");
}

/**
 * @brief Kill all processes loaded processes.
 *
 * @param a_optional If true won't report failure when signal is not sent.
 *
 * @return True on success, false otherwise.
 */
bool casper::app::monitor::Watchdog::KillAll (const bool& a_optional, const pid_t a_parent_pid)
{
    return SignalAll(SIGKILL, a_optional, a_parent_pid);
}

/**
 * @brief Terminate all processes loaded processes.
 *
 * @param a_optional If true won't report failure when signal is not sent.
 *
 * @return True on success, false otherwise.
 */
bool casper::app::monitor::Watchdog::TerminateAll (const bool& a_optional, const pid_t a_parent_pid)
{
    return SignalAll(SIGTERM, a_optional, a_parent_pid);
}

/**
 * @brief Signal all processes loaded processes.
 *
 * @param a_no       Signal number to send.
 * @param a_optional If true won't report failure when signal is not sent.
 *
 * @return True on success, false otherwise.
 */
bool casper::app::monitor::Watchdog::SignalAll (const int a_no, const bool& a_optional, const pid_t a_parent_pid)
{
    // ... reverse terminate process(es) ...
    for ( std::list<::sys::Process*>::reverse_iterator it = list_.rbegin() ; list_.rend() != it ; ++it ) {
        
        ::sys::Process* process = (::sys::Process*)(*it);
        
        // ... try to send a signal to process ...
        bool is_running, is_zombie = false;
        
        if ( ( true == process->IsRunning(/* a_optional */ true, /* a_parent_pid */ 0 != a_parent_pid ? a_parent_pid : 1, is_running) && true == is_running )
                ||
             ( true == process->IsZombie(/* a_optional */ true, is_zombie) && true == is_zombie )
        ) {
            if ( false == process->Signal(a_no, a_optional) ) {
                // ... error should be set, nothing else to do here ...
                if ( a_optional == false ) {
                    return false;
                }
            }
        }
        
        // ... log ...
        CASPER_APP_DEBUG_LOG("status", "%-30s %-4d %-8d %c%c",
                             process->info().id_.c_str(),
                             a_no,
                             static_cast<int>(process->pid()),
                             is_running ? 'R' : '-', is_zombie ? 'Z' : '-'
        );

    }
    
    // ... done ...
    return true;
}

#ifdef __APPLE__
#pragma mark -
#endif

/**
 * @brief Spawn a new process by fork-exec combination.
 *
 * @param a_process       The process that requested this action.
 * @param a_block_sigchld When true SIGCHLD will be blocked.
 *
 * @return True on success, false on failure.
 */
bool casper::app::monitor::Watchdog::Spawn (::sys::Process& a_process, const bool& a_block_sigchld)
{
    sigset_t sigmask;
    sigset_t saved_sigmask;

    // ... reset all signals ...
    sigemptyset(&sigmask);
    pthread_sigmask(SIG_SETMASK, &sigmask, NULL);

    // ... block SIGCHLD?
    if ( true == a_block_sigchld ) {
        sigaddset(&sigmask, SIGCHLD);
        pthread_sigmask(SIG_BLOCK, &sigmask, &saved_sigmask);
    }

    // ... log ...
    CASPER_APP_DEBUG_LOG("status",
                         "1) %s", a_process.uri().c_str()
    );
    
    a_process = fork();    
    
    if ( 0 > a_process.pid() ) { // ... unable to fork ...
        CASPER_APP_MONITOR_SET_ERROR(&a_process, last_error_,
                                     errno,
                                     "unable to launch '%s' - fork failure", a_process.uri().c_str()
        );
        return false;
    } else if ( 0 == a_process.pid() ) { // ... child ...
        
        // ... close ALL open files ...
        const int max = getdtablesize();
        // ... but skip 0 - stdin, 1 - stdout, 2 - stderr ....
        for ( int n = 3; n < max; n++ ) {
            close(n);
        }

        // ... restart logger ...
        if ( true == detached_ ) {
            casper::app::Logger::GetInstance().Restart("watchdog", a_process.info().id_.c_str());
        }
        
        // ... redirect stdout and stderr to a file
        const std::list<std::pair<FILE*, std::string>> redirect_list = {
            { stdout, a_process.info().log_dir_ + a_process.info().id_ + "-stdout.log" },
            { stderr, a_process.info().log_dir_ + a_process.info().id_ + "-stderr.log" }
        };
        
        if ( false == Redirect(a_process, redirect_list) ) {
            CASPER_APP_WATCHDOG_BARK_ONCE_UNSAFE();
        }

        // ... create session and set process group ID ...
        setsid();
        
        // ... set pid ...
        a_process = getpid();
        
        // ... signal parent ...
        kill(getppid(), SIGUSR2);
        
        // ... log ...
        CASPER_APP_DEBUG_LOG("status",
                             "2) %s", a_process.uri().c_str()
        );
        
        // ... execute process ...
        if ( false == Exec(a_process) ) {
            CASPER_APP_WATCHDOG_FATAL_BITE_UNSAFE();
        }
        
    } /* else { ... } - parent */
    
    // ... restore the signal mask?
    if ( true == a_block_sigchld ) {
        pthread_sigmask(SIG_SETMASK, &saved_sigmask, NULL);
    }
    
    // ... done ...
    return true;
}

/**
 * @brief Exec a child process, assuming fork was already called.
 *
 * @param a_process The process to be launched.
 *
 * @return True on success, false on failure.
 */
bool casper::app::monitor::Watchdog::Exec (::sys::Process& a_process)
{
    const std::string uri  = a_process.uri();
    const std::string args = a_process.info().arguments_;

    // ... log ...
    CASPER_APP_DEBUG_LOG("status",
                         "%s %s", uri.c_str(), args.c_str()
    );
    
    // ... write pid ...
    if ( false == a_process.WritePID() ) {
        // ... error already set ...
        return false;
    }
    
    // ... the child process if the parent dies ...
    // LINUX ONLY: prctl(PR_SET_PDEATHSIG, SIGHUP);
    
    // TODO CW: confirm this
    signal(SIGINT , SIG_DFL);
    signal(SIGHUP , SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGUSR2, SIG_DFL);
    signal(SIGPIPE, SIG_DFL);
    // TODO CW: fix this?
    signal(SIGTRAP, SIG_DFL);
    
    // TODO
    // Clear(); ?

    if ( -1 == setenv("PATH", a_process.info().path_.c_str(), 1) ) {
        CASPER_APP_LOG("status",
                       "WARNING: Failed to set environment variable 'PATH' to '%s', while preparing to execute %s!",
                        a_process.info().path_.c_str(), a_process.info().executable_.c_str()
        );
    }
    
    (void)execvP(a_process.info().executable_.c_str(), a_process.info().path_.c_str(), a_process.argv());    
    
    // ... if it reaches here, an error occurred with execvP ...
    CASPER_APP_MONITOR_SET_ERROR(&a_process, last_error_,
                                 errno,
                                 "unable to start '%s' - exec failure", a_process.uri().c_str()
    );
    
    // ... failure ...
    return false;
}

/**
 * @brief Ensure a process directories can be created and accessed.
 *
 * @param a_process   The process that requested this action.
 * @param a_directory The directory that needs to be created.
 *
 * @return True on success, false on failure.
 */
bool casper::app::monitor::Watchdog::MKDIR (const ::sys::Process* a_process, const std::string& a_directory)
{
    
    if ( nullptr == strchr(a_directory.c_str(), '/') ) {
        CASPER_APP_MONITOR_SET_ERROR(a_process, last_error_,
                                     errno,
                                     "unable to create directory '%s': not a valid path", a_directory.c_str()
        );
        return false;
    }

    // ... split ...
    std::istringstream stream(a_directory);
    std::vector<std::string> components;
    std::string component;
    while ( std::getline(stream, component, '/') ) {
        components.push_back(component);
    }
    
    std::string path = "/";
    for ( auto it = components.begin() + 1  ; it != components.end() ; ++it ) {
        
        path += (*it);
        
        const char* const directory = path.c_str();
        struct stat stat_info;
        if ( 0 == stat(directory, &stat_info)  ) {
            if ( S_ISREG(stat_info.st_mode) ) {
                CASPER_APP_MONITOR_SET_ERROR(a_process, last_error_,
                                             errno,
                                             "unable to create directory '%s' already set as a file", directory
                );
                break;
            } else if ( ! S_ISDIR(stat_info.st_mode) ) {
                if ( -1 == mkdir(directory, (S_IRWXU | S_IRGRP | S_IXGRP | S_IXOTH)) ) {
                    CASPER_APP_MONITOR_SET_ERROR(a_process, last_error_,
                                                 errno,
                                                 "unable to create directory '%s'", directory
                    );
                    break;
                }
            }
        } else {
            CASPER_APP_MONITOR_SET_ERROR(a_process, last_error_,
                                         errno,
                                         "unable to access directory '%s'", directory
            );
            break;
        }
        
        path += "/";
    }
    
    
    // ... done ...
    return ( false == IsErrorSetUnsafe() );
}

/**
 * @brief Ensure a process is executable and all directories can be accessed and / or created.
 *
 * @param a_process The process that requested this action.
 *
 * @return True on success, false on failure.
 */
bool casper::app::monitor::Watchdog::EnsureRequirements (const ::sys::Process& a_process)
{
    
    // ... ensure it's an executable ...
    if ( 0 != access(a_process.uri().c_str(), X_OK) ) {
        // ... it's not ...
        CASPER_APP_MONITOR_SET_ERROR(&a_process, last_error_,
                                     errno,
                                     "%s is not an executable program", a_process.uri().c_str()
       );
    }
    
    const auto dir_name = [] (const std::string& a_uri) -> std::string {
        
        const char* pch = strrchr(a_uri.c_str(), '/');
        if ( nullptr == pch ) {
            return a_uri;
        }
        return std::string(a_uri.c_str(), pch - a_uri.c_str());
        
    };
    
    const std::vector<std::string> dirs = {
        a_process.info().working_dir_,
        dir_name(a_process.info().pid_file_)
    };
    
    // ... ensure all directories are created ...
    for ( auto it : dirs ) {
        if ( false == MKDIR(&a_process, it) ) {
            break;
        }
    }
    
    // ... done ...
    return ( false == IsErrorSetUnsafe() );
}

/**
 * @brief Redirect a FILE* output to a file.
 *
 * @param a_process The \link Process \link that wants to redirect.
 * @param a_list    The list of FILE* to redirect.
 *
 * @return True on success, false on failure.
 */
bool casper::app::monitor::Watchdog::Redirect (const ::sys::Process& a_process,
                                               const std::list<std::pair<FILE *, std::string>>& a_list)
{
    // TODO CW : check why S_IWGRP does't stick
    const int mode        = O_RDWR | O_CREAT | O_APPEND;                     /* Open for Read and Write, Create if doesn't exist or Append */
    const int permissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH; /* Grant User Read and Write permissions, Read for Group and all others */
    
    // ... redirect all streams to a file ...
    for ( auto it : a_list ) {
        
        const int src_fd = fileno(it.first);
        const int dst_fd = open(it.second.c_str(), mode, permissions);
        
        // ... log ...
        CASPER_APP_DEBUG_LOG("status",
                             "redirecting %s fd %d to %d ( %s ) ", a_process.info().id_.c_str(), src_fd, dst_fd, it.second.c_str()
        );

        if ( -1 == dst_fd ) {
            CASPER_APP_MONITOR_SET_ERROR(&a_process, last_error_,
                                         errno,
                                         "unable to open %s for stream redirect", it.second.c_str()
            );
        } else if ( -1 == dup2(dst_fd, src_fd) ) {
            CASPER_APP_MONITOR_SET_ERROR(&a_process, last_error_,
                                         errno,
                                         "unable to duplicate fd %d for stream redirect to %s", src_fd, it.second.c_str()
            );
        }
        
    }
    
    // ... done ...
    return ( false == IsErrorSetUnsafe() );
}

#ifdef __APPLE__
#pragma mark -
#endif

/**
 * @brief Signal callback: called when a child process is about to start.
 *
 * @param a_signal_no The signal number.
 */
void casper::app::monitor::Watchdog::OnSignal (int a_signal_no)
{
    casper::app::monitor::Watchdog& instance = casper::app::monitor::Watchdog::GetInstance();
    
    // ... log ...
    CASPER_APP_DEBUG_LOG("status", "%2d", a_signal_no);
    
    // ... notify parent?
    if ( getpid() == instance.main_pid_ ) {
        instance.Notify(a_signal_no);
    }
}

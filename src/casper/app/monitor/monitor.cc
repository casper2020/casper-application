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

#include "casper/app/monitor/monitor.h"

#include "casper/app/monitor/helper.h"

#include "casper/app/logger.h"

#include <unistd.h> // access, pid_t, getppid
#include <errno.h>  // errno
#include <signal.h> // sigemptyset, sigaddset, pthread_sigmask, etc
#include <sstream>  // stringstream
#include <inttypes.h> // PRId32
#include <sys/stat.h> //fstat

#include <sys/event.h> // kevent

#include <grp.h> // getgrgid

#include <set>     // std::set

#include <cstdarg> // va_start, va_end, std::va_list

#include <spawn.h>

#include "json/json.h"
#include <iostream> // std::istream, std::ios
#include <fstream>  // std::filebuf

#include <fcntl.h>

#include "lemon/topology_sort.h"

/**
 * @brief This method will be called when it's time to initialize this singleton.
 */
casper::app::monitor::Initializer::Initializer (casper::app::monitor::Minimalist& a_instance)
    : ::cc::Initializer<casper::app::monitor::Minimalist>(a_instance)
{
    instance_.listener_ptr_ = nullptr;
    instance_.thread_       = nullptr;
}

casper::app::monitor::Initializer::~Initializer ()
{
    instance_.Stop();
}

/**
 * @brief Spawn and monitor a list of processes.
 *
 * @param a_listener
 * @param a_settings
 *
 * @return True on success, false otherwise.
 */
bool casper::app::monitor::Minimalist::Start (casper::app::monitor::Minimalist::Listener* a_listener,
                                              const Settings& a_settings)
{
    const std::string config_file_uri = a_settings.resources_directory_ + "monitor.json";
   
    std::string uri;

    struct stat stat_info;
    if ( 0 == stat(config_file_uri.c_str(), &stat_info) ) {
        if ( 0 != S_ISLNK(stat_info.st_mode) ) {
            char tmp[PATH_MAX];
            tmp[0] = 0;
            const int count = readlink(config_file_uri.c_str(), tmp, ( sizeof(tmp) / sizeof(tmp[0]) ));
            if ( -1 == count ) {
                CASPER_APP_MONITOR_SET_ERROR(nullptr, last_error_,
                                             errno,
                                             "an error occurred while trying to follow symbolic link for configuration file '%s'", config_file_uri.c_str()
                );
                return false;
            }
            uri = std::string(tmp, 0, static_cast<size_t>(count));
        } else if ( 0 != S_ISREG(stat_info.st_mode) ) {
            uri = config_file_uri;
        }
    } else {
        CASPER_APP_MONITOR_SET_ERROR(nullptr, last_error_,
                                     casper::app::monitor::Error::k_no_error_,
                                     "an error occurred while trying to open configuration file '%s'", config_file_uri.c_str()
        );
    }
    
    Json::Reader reader;
    Json::Value  config;
    
    std::ifstream stream(uri);
    if ( false == stream.is_open() ) {
        CASPER_APP_MONITOR_SET_ERROR(nullptr, last_error_,
                                     casper::app::monitor::Error::k_no_error_,
                                     "an error occurred while trying to open configuration file '%s'", config_file_uri.c_str()
        );
    } else if ( false == reader.parse(stream, config, false)  ) {
        CASPER_APP_MONITOR_SET_ERROR(nullptr, last_error_,
                                     casper::app::monitor::Error::k_no_error_,
                                     "an error occurred while parsing configuration file '%s'", config_file_uri.c_str()
        );
    }
    
    // ... continue?
    if ( true == IsErrorSet() ) {
        // ... no ...
        return false;
    }
    
    const Json::Value& children = config["children"];
    
    const auto replace_prefix = [] (const std::string& a_string, const std::string& a_from, const std::string& a_to) -> std::string{
        
        const std::string to = a_to.substr(0, a_to.length() - 1);
        
        std::string rv  = a_string;
        size_t      pos = 0;
        while ( std::string::npos != ( pos = rv.find(a_from, pos) ) ) {
            rv.replace(pos, a_from.length(), to);
            pos += rv.length();
        }
        
        return rv;
    };
    
    //
    // ... load processes to launch and monitor ...
    //
    struct Dependency {
        const std::string      id_;
        std::list<std::string> precedents_;
    };
    std::vector<struct Dependency> dependencies;
    
    std::vector<casper::app::monitor::Process::Info> list;
    
    for ( Json::ArrayIndex idx = 0 ; idx < children.size() ; ++idx ) {
        
        const Json::Value& entry = children[idx];

        std::string arguments = replace_prefix(entry.get("arguments", "").asString(), "@@APP_CONFIG_DIRECTORY_PREFIX@@", a_settings.config_directory_prefix_);
        if ( 0 == strcasecmp("postgresql", entry["id"].asCString()) ) {
            const auto data_dir_pos = arguments.find("@@APP_POSTGRESQL_DATA_DIR@@");
            if ( std::string::npos != data_dir_pos ) {
                arguments.replace(data_dir_pos, strlen("@@APP_POSTGRESQL_DATA_DIR@@"), a_settings.postgresql_.data_directory_);
            }
            const auto args_pos = arguments.find("@@APP_POSTGRESQL_ARGUMENTS@@");
            if ( std::string::npos != args_pos ) {
                arguments.replace(args_pos, strlen("@@APP_POSTGRESQL_ARGUMENTS@@"), a_settings.postgresql_.arguments_);
            }
        }
        
        dependencies.push_back({
            /* id_         */ entry["id"].asString(),
            /* precedents_ */ {}
        });
        
        auto& dependency = dependencies[dependencies.size()-1];
        
        const Json::Value& depends_on = entry["depends_on"];
        if ( false == depends_on.isNull() && true == depends_on.isArray() && depends_on.size() > 0 ) {
            for ( Json::ArrayIndex idx = 0 ; idx < depends_on.size() ; ++idx ) {
                dependency.precedents_.push_back(depends_on[idx].asString());
            }
        }

        list.push_back({
            /* id_          */ entry["id"].asString(),
            /* owner_       */ "",
            /* path_        */ replace_prefix(entry["path"].asString(), "@@APP_DIRECTORY_PREFIX@@", a_settings.app_directory_prefix_),
            /* executable_  */ entry["executable"].asString(),
            /* arguments_   */ arguments,
            /* user_        */ "",
            /* group_       */ "",
            /* working_dir_ */ replace_prefix(
                                entry.get("working_dir"    , a_settings.working_directory_).asString(), "@@APP_WORKING_DIRECTORY_PATH@@", a_settings.working_directory_
                               ),
            /* log_dir      */ replace_prefix(
                                entry.get("logs_directory_", a_settings.logs_directory_).asString(), "@@APP_WORKING_DIRECTORY_PATH@@", a_settings.logs_directory_
                               ),
            /* pid_file_    */ entry.get("pid_file", ( a_settings.runtime_directory_ + entry["executable"].asString() + ".pid" ) ).asString(),
            /* depends_on_  */ dependency.precedents_
        });
        
    }
    
    //
    // ... dependencies check ...
    //
    lemon::ListDigraph               dep_graph;
    lemon::ListDigraph::ArcMap<int>  arc_cost(dep_graph);
    lemon::ListDigraph::NodeMap<int> ordered(dep_graph);

    std::map<std::string, lemon::ListDigraph::Node> nodes_to_names;
    for ( auto dependency : dependencies ) {
        auto node = dep_graph.addNode();
        nodes_to_names[dependency.id_] = node;
    }
    
    for ( auto dependency: dependencies ) {
        auto& node = nodes_to_names[dependency.id_];
        for ( auto precedents_it = dependency.precedents_.begin(); dependency.precedents_.end() != precedents_it ; ++precedents_it ) {
            auto iterator = nodes_to_names.find(*precedents_it);
            if ( nodes_to_names.end() != iterator ) {
                arc_cost.set(dep_graph.addArc(node, iterator->second), 1);
            }
        }
    }

    // ... check if there are circular dependencies...
    if ( lemon::checkedTopologicalSort(dep_graph, ordered) == false ) {
        
        lemon::Path<lemon::ListDigraph>                                              path;
        lemon::HartmannOrlinMmc<lemon::ListDigraph, lemon::ListDigraph::ArcMap<int>> hart(dep_graph, arc_cost);
        
        hart.cycle(path).run();
        
        std::string reason = "\t + Found circular dependency in:\n";
        
        for ( int i = 0; i < path.length() ; ++i ) {
            const auto                    item = list.begin() + dep_graph.id(dep_graph.source(path.nth(i)));
            reason += "\t\t - Process: " + item->id_ + " <=";
            if ( item->depends_on_.size() > 0 ) {
                for ( auto d : item->depends_on_ ) {
                    reason += " " + d + "," ;
                }
                reason.pop_back();
            }
            reason += "\n";
        }
        CASPER_APP_MONITOR_SET_ERROR(nullptr, last_error_,
                                     casper::app::monitor::Error::k_no_error_,
                                     "an error occurred while sorting processes list:\n%s", reason.c_str()
        );
        return false;
    }
    
    std::map<size_t, casper::app::monitor::Process::Info*> map;

    const size_t formula_count = list.size();
    for ( lemon::ListDigraph::NodeIt n(dep_graph); n != lemon::INVALID; ++n ) {
         map[( ( formula_count - ordered[n] ) - 1 )] = &(*(list.begin() + dep_graph.id(n)));
    }

    std::list<const casper::app::monitor::Process::Info> sorted;
    for ( size_t idx = 0 ; idx < map.size(); ++idx ) {
        sorted.push_back(*(map.find(idx)->second));
    }
    map.clear();

    // ... try to launch and start monitoring them ...
    return Start(sorted, a_listener);
}

/**
 * @brief Spawn and monitor a list of processes.
 *
 * @param a_list
 * @param a_listener
 *
 * @return True on success, false otherwise.
 */
bool casper::app::monitor::Minimalist::Start (const std::list<const casper::app::monitor::Process::Info>& a_list,
                                              casper::app::monitor::Minimalist::Listener* a_listener)
{
    // ... cleanup, if required ...
    Stop();
    
    mutex_.lock();
    
    listener_ptr_ = a_listener;
    
    // ... keep track of new process(es) to spawn ...
    for ( auto info : a_list ) {
        // ... create a new process ...
        casper::app::monitor::Process* process = new casper::app::monitor::Process(casper::app::monitor::Process::Info(info));
        // ... check if it's an executable and ensure directories are created and can be accessed ...
        if ( false == EnsureRequirements(*process) ) {
            // ... forget process ...
            delete process;
            // ... failure ...
            return false;
        }
        // ... keep track of it ...
        list_.push_back(process);
    }
    
    // ... kill all processes previously launched by this app ....
    if ( false == KillAll(/* a_optional */ false) && true == IsErrorSet() ) {
        Log(last_error_);
    }
    
    mutex_.unlock();
    
    // ... install signal(s) handler(s) ...
    signal(SIGUSR1, casper::app::monitor::Minimalist::USR1);
    signal(SIGUSR2, casper::app::monitor::Minimalist::USR2);
    
    thread_ = new std::thread(&casper::app::monitor::Minimalist::Loop, this);
    thread_->detach();
    
    // ... wait for thread to be running ...
    thread_cv_.Wait();
    
    // ... we're done ...
    return true;
}

/**
 * @brief Stop monitoring processes.
 */
void casper::app::monitor::Minimalist::Stop ()
{
    // ... first release thread ...
    if ( nullptr != thread_ ) {
        delete thread_;
        thread_ = nullptr;
    }
    // ... now try to kill all running processes ...
    if ( false == TerminateAll(/* a_optional */ true) ) {
        Log(last_error_);
    }
    // ... then release them ...
    for ( auto it : list_ ) {
        delete it;
    }
    list_.clear();
    last_error_.Reset();
    // ... forget all other data ...
    listener_ptr_ = nullptr;
}

/**
 * @brief Notify listener about processes list changes.
 */
void casper::app::monitor::Minimalist::Notify ()
{
    mutex_.lock();
    if ( nullptr != listener_ptr_ ) {
        listener_ptr_->OnRunningProcessesUpdated(list_);
    }
    mutex_.unlock();
}

#ifdef __APPLE__
#pragma mark -
#endif

/**
 * @brief
 */
void casper::app::monitor::Minimalist::Loop ()
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
    
    mutex_.lock();
    // ... first try to kill all running processes ...
    if ( false == KillAll(/* a_optional */ true) ) {
        Log(last_error_);
    }
    // ... now spawn new processes ...
    for ( auto internal : list_ ) {
        if ( false == Spawn(*internal, /* a_block_sigchld */ false) ) {
            // ... log error ...
            Log(last_error_);
            // ... and exit now ...
            exit(last_error_.no());
        }
        // TODO CW: fix this?
        usleep(1000*1000);
    }
    mutex_.unlock();
    
    // ... restore the signal mask ...
    pthread_sigmask(SIG_SETMASK, &saved_sigmask, NULL);
    
    // ... if an error is set, we're in a invalid state!
    if ( true == IsErrorSet() ) {
        // ... log error ...
        Log(last_error_);
        // ... and exit now ...
        exit(last_error_.no());
    }
    
    const int wait_options = WUNTRACED | WCONTINUED;
    
    typedef struct  {
        pid_t                                pid_;
        const casper::app::monitor::Process* process_;
        std::string                          reason_;
        bool                                 terminated_;
        int                                  status_;
        bool                                 signalled_;
        int                                  signal_;
        bool                                 stopped_;
    } Child;
    
    // ... monitor children ...
    while ( true ) {
        
        Child child = { 0, nullptr, "", false, INT_MIN, false, INT_MIN, false };
        
        // ... log ...
        CASPER_APP_DEBUG_LOG("status", "%s", "Wating...");
        
        child.pid_ = waitpid(-1, &child.status_, wait_options);
        if ( -1 == child.pid_ ) {
            const auto err_no = errno;
            if ( SIGILL == err_no ) { // 4 - Interrupted system call
                // ... log ...
                CASPER_APP_DEBUG_LOG("status", "%s", "4 - Interrupted system call, continue...");
                // ... wait for next signal ...
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
        
        mutex_.lock();
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
        
        mutex_.unlock();
        
        // ... if an error is set, we're in a invalid state!
        if ( true == IsErrorSet() ) {
            break;
        }
        
        mutex_.lock();

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
            child.reason_    = "was signaled";
            if ( true == WCOREDUMP(child.status_) ) {
                // ... child produced a core dump ...
                child.reason_ += " and produced a core dump";
            } else {
                //  ... grab number of the signal that caused the child process to terminate ...
                child.signal_ = WTERMSIG(child.status_);
                if ( SIGTRAP == child.signal_ ) {
                    child.signalled_ = false;
                }
                child.reason_ += " ( " + std::to_string(child.signal_) + " )";
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
            
                casper::app::monitor::Process* process = const_cast<casper::app::monitor::Process*>(child.process_);
                process->UnlinkPID(/* a_optional */ true);
                (*process) = static_cast<pid_t>(0);
                
                // ... signal parent ...
                kill(getppid(), SIGUSR2);
                
                if ( false == Spawn(*const_cast<casper::app::monitor::Process*>(child.process_), /* a_block_sigchld */ true) ) {
                    if ( true == IsErrorSet() )
                        Log(last_error_);
                }
                
            }
            
        }

        mutex_.unlock();

    }
    
    if ( true == IsErrorSet() ) {
        Log(last_error_);
    }
    
}

/**
 * @brief Kill all processes loaded processes.
 *
 * @param a_optional
 *
 * @return True on success, false otherwise.
 */
bool casper::app::monitor::Minimalist::KillAll (const bool& a_optional)
{
    return SignalAll(SIGKILL, a_optional);
}

/**
 * @brief Terminate all processes loaded processes.
 *
 * @param a_optional
 *
 * @return True on success, false otherwise.
 */
bool casper::app::monitor::Minimalist::TerminateAll (const bool& a_optional)
{
    return SignalAll(SIGTERM, a_optional);
}

/**
 * @brief Signal all processes loaded processes.
 *
 * @param a_no
 * @param a_optional
 *
 * @return True on success, false otherwise.
 */
bool casper::app::monitor::Minimalist::SignalAll (const int a_no, const bool& a_optional)
{
    std::list<const casper::app::monitor::Process*> filter;
    for ( auto process : list_ ) {
        filter.push_back(process);
    }
    std::list<const casper::app::monitor::Process*> filtered;
    // ... get list of running 'interest' processes ...
    if ( false == casper::app::monitor::Process::Filter(filter, filtered) ) {
        // ... error should be set, nothing else to do here ...
        return false;
    }
    
    // ... reverse kill list ...
    for ( std::list<const casper::app::monitor::Process*>::reverse_iterator it = filtered.rbegin() ; filtered.rend() != it ; ++it ) {
        if ( false == const_cast<casper::app::monitor::Process*>((*it))->Signal(a_no) && false == a_optional ) {
            // ... error should be set, nothing else to do here ...
            return false;
        }
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
 * @param a_process
 * @param a_block_sigchld
 *
 * @return True on success, false otherwise.
 */
bool casper::app::monitor::Minimalist::Spawn (casper::app::monitor::Process& a_process, const bool& a_block_sigchld)
{
    sigset_t sigmask;
    sigset_t saved_sigmask;
    
    // ... block SIGCHLD?
    if ( true == a_block_sigchld ) {
        sigemptyset(&sigmask);
        sigaddset(&sigmask, SIGCHLD);
        pthread_sigmask(SIG_BLOCK, &sigmask, &saved_sigmask);
    }

    // ... log ...
    CASPER_APP_DEBUG_LOG("status",
                         "%s", a_process.uri().c_str()
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
        casper::app::Logger::GetInstance().Restart(a_process.info().id_.c_str());
        
        // ... redirect stdout and stderr to a file
        const std::list<std::pair<FILE*, std::string>> redirect_list = {
            { stdout, a_process.info().log_dir_ + a_process.info().id_ + "-stdout.log" },
            { stderr, a_process.info().log_dir_ + a_process.info().id_ + "-stderr.log" }
        };
        
        if ( false == Redirect(a_process, redirect_list) && true == IsErrorSet() ) {
            // ... dump error to log ...
            Log(last_error_);
            // ... not critical ...
            CASPER_APP_MONITOR_RESET_ERROR(last_error_);
        }

        // ... create session and set process group ID ...
        setsid();
        
        // ... set pid ...
        a_process = getpid();
        
        // ... signal parent ...
        // TODO
        kill(getppid(), SIGUSR1);
        
        // ... execute process ...
        if ( false == Exec(a_process, sigmask) ) {
            // TODO CW: FALTAL ERROR
            // ... log error ...
            Log(last_error_);
            // ... and exit now ...
            exit(last_error_.no());
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
 * @param a_process
 * @param a_sigmask
 *
 * @return On success no return, on failure false is returned.
 */
bool casper::app::monitor::Minimalist::Exec (casper::app::monitor::Process& a_process, sigset_t& a_sigmask)
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
    
    // ... reset all signals ...
    sigemptyset(&a_sigmask);
    pthread_sigmask(SIG_SETMASK, &a_sigmask, NULL);
    
    // TODO CW: confirm this
    signal(SIGINT , SIG_DFL);
    signal(SIGHUP , SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGUSR1, SIG_DFL);
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
 * @param a_process
 * @param a_directory
 *
 * @return
 */
bool casper::app::monitor::Minimalist::MKDIR (const casper::app::monitor::Process* a_process, const std::string& a_directory)
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
    return ( false == IsErrorSet() );
}

/**
 * @brief Ensure a process is executable and all directories can be accessed and / or created.
 *
 * @brief a_process
 *
 * @return
 */
bool casper::app::monitor::Minimalist::EnsureRequirements (const casper::app::monitor::Process& a_process)
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
    return ( false == IsErrorSet() );
}

/**
 * @brief
 *
 * @param a_process
 * @param a_list
 */
bool casper::app::monitor::Minimalist::Redirect (const casper::app::monitor::Process& a_process,
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
    return ( false == IsErrorSet() );
}

#ifdef __APPLE__
#pragma mark -
#endif

/**
 * @brief Log an error message.
 *
 * @param a_error
 */
void casper::app::monitor::Minimalist::Log (const casper::app::monitor::Error& a_error)
{
    CASPER_APP_LOG("error", "------ [B] %s ------", "ERROR");
    CASPER_APP_LOG("error", "%s:%d", a_error.function(), a_error.line());
    CASPER_APP_LOG("error", "message: %s", a_error.message().c_str());
    if ( casper::app::monitor::Error::k_no_error_ != a_error.no() ) {
        CASPER_APP_LOG("error", "system: %s", a_error.str().c_str());
    }
    CASPER_APP_LOG("error", "------ [E] %s ------", "ERROR");
}

#ifdef __APPLE__
#pragma mark -
#endif

/**
 * @brief
 *
 * @param a_signal
 */
void casper::app::monitor::Minimalist::USR1 (int a_signal)
{
    // ... log ...
    CASPER_APP_DEBUG_LOG("status", "%d", a_signal);
    // ... notify parent ...
    casper::app::monitor::Minimalist::GetInstance().Notify();
}

/**
 * @brief
 *
 * @param a_signal
 */
void casper::app::monitor::Minimalist::USR2 (int a_signal)
{
    // ... log ...
    CASPER_APP_DEBUG_LOG("status", "%d", a_signal);
    // ... notify parent ...
    casper::app::monitor::Minimalist::GetInstance().Notify();
}

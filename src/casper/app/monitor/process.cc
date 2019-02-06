/**
 * @file process.cc
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

#include "casper/app/monitor/process.h"

#include "casper/app/monitor/helper.h"

#include "casper/app/logger.h"

#include <errno.h>    // errno
#include <sys/stat.h> // stat
#include <signal.h>   // kill
#include <string>
#include <sstream>

/**
 * @brief Default constructor.
 *
 * @param a_info
 */
casper::app::monitor::Process::Process (casper::app::monitor::Process::Info&& a_info)
    : info_(a_info), pid_(0)
{
    argv_ = nullptr;
    argc_ = 0;
    
    if ( info_.path_.size() > 0 && '/' != info_.path_.c_str()[info_.path_.size()-1] ) {
        uri_ = info_.path_ + '/' + info_.executable_;
    } else {
        uri_ = info_.path_ + info_.executable_;
    }
    
    std::vector<std::string> args;
    std::stringstream ss;
    std::string arg;
    ss.str(info_.arguments_);
    while (std::getline(ss, arg, ' ')) {
        args.push_back(arg);
    }
    (*this) = args;
}

/**
 * @brief Destructor
 */
casper::app::monitor::Process::~Process ()
{
    if ( nullptr != argv_ ) {
        for ( size_t idx = 0 ; idx < sizeof(argv_) / sizeof(argv_[0]) ; ++idx ) {
            if ( nullptr != argv_[idx] ) {
                free(argv_[idx]);
            }
        }
        free(argv_);
    }
}

/**
 * @brief Send a signal to a child process.
 *
 * @param a_signal_no
 *
 * @return True on success, false otherwise.
 */
bool casper::app::monitor::Process::Signal (const int& a_signal_no)
{
    pid_t read_pid = pid();

    // ... running processs?
    if ( 0 != read_pid ) {
        // ... log ...
        CASPER_APP_DEBUG_LOG("status",
                             "sending signal %d to %s ( %d )...",
                             a_signal_no, info_.id_.c_str(), read_pid
                             );
        // ... try to signal process ...
        if ( 0 != kill(pid(), a_signal_no) ) {
            CASPER_APP_MONITOR_SET_ERROR(this, error_,
                                         errno,
                                         "failed to a signal to '%s', with pid %d!", info().id_.c_str(), read_pid
            );
        }
    } else if ( true == ReadPID(/* a_optional */ true, read_pid) ) {
        // ... log ...
        CASPER_APP_DEBUG_LOG("status",
                             "sending signal %d to %s ( %d )...",
                             a_signal_no, info_.id_.c_str(), read_pid
        );
        // ... try to signal process ...
        if ( 0 != kill(read_pid, SIGKILL) ) {
            CASPER_APP_MONITOR_SET_ERROR(this, error_,
                                         errno,
                                         "failed to send a signal to '%s' with pid %d!", info().id_.c_str(), read_pid
            );
        } // else { /* nothing to do, error should be set */ }
    }
    
    // ... log ...
    CASPER_APP_DEBUG_LOG("status",
                         "signal %d to %s ( %d ) %s",
                         a_signal_no, info_.id_.c_str(), pid_,
                         is_error_set()? "failed!" : "sent."
   );
    
    // ... done ...
    return ( false == is_error_set() );
}

/**
 * @brief Kill a process.
 *
 * @return True on success, false otherwise.
 */
bool casper::app::monitor::Process::Kill ()
{
    // ... running processs?
    if ( 0 != pid() ) {
        // ... yes , send SIGKILL ...
        const bool success = Signal(SIGKILL);
        if ( true == success && true == UnlinkPID(/* a_optional */ false) ) {
            pid_ = 0;
        }
        // and we're done ...
        return success;
    }
    
    pid_t read_pid = 0;
    if ( true == ReadPID(/* a_optional */ true, read_pid) ) {
        
        // ... log ...
        CASPER_APP_DEBUG_LOG("status",
                             "sending SIGKILL to %s ( %d )...",
                             info_.id_.c_str(), read_pid
        );

        if ( 0 != kill(read_pid, SIGKILL) ) {
            const auto no = errno;
            if ( ESRCH != no ) { // != 3 - No such process
                CASPER_APP_MONITOR_SET_ERROR(this, error_,
                                             errno,
                                             "failed to kill '%s' with pid %d!", uri().c_str(), read_pid
                );
            }
        } else {
            // ... killed ...
            (void)UnlinkPID(/* a_optional */ false);
        }
        
        // ... log ...
        CASPER_APP_DEBUG_LOG("status",
                             "SIGKILL to %s ( %d ) %s",
                             info_.id_.c_str(), read_pid,
                             is_error_set()? "failed!" : "succeeded."
        );
    }
    
    
    // ... done ...
    return ( false == is_error_set() );
}

/**
 * @brief Write a PID to a file.
 *
 * @return True on success, false otherwise.
 */
bool casper::app::monitor::Process::WritePID ()
{
    const char* const uri = info_.pid_file_.c_str();
    
    FILE* file = fopen(uri, "w");
    if ( nullptr == file ) {
        CASPER_APP_MONITOR_SET_ERROR(this, error_,
                                     errno,
                                     "Unable to write to pid file %s!", uri
        );
        return false;
    }
    
    char buffer[20] = {0};
    const int max = sizeof(buffer) / sizeof(buffer[0]) - 1;
    const int aux = snprintf(buffer, max, "%d", pid());
    if ( aux < 0 || aux > max ) {
        CASPER_APP_MONITOR_SET_ERROR(this, error_,
                                     errno,
                                     "Unable to write to pid file %s: buffer write error!", uri
        );
        return false;
    }
    
    fwrite(buffer, sizeof(char), aux, file);
    fclose(file);
    
    return true;
}

/**
 * @brief Read a PID from a file.
 *
 * @param a_optional
 *
 * @param o_pid
 *
 * @return True on success, false otherwise.
 */
bool casper::app::monitor::Process::ReadPID (const bool& a_optional, pid_t& o_pid)
{
    const char* const pid_file_uri = info_.pid_file_.c_str();
    
    struct stat stat_info;
    if ( -1 == stat(pid_file_uri, &stat_info) ) {
        // ... unable to access pid file ...
        CASPER_APP_MONITOR_SET_ERROR(this, error_,
                                     casper::app::monitor::Error::k_no_error_,
                                     "Cannot read '%s' pid - unable to access file '%s'!", uri().c_str(), pid_file_uri
        );
        return false;
    } else if ( 0 == S_ISREG(stat_info.st_mode) ) {
        // ... file does not exist ...
        if ( false == a_optional ) {
            // ... unable to access pid file ...
            CASPER_APP_MONITOR_SET_ERROR(this, error_,
                                         casper::app::monitor::Error::k_no_error_,
                                         "Cannot read '%s' pid - file '%s' does not exist!", uri().c_str(), pid_file_uri
            );
        }
        // ... we're done ...
        return ( a_optional ? true : ( false == is_error_set()) );
    } /* else { // ... file exists, fallthrough ... } */
    
    // ... read from file ...
    FILE* file = fopen(pid_file_uri, "r");
    if ( nullptr == file ) {
        CASPER_APP_MONITOR_SET_ERROR(this, error_,
                                     errno,
                                     "Cannot kill %s - unable to open file '%s' !", uri().c_str(), pid_file_uri
        );
    } else {
        char buffer[20] = {0};
        const size_t max = sizeof(buffer) / sizeof(buffer[0]) - 1;
        const size_t aux = fread(buffer, sizeof(char), max, file);
        if ( 0 == aux ) {
            CASPER_APP_MONITOR_SET_ERROR(this, error_,
                                         errno,
                                         "Cannot kill '%s' - unable to read '%s'!", uri().c_str(), pid_file_uri
            );
        } else {
            if ( 1 != sscanf(buffer, "%d", &o_pid) ) {
                CASPER_APP_MONITOR_SET_ERROR(this, error_,
                                             errno,
                                             "Cannot kill '%s' - unable to scan '%s'!", uri().c_str(), pid_file_uri
                );
            }/* else { // ... pid read ... } */
        }
        fclose(file);
    }
    
    // ... done ...
    return ( false == is_error_set() );
}


/**
 * @brief Unlink a PID file.
 *
 * @param a_optional
 *
 * @return True on success, false otherwise.
 */
bool casper::app::monitor::Process::UnlinkPID (const bool& a_optional)
{
    const char* const pid_file_uri = info_.pid_file_.c_str();
    
    struct stat stat_info;
    if ( -1 == stat(pid_file_uri, &stat_info) ) {
        // ... unable to access pid file ..
        if ( false == a_optional ) {
            CASPER_APP_MONITOR_SET_ERROR(this, error_,
                                         casper::app::monitor::Error::k_no_error_,
                                         "Cannot unlink '%s' pid - unable to access file '%s'!", uri().c_str(), pid_file_uri
            );
            // ... failure ...
            return true;
        } else {
            // ... ignored, done ...
            return true;
        }
    } else if ( 0 == S_ISREG(stat_info.st_mode) ) {
        // ... file does not exist ...
        if ( false == a_optional ) {
            // ... unable to access pid file ...
            CASPER_APP_MONITOR_SET_ERROR(this, error_,
                                         casper::app::monitor::Error::k_no_error_,
                                         "Cannot unlink '%s' pid - file '%s' does not exist!", uri().c_str(), pid_file_uri
            );
            // ... failure ...
            return true;
        } else {
            // ... ignored, done ...
            return true;
        }
    } /* else { // ... file exists, fallthrough ... } */
    
    // ... unlink file ...
    if ( -1 == unlink(pid_file_uri) ) {
        if ( errno == ENOENT ) {
            // ... file does not exist, done ...
            return true;
        } else {
            // ... unable to unlink pid file ...
            CASPER_APP_MONITOR_SET_ERROR(this, error_,
                                         casper::app::monitor::Error::k_no_error_,
                                         "Cannot unlink '%s' pid - file '%s'!", uri().c_str(), pid_file_uri
            );
            // ... failure ...
            return false;
        }
    }
    
    // ... done ...
    return true;
}

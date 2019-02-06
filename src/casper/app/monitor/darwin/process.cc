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

#include "casper/app/monitor/darwin/process.h"

#include <unistd.h>     // access, pid_t, getppid
#include <errno.h>      // errno
#include <sys/sysctl.h> // sysctl

/**
 * @brief Destructor
 */
casper::app::monitor::darwin::Process::~Process ()
{
    /* empty */
}

/**
 * @brief Get the list of running processes 'kill'.
 *
 * @param a_interest
 * @param o_list
 *
 * @return True on success, false otherwise and an error should be set.
 */
bool casper::app::monitor::Process::Filter (const std::list<const casper::app::monitor::Process*>& a_interest,
                                            std::list<const casper::app::monitor::Process*>& o_list)
{
    static const int   name[]      = { CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0 };
    static const u_int name_length = ( sizeof(name) / sizeof(*name) ) - 1;
    struct kinfo_proc* list   = nullptr; // TODO CACHE IT
    int                err_no = 0;
    size_t             length = 0;
    
    const size_t max_attempts = 3;
    for ( size_t attempt_no = 0 ; attempt_no < max_attempts ; ++attempt_no ) {
        
        // ... first sysctl call is to calculate buffer size ...
        length = 0;
        err_no = sysctl((int *)name, name_length, NULL, &length, NULL, 0);
        if ( -1 == err_no ) {
            err_no = errno;
            if ( ENOMEM == err_no ) {
                err_no = 0;
                continue;
            } else {
                break;
            }
        }
        
        // ... allocate an buffer so we make a second call and fill the list ...
        list = (kinfo_proc*)malloc(length);
        if ( list == NULL ) {
            err_no = ENOMEM;
            continue;
        }
        
        // ... obtain processes list...
        // ⚠️ ( since we're making 2 calls in-between we might miss processes ) ⚠️
        if ( -1 == sysctl( (int *) name, name_length, list, &length, NULL, 0) ) {
            free(list);
            list = NULL;
            err_no = errno;
            if ( ENOMEM == err_no ) {
                err_no = 0;
                continue;
            } else {
                break;
            }
        } else {
            break;
        }
        
    }
    
    errno = err_no;
    
    // ... an error is set or no data?
    if ( 0 != err_no ) { // ... error is set ...
        free(list);
        return false;
    } else if ( nullptr == list ) { // ... no data ...
        return false;
    }
    
    std::map<std::string, const casper::app::monitor::Process*> interest;
    for ( const auto it : a_interest ) {
        interest[it->info_.executable_] = it;
    }
    
    // ... collect processes that still have the same pid created previously by this monitor ...
    const size_t count  = length / sizeof(kinfo_proc);
    for ( size_t idx = 0 ; idx < count ; ++idx ) {
        const auto process = &list[idx];
        const auto it      = interest.find(process->kp_proc.p_comm);
        if ( interest.end() == it ) {
            continue;
        }
        o_list.push_back(it->second);
    }
    free(list);
    
    // ... we're done ..
    return true;
}

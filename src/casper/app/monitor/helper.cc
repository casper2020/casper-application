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

#include "casper/app/monitor/helper.h"

#include <vector>  // std::vector
#include <cstdarg> // va_start, va_end, std::va_list

#ifdef __APPLE__
#pragma mark -
#endif

/**
 * @brief Set an error.
 *
 * @param a_process
 * @param o_error
 * @param a_file
 * @param a_function
 * @param a_line
 * @param a_errno
 * @param a_format
 * @param ...
 *
 * @return True on success, false otherwise.
 */
void casper::app::monitor::Helper::SetError (const ::sys::Process* a_process, ::sys::Error& o_error,
                                             const char* const /* a_file */, const char* const a_function, const int a_line,
                                             const errno_t a_errno, const char* const a_format, ...)
{
    std::lock_guard<std::mutex> lock(mutex_);

    o_error.Reset();
    
    o_error = a_errno;
    auto temp   = std::vector<char> {};
    auto length = std::size_t { 512 };
    std::va_list args;
    while ( temp.size() <= length ) {
        temp.resize(length + 1);
        va_start(args, a_format);
        const auto status = std::vsnprintf(temp.data(), temp.size(), a_format, args);
        va_end(args);
        if ( status < 0 ) {
            CASPER_APP_MONITOR_FATAL_ERROR("%s",
                                           "An error occurred while writing an error message to a buffer!"
            );
        }
        length = static_cast<std::size_t>(status);
    }
    o_error = length > 0 ? std::string { temp.data(), length } : 0;
    o_error = std::make_pair(a_function, a_line);
}

/**
 * @file error.h
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

#ifndef CASPER_APP_MONITOR_ERROR_H_
#define CASPER_APP_MONITOR_ERROR_H_
#pragma once

#include <string>

#include <errno.h>  // errno_t

namespace casper
{
    
    namespace app
    {
        
        namespace monitor
        {
            
            class Error
            {
                
            public: // Don't allow copy or move operations
                
                Error (Error const&) = delete; //!< Copy Constructor
                Error (Error &&)     = delete; //!< Move Constructor
                
                Error& operator = (Error const&) = delete; //!< Copy Assign
                Error& operator = (Error &&)     = delete; //!< Move Assign
                
            public: // Static Const Data
                
                static const errno_t k_no_error_ = 0;
                
            private: // Data
                
                errno_t         no_;      //!<
                std::string     str_;     //!<
                std::string     msg_;     //!<
                std::string     fnc_;     //!<
                int             ln_;      //!<
                
            public: // Constructor(s) / Destructor
                
                /**
                 * @brief Default constructor
                 */
                Error ()
                {
                    no_ = k_no_error_;
                    ln_ = 0;
                }
                
                /**
                 * @brief Destructor.
                 */
                virtual ~Error ()
                {
                    /* empty */
                }
                
            public: // Method(s) / Function(s)
                
                inline void Reset ()
                {
                    no_  = k_no_error_;
                    str_ = "";
                    msg_ = "";
                    fnc_ = "";
                    ln_  = 0;
                }
                
            public: // Operator(s) Overloading
                
                inline void operator = (const errno_t a_errno)
                {
                    if ( k_no_error_ != a_errno ) {
                        const char* const str = strerror(a_errno);
                        if ( nullptr != str ) {
                            str_ = std::to_string(a_errno) + " - " + std::string(str);
                        } else {
                            str_ = std::to_string(a_errno) + " - ????";
                        }
                    } else {
                        str_ = "";
                    }
                    no_ = a_errno;
                }
                
                inline void operator = (const std::string& a_message)
                {
                    msg_ = a_message;
                }
                
                inline void operator = (const std::pair<const char* const, int>& a_fl)
                {
                    fnc_ = a_fl.first;
                    ln_  = a_fl.second;
                }
                
            public: // Access Method(s) / Function(s)
                
                inline const errno_t& no () const
                {
                    return no_;
                }
                
                inline const std::string& str () const
                {
                    return str_;
                }
                
                inline const std::string& message () const
                {
                    return msg_;
                }
                
                inline const char* function () const
                {
                    return fnc_.c_str();
                }
                
                inline const int& line () const
                {
                    return ln_;
                }
                              
            }; // end of class 'Error'
            
        } // end of namespace 'monitor'
        
    } // end of namespace 'app'
    
} // end of namespace 'casper'

#endif // CASPER_APP_MONITOR_ERROR_H_

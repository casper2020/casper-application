/**
 * @file process.h
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

#ifndef CASPER_APP_MONITOR_DARWIN_PROCESS_H_
#define CASPER_APP_MONITOR_DARWIN_PROCESS_H_
#pragma once

#include "casper/app/monitor/process.h"

namespace casper
{
    
    namespace app
    {
        
        namespace monitor
        {
            
            namespace darwin
            {
                
                class Process final : public casper::app::monitor::Process
                {
                    
                public: // Constructor(s) / Destructor
                    
                    virtual ~Process ();
                    
                }; // end of class 'Process'
                
            } // end of namepsace 'darwin'
            
        } // end of namepsace 'monitor'
        
    } // end of namespace 'app'
    
} // end of namespace 'casper'

#endif // endif CASPER_APP_MONITOR_DARWIN_PROCESS_H_

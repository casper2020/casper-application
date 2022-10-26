/**
 * @file main_context.h
 *
 * Copyright (c) 2011-2022 Cloudware S.A. All rights reserved.
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

#ifndef CASPER_CEF3_CLIENT_MACOS_MAIN_CONTEXT_H_
#define CASPER_CEF3_CLIENT_MACOS_MAIN_CONTEXT_H_

#pragma once

#include "cef3/client/common/main_context.h"

namespace casper
{
    
    namespace cef3
    {
        
        namespace client
        {
            
            namespace mac
            {
                
                class MainContext : public ::casper::cef3::client::common::MainContext
                {
                    
                public: // Constructor(s) / Destructor
                    
                    MainContext (const ::casper::cef3::browser::Settings& a_settings, CefRefPtr<CefCommandLine> a_command_line);
                    virtual ~MainContext ();
                    
                public: // Inherited Virtual Method(s) / Function(s)
                    
                    virtual bool GetLogsPath      (std::string& o_path) override;
                    virtual bool GetDownloadsPath (std::string& o_path) override;
                    
                private:
                    
                    DISALLOW_COPY_AND_ASSIGN(MainContext);
                    
                }; // end of class 'MainContext'
                
            } // end of namespace 'mac'
            
        } // end of namespace 'client'
        
    } // end of namespace 'cef3'
    
} // end of namespace 'casper'

#endif // CASPER_CEF3_CLIENT_MACOS_MAIN_CONTEXT_H_


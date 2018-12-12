/**
 * @file main_context_types.h
 *
 * Copyright (c) 2010-2018 Neto Ranito & Seabra LDA. All rights reserved.
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

#ifndef CASPER_CEF3_BROWSER_MAIN_CONTEXT_TYPES_H_
#define CASPER_CEF3_BROWSER_MAIN_CONTEXT_TYPES_H_
#pragma once

namespace casper
{
    
    namespace cef3
    {
        
        namespace browser
        {
            
            typedef struct
            {
                
                typedef struct {
                    const std::string name_;
                    const std::string version_;
                } Product;
                
                typedef struct {
                    const std::string name_;
                    const std::string version_;
                } OS;
                
                typedef struct {
                    const cef_color_t background_color_;
                    const bool        use_views_;
                    const bool        use_windowless_rendering_;
                    const CefSize     minimum_size_;
                    const bool        allow_title_changes_;
                } Window;
                
                typedef struct {
                    const std::string main_url_;
                    const std::string user_agent_;
                    const Window      window_;
                    const bool        terminate_when_all_windows_closed_;
                } Application;
                
                
                typedef struct {
                    std::string app_working_directory_;
                    std::string logs_path_;
                    std::string downloads_path_;
                    std::string cache_path_;
                } Paths;
                
                
                const OS          os_;
                const Product     product_;
                const Application application_;
                
                Paths             paths_;
                
            } Settings;
            
        } // end of namespace 'browser'
        
    } // end of namespace 'cef3'
    
}  // end of namespace 'casper'

#endif // CASPER_CEF3_BROWSER_MAIN_CONTEXT_TYPES_H_

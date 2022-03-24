// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CASPER_CEF3_BROWSER_MAIN_CONTEXT_H_
#define CASPER_CEF3_BROWSER_MAIN_CONTEXT_H_
#pragma once

#include <string>

#include "include/base/cef_ref_counted.h"
#include "include/internal/cef_types_wrappers.h"
#include "include/base/cef_macros.h"


#include "cef3/browser/main_context_types.h"

namespace casper
{
    
    namespace cef3
    {
        
        
        namespace browser
        {
            
            class RootWindowManager;

            // Used to store global context in the browser process. The methods of this
            // class are thread-safe unless otherwise indicated.
            class MainContext
            {
            public:
                // Returns the singleton instance of this object.
                static MainContext* Get();
                
                // Returns the full path to the console log file.
                virtual std::string GetConsoleLogPath() = 0;
                
                // Returns the full path to |file_name|.
                virtual std::string GetDownloadPath(const std::string& file_name) = 0;
                
                // Returns the app working directory including trailing path separator.
                virtual std::string GetAppWorkingDirectory() = 0;
                
                // Returns the main application URL.
                virtual std::string GetMainURL() = 0;
                
                // Returns the background color.
                virtual cef_color_t GetBackgroundColor() = 0;
                
                // Returns true if the Views framework will be used.
                virtual bool UseViews() = 0;
                
                // Returns true if windowless (off-screen) rendering will be used.
                virtual bool UseWindowlessRendering() = 0;
                
                // Populate |settings| based on command-line arguments.
                virtual void PopulateSettings(CefSettings* settings) = 0;
                virtual void PopulateBrowserSettings(CefBrowserSettings* settings) = 0;
                //        virtual void PopulateOsrSettings(OsrRenderer::Settings* settings) = 0;
                
                // Returns the object used to create/manage RootWindow instances.
                virtual RootWindowManager* GetRootWindowManager() = 0;
                
            protected: // Data
                
                Settings settings_;
                
            protected:
                
                MainContext(const Settings& a_settings);
                virtual ~MainContext();
                
            public: // Const Method(s) / Function(s)
                
                const Settings& settings () const { return settings_; }
                                
            private:
                DISALLOW_COPY_AND_ASSIGN(MainContext);
            };
            
        } // end of namespace 'client'
        
    } // end of namespace 'cef3'
    
}  // end of namespace 'casper'

#endif // CASPER_CEF3_BROWSER_MAIN_CONTEXT_H_


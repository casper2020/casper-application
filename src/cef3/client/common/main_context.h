// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CASPER_CEF3_CLIENT_COMMON_MAIN_CONTEXT_H_
#define CASPER_CEF3_CLIENT_COMMON_MAIN_CONTEXT_H_

#pragma once

#include "include/cef_app.h"

#include "include/cef_command_line.h"

#include "include/base/cef_scoped_ptr.h"
#include "include/base/cef_thread_checker.h"

//#include "client/browser/osr_renderer.h"

#include "cef3/browser/main_context.h"

namespace casper
{
    
    namespace cef3
    {
        
        namespace browser
        {
            class RootWindowManager;
        }

        namespace client
        {
            
            namespace common
            {

                class MainContext : public ::casper::cef3::browser::MainContext
                {
                    
                private: // Data
                    
                    CefRefPtr<CefCommandLine> command_line_;

                private: // Status
                    
                    typedef struct {
                        
                        bool initialized_;
                        bool shutdown_;
                        
                        /*
                         * Returns true if the context is in a valid state (initialized and not yet shut down).
                         */
                        bool Valid() const { return initialized_ && !shutdown_; }
                        
                    } Status;
                    
                    Status status_;
                    
                private: // Helpers
                    
                    scoped_ptr<casper::cef3::browser::RootWindowManager> root_window_manager_;
                    base::ThreadChecker                                  thread_checker_;      //!< Used to verify that methods are called on the correct thread.

                    
                public: // Constructor(s) / Destructor
                    
                    MainContext (const ::casper::cef3::browser::Settings& a_settings, CefRefPtr<CefCommandLine> a_command_line);
                    virtual ~MainContext ();
                    
                 public: // Method(s) / Function(s) - from
                    
                    // MainContext members.
                    std::string GetConsoleLogPath       () OVERRIDE;
                    std::string GetDownloadPath         (const std::string& file_name) OVERRIDE;
                    std::string GetAppWorkingDirectory  () OVERRIDE;
                    std::string GetMainURL              () OVERRIDE;
                    cef_color_t GetBackgroundColor      () OVERRIDE;
                    
                    bool UseViews                       () OVERRIDE;
                    bool UseWindowlessRendering         () OVERRIDE;
                    
                    void PopulateSettings               (CefSettings* o_settings) OVERRIDE;
                    void PopulateBrowserSettings        (CefBrowserSettings* o_settings) OVERRIDE;
//                    void PopulateOsrSettings            (::client::OsrRenderer::Settings* o_settings) ;
                    
                    casper::cef3::browser::RootWindowManager* GetRootWindowManager() OVERRIDE;
                    
                public: // One-shot Call Method(s) / Function(s)
                    
                    virtual bool Initialize (const CefMainArgs& args, const CefSettings& settings, CefRefPtr<CefApp> application, void* windows_sandbox_info);
                    virtual void Shutdown   ();

                public: // API Method(s) / Function(s)

                    virtual bool GetDownloadURI   (const std::string& a_suggested_file_name, std::string& o_uri);

                public: // Pure Virtual Method(s) / Function(s)
                    
                    virtual bool GetLogsPath      (std::string& o_path) = 0;
                    virtual bool GetDownloadsPath (std::string& o_path) = 0;
                    
                private:
                    
                    DISALLOW_COPY_AND_ASSIGN(MainContext);
                    
                }; // end of class 'MainContext'
                
            } // end of namespace 'common'
            
        } // end of namespace 'client'
        
    } // end of namespace 'cef3'
    
}  // end of namespace 'casper'

#endif // CASPER_CEF3_CLIENT_COMMON_MAIN_CONTEXT_H_

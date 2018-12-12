// Copyright (c) 2016 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CASPER_CEF3_CLIENT_BROWSER_CLIENT_BROWSER_DELEGATE_H_
#define CASPER_CEF3_CLIENT_BROWSER_CLIENT_BROWSER_DELEGATE_H_
#pragma once

#include "include/cef_command_line.h"
#include "include/cef_crash_util.h"
#include "include/cef_file_util.h"

#include "cef3/client/browser/client_app_browser.h"

namespace casper
{
    
    namespace cef3
    {
        
        namespace client
        {
            
            namespace browser
            {
                
                class ClientBrowserDelegate : public ClientAppBrowser::Delegate {
                    
                public:
                    
                    ClientBrowserDelegate();
                    
                    void OnContextInitialized(CefRefPtr<ClientAppBrowser> app) OVERRIDE;
                    
                private:
                    DISALLOW_COPY_AND_ASSIGN(ClientBrowserDelegate);
                    IMPLEMENT_REFCOUNTING(ClientBrowserDelegate);
                };

                
            } // end of namespace 'browser'
            
        } // end of namespace 'client'
        
    } // end of namespace 'cef3'
    
} // end of namespace 'casper'

#endif // CASPER_CEF3_CLIENT_BROWSER_CLIENT_BROWSER_DELEGATE_H_

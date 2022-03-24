// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CASPER_CEF3_CLIENT_BROWSER_CLIENT_APP_BROWSER_H_
#define CASPER_CEF3_CLIENT_BROWSER_CLIENT_APP_BROWSER_H_
#pragma once

#include <set>

#include "cef3/common/app.h"

namespace casper
{
    
    namespace cef3
    {
        
        namespace client
        {
            
            namespace browser
            {
                
                // Client app implementation for the browser process.
                class ClientAppBrowser : public casper::cef3::common::App, public CefBrowserProcessHandler
                {
                public:
                    // Interface for browser delegates. All Delegates must be returned via
                    // CreateDelegates. Do not perform work in the Delegate
                    // constructor. See CefBrowserProcessHandler for documentation.
                    class Delegate : public virtual CefBaseRefCounted {
                    public:
                        virtual void OnBeforeCommandLineProcessing(
                                                                   CefRefPtr<ClientAppBrowser> app,
                                                                   CefRefPtr<CefCommandLine> command_line) {}
                        
                        virtual void OnContextInitialized(CefRefPtr<ClientAppBrowser> app) {}
                        
                        virtual void OnBeforeChildProcessLaunch(
                                                                CefRefPtr<ClientAppBrowser> app,
                                                                CefRefPtr<CefCommandLine> command_line) {}
                        
                        virtual void OnRenderProcessThreadCreated(
                                                                  CefRefPtr<ClientAppBrowser> app,
                                                                  CefRefPtr<CefListValue> extra_info) {}
                    };
                    
                    typedef std::set<CefRefPtr<Delegate>> DelegateSet;
                    
                    ClientAppBrowser();
                    
                private:
                    // Creates all of the Delegate objects. Implemented by cefclient in
                    // client_app_delegates_browser.cc
                    static void CreateDelegates(DelegateSet& delegates);
                    
                    // CefApp methods.
                    void OnBeforeCommandLineProcessing(
                                                       const CefString& process_type,
                                                       CefRefPtr<CefCommandLine> command_line) override;
                    CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
                        return this;
                    }
                    
                    // CefBrowserProcessHandler methods.
                    void OnContextInitialized() override;
                    void OnBeforeChildProcessLaunch(
                                                    CefRefPtr<CefCommandLine> command_line) override;
//                    void OnRenderProcessThreadCreated(
//                                                      CefRefPtr<CefListValue> extra_info) override;
//                    CefRefPtr<CefPrintHandler> GetPrintHandler() override {
//                        return print_handler_;
//                    }
                    void OnScheduleMessagePumpWork(int64 delay) override;
                    
                    // Set of supported Delegates.
                    DelegateSet delegates_;
                    
                    
                    IMPLEMENT_REFCOUNTING(ClientAppBrowser);
                    DISALLOW_COPY_AND_ASSIGN(ClientAppBrowser);
                };
                
            } // end of namespace 'browser'
            
        } // end of namespace 'client'
        
    } // end of namespace 'cef3'

} // end of namespace 'casper'

#endif  // CASPER_CEF3_CLIENT_BROWSER_CLIENT_APP_BROWSER_H_

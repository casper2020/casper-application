// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CASPER_CEF3_HELPER_COMMON_RENDERER_APP_H_
#define CASPER_CEF3_HELPER_COMMON_RENDERER_APP_H_

#pragma once

#include "cef3/common/app.h"

namespace casper
{
    
    namespace cef3
    {
        
        namespace helper
        {
            
            namespace common
            {
                
                class RendererApp : public casper::cef3::common::App, public CefRenderProcessHandler
                {

                public: // Constructor(s) / Destructor(s)
                    
                    RendererApp();

                public: // Inherited Method(s) / Function(s) - from CefApp
                    
                    CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() OVERRIDE {
                        return this;
                    }

                public: // Inherited Method(s) / Function(s) - from CefRenderProcessHandler

                    void OnRenderThreadCreated(CefRefPtr<CefListValue> extra_info) OVERRIDE;
                    void OnWebKitInitialized() OVERRIDE;
                    void OnBrowserCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
                    void OnBrowserDestroyed(CefRefPtr<CefBrowser> browser) OVERRIDE;
                    CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE;
                    void OnContextCreated(CefRefPtr<CefBrowser> browser,
                                          CefRefPtr<CefFrame> frame,
                                          CefRefPtr<CefV8Context> context) OVERRIDE;
                    void OnContextReleased(CefRefPtr<CefBrowser> browser,
                                           CefRefPtr<CefFrame> frame,
                                           CefRefPtr<CefV8Context> context) OVERRIDE;
                    void OnUncaughtException(CefRefPtr<CefBrowser> browser,
                                             CefRefPtr<CefFrame> frame,
                                             CefRefPtr<CefV8Context> context,
                                             CefRefPtr<CefV8Exception> exception,
                                             CefRefPtr<CefV8StackTrace> stackTrace) OVERRIDE;
                    void OnFocusedNodeChanged(CefRefPtr<CefBrowser> browser,
                                              CefRefPtr<CefFrame> frame,
                                              CefRefPtr<CefDOMNode> node) OVERRIDE;
                    bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                                  CefProcessId source_process,
                                                  CefRefPtr<CefProcessMessage> message) OVERRIDE;

                private:
                    
                    IMPLEMENT_REFCOUNTING(RendererApp);
                    DISALLOW_COPY_AND_ASSIGN(RendererApp);
                    
                }; // end of class 'RendererApp'
                
            } // end of namespace 'common'
            
        } // end of namespace 'helper'
        
    } // end of namespace 'cef3'
    
}  // end of namespace 'casper'

#endif // CASPER_CEF3_HELPER_COMMON_RENDERER_APP_H_

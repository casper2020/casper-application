// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CASPER_CEF3_CLIENT_COMMON_REQUEST_CONTEXT_HANDLER_H_
#define CASPER_CEF3_CLIENT_COMMON_REQUEST_CONTEXT_HANDLER_H_

#pragma once

#include "include/cef_request_context_handler.h" // CefRequestContextHandler

#include "cef3/browser/extension_handler.h"

#include "include/base/cef_ref_counted.h" // CefRefPtr
#include "include/cef_cookie.h"           // CefCookieManager

namespace casper
{
    
    namespace cef3
    {
        
        namespace browser
        {
            
            class RequestContextHandler : public CefRequestContextHandler, public ExtensionHandler
            {
                
            private: // Ptrs
                
                CefRefPtr<CefCookieManager> cookie_manager_;
                
            public: // Constructor(s) / Destructor
                
                RequestContextHandler ();
                
            public: // Overriden Method(s) / Function(s) - from CefRequestContextHandler
                
                // CefRequestContextHandler methods:
                
//                bool OnBeforePluginLoad(const CefString& mime_type,
//                                        const CefString& plugin_url,
//                                        bool is_main_frame,
//                                        const CefString& top_origin_url,
//                                        CefRefPtr<CefWebPluginInfo> plugin_info,
//                                        PluginPolicy* plugin_policy) override;
                
                void OnRequestContextInitialized(
                                                 CefRefPtr<CefRequestContext> request_context) override ;
                
//                CefRefPtr<CefCookieManager> GetCookieManager() override {
//                    return cookie_manager_;
//                }
                
            private:
                
                IMPLEMENT_REFCOUNTING(RequestContextHandler);
                DISALLOW_COPY_AND_ASSIGN(RequestContextHandler);
                
            }; // end of class 'RequestContextHandler'
            
        } // end of namespace 'browser'
        
    } // end of namespace 'cef3'
    
}  // end of namespace 'casper'

#endif // CASPER_CEF3_CLIENT_COMMON_REQUEST_CONTEXT_HANDLER_H_


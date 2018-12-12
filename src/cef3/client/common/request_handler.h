// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CASPER_CEF3_CLIENT_COMMON_REQUEST_HANDLER_H_
#define CASPER_CEF3_CLIENT_COMMON_REQUEST_HANDLER_H_

#pragma once

#include "include/cef_request_handler.h"          // CefRequestHandler

#include "include/base/cef_ref_counted.h"         // CefRefPtr

#include "include/wrapper/cef_message_router.h"   // CefMessageRouterBrowserSide

#include "include/wrapper/cef_resource_manager.h" // CefResourceManager

#include "include/cef_load_handler.h"             // ErrorCode

#include "cef3/client/common/base_handler.h"

namespace casper
{
    
    namespace cef3
    {
        
        namespace client
        {
            
            namespace common
            {
                
                class RequestHandler : public CefRequestHandler
                {
                    
                    IMPLEMENT_REFCOUNTING(RequestHandler);
                    
                protected: // Const Data
                    
                    // The startup URL.
                    const std::string startup_url_;

                    // True if this handler uses off-screen rendering.
                    const bool        is_osr_;
                    
                protected: // Ptrs
                    
                    CefRefPtr<BaseHandler> base_handler_;
                    
                public: // Constructor(s) / Destructor
                    
                    RequestHandler (const std::string&    a_startup_url,
                                    const bool&           a_is_osr,
                                    CefRefPtr<BaseHandler> a_base_handler);
                    virtual ~RequestHandler();
                    
                public: // CefRequestHandler Method(s) / Function(s)
                    
                    bool OnBeforeBrowse (CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                         CefRefPtr<CefRequest> request,
                                         bool user_gesture, bool is_redirect) OVERRIDE;
                    
                    bool OnOpenURLFromTab (CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                           const CefString& target_url,
                                           CefRequestHandler::WindowOpenDisposition target_disposition,
                                           bool user_gesture) OVERRIDE;
                    
                    cef_return_value_t            OnBeforeResourceLoad      (CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                                                             CefRefPtr<CefRequest> request,
                                                                             CefRefPtr<CefRequestCallback> callback) OVERRIDE;
                    
                    CefRefPtr<CefResourceHandler> GetResourceHandler        (CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                                                             CefRefPtr<CefRequest> request) OVERRIDE;
                    
                    CefRefPtr<CefResponseFilter>  GetResourceResponseFilter (CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                                                             CefRefPtr<CefRequest> request,
                                                                             CefRefPtr<CefResponse> response) OVERRIDE;
                    bool OnQuotaRequest(CefRefPtr<CefBrowser> browser,
                                        const CefString& origin_url,
                                        int64 new_size,
                                        CefRefPtr<CefRequestCallback> callback) OVERRIDE;
                    
                    void OnProtocolExecution(CefRefPtr<CefBrowser> browser,
                                             const CefString& url,
                                             bool& allow_os_execution) OVERRIDE;
                    
                    bool OnCertificateError(CefRefPtr<CefBrowser> browser,
                                            CefLoadHandler::ErrorCode cert_error,
                                            const CefString& request_url,
                                            CefRefPtr<CefSSLInfo> ssl_info,
                                            CefRefPtr<CefRequestCallback> callback) OVERRIDE;
                    
                    bool OnSelectClientCertificate(
                                                   CefRefPtr<CefBrowser> browser,
                                                   bool isProxy,
                                                   const CefString& host,
                                                   int port,
                                                   const X509CertificateList& certificates,
                                                   CefRefPtr<CefSelectClientCertificateCallback> callback) OVERRIDE;
                    
                    void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
                                                   TerminationStatus status) OVERRIDE;
                    
                }; // end of class 'RequestHandler'
                
            } // end of namespace 'common'
            
        } // end of namespace 'client'
        
    } // end of namespace 'cef3'
    
}  // end of namespace 'casper'

#endif // CASPER_CEF3_CLIENT_COMMON_REQUEST_HANDLER_H_

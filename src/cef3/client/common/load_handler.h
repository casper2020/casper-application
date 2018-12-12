// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CASPER_CEF3_CLIENT_COMMON_LOAD_HANDLER_H_
#define CASPER_CEF3_CLIENT_COMMON_LOAD_HANDLER_H_

#pragma once

#include "include/cef_load_handler.h"     // CefLoadHandler

#include "include/base/cef_ref_counted.h" // CefRefPtr

#include "cef3/client/common/base_handler.h"

namespace casper
{
    
    namespace cef3
    {
        
        namespace client
        {
            
            namespace common
            {
                
                class ClientHandlerDelegate;
                
                class LoadHandler : public CefLoadHandler
                {
                    
                    IMPLEMENT_REFCOUNTING(LoadHandler);
                    
                protected: // Ptrs
                    
                    CefRefPtr<BaseHandler> base_handler_;
                    
                public: // Constructor(s) / Destructor
                    
                    LoadHandler (CefRefPtr<BaseHandler> a_base_handler);
                    virtual ~LoadHandler ();

                public: // CefLoadHandler Method(s) / Function(s)
                    
                    void OnLoadingStateChange (CefRefPtr<CefBrowser> browser,
                                               bool isLoading, bool canGoBack, bool canGoForward) OVERRIDE;
                    
                    void OnLoadError          (CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                               CefLoadHandler::ErrorCode errorCode,
                                               const CefString& errorText,
                                               const CefString& failedUrl) OVERRIDE;
                    
                protected: //
                    
                    void NotifyLoadingState(bool isLoading, bool canGoBack, bool canGoForward);


                }; // end of class 'LoadHandler'
                
            } // end of namespace 'common'
            
        } // end of namespace 'client'
        
    } // end of namespace 'cef3'
    
}  // end of namespace 'casper'

#endif // CASPER_CEF3_CLIENT_COMMON_LOAD_HANDLER_H_

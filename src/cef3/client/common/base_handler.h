// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CASPER_CEF3_CLIENT_BASE_HANDLER_H_
#define CASPER_CEF3_CLIENT_BASE_HANDLER_H_

#pragma once

#include "include/cef_life_span_handler.h" // CefLifeSpanHandler

#include "include/base/cef_ref_counted.h" // CefRefPtr

#include "include/wrapper/cef_message_router.h"
#include "include/wrapper/cef_resource_manager.h"

#include <set>

namespace casper
{
    
    namespace cef3
    {
        
        namespace client
        {
            
            namespace common
            {
                
                class ClientHandlerDelegate;
                
                class BaseHandler : CefBaseRefCounted
                {
                    
                    IMPLEMENT_REFCOUNTING(BaseHandler);
                    
                public: // Ptrs
                    
                    // Handles the browser side of query routing. The renderer side is handled
                    // in client_renderer.cc.
                    CefRefPtr<CefMessageRouterBrowserSide> message_router_;
                    
                    // Manages the registration and delivery of resources.
                    CefRefPtr<CefResourceManager>          resource_manager_;
                    
                    //
                    ClientHandlerDelegate*                 delegate_ptr_;
                    
                public: // Constructor(s) / Destructor
                    
                    BaseHandler (CefRefPtr<CefMessageRouterBrowserSide> a_message_router,
                                 CefRefPtr<CefResourceManager> a_resource_manager,
                                 ClientHandlerDelegate* a_delegate_ptr);
                    virtual ~BaseHandler ();
                  
                }; // end of class 'BaseHandler'
                
            } // end of namespace 'common'
            
        } // end of namespace 'client'
        
    } // end of namespace 'cef3'
    
}  // end of namespace 'casper'

#endif // CASPER_CEF3_CLIENT_BASE_HANDLER_H_

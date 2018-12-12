// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CASPER_CEF3_CLIENT_COMMON_CONTEXT_MENU_HANDLER_H_
#define CASPER_CEF3_CLIENT_COMMON_CONTEXT_MENU_HANDLER_H_

#pragma once

#include "include/cef_context_menu_handler.h" // CefContextMenuHandler

#include "include/base/cef_ref_counted.h"     // CefRefPtr

#include "cef3/client/common/base_handler.h"

namespace casper
{
    
    namespace cef3
    {
        
        namespace client
        {
            
            namespace common
            {
                
                class ContextMenuHandler : public CefContextMenuHandler
                {
                    
                    IMPLEMENT_REFCOUNTING(ContextMenuHandler);
                                        
                protected: // Ptrs
                    
                    CefRefPtr<BaseHandler> base_handler_;
                    
                public: // Constructor(s) / Destructor
                    
                    ContextMenuHandler (CefRefPtr<BaseHandler> a_base_handler);
                    virtual ~ContextMenuHandler();
                    
                public: // CefContextMenuHandler Method(s) / Function(s)
                    
                    void OnBeforeContextMenu  (CefRefPtr<CefBrowser> a_browser, CefRefPtr<CefFrame> a_frame,
                                               CefRefPtr<CefContextMenuParams> a_params, CefRefPtr<CefMenuModel> a_model) OVERRIDE;
                    bool OnContextMenuCommand (CefRefPtr<CefBrowser> a_browser, CefRefPtr<CefFrame> a_frame,
                                               CefRefPtr<CefContextMenuParams> a_params, int a_command_id, EventFlags a_event_flags) OVERRIDE;
                    
                }; // end of class 'RequestHandler'
                
            } // end of namespace 'common'
            
        } // end of namespace 'client'
        
    } // end of namespace 'cef3'
    
}  // end of namespace 'casper'

#endif // CASPER_CEF3_CLIENT_COMMON_CONTEXT_MENU_HANDLER_H_

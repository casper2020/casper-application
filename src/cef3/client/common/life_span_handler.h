// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CASPER_CEF3_CLIENT_LIFE_SPAN_HANDLER_H_
#define CASPER_CEF3_CLIENT_LIFE_SPAN_HANDLER_H_

#pragma once

#include "include/cef_life_span_handler.h" // CefLifeSpanHandler

#include "include/base/cef_ref_counted.h" // CefRefPtr

#include "include/wrapper/cef_resource_manager.h" // CefResourceManager

#include "include/wrapper/cef_message_router.h"   //

#include "cef3/client/common/base_handler.h"

#include <set>

namespace casper
{
    
    namespace cef3
    {
        
        namespace client
        {
            
            namespace common
            {
                
                class LifeSpanHandler : public CefLifeSpanHandler
                {
                    
                     IMPLEMENT_REFCOUNTING(LifeSpanHandler);
                    
                protected: // Const Data
                    
                    const bool is_osr_;
                    
                protected: // Ptrs
                    
                    CefRefPtr<BaseHandler> base_handler_;
                    
                protected: // Data
                    
                    // The current number of browsers using this handler.
                    int browser_count_;
                    
                    // True if mouse cursor change is disabled.
                    bool mouse_cursor_change_disabled_;
                    
                    // Set of Handlers registered with the message router.
                    typedef std::set<CefMessageRouterBrowserSide::Handler*> MessageHandlerSet;
                    MessageHandlerSet                                       message_handler_set_;
                    
                public: // Constructor(s) / Destructor
                    
                    LifeSpanHandler (const bool& a_is_osr, CefRefPtr<BaseHandler> a_base_handler);
                    virtual ~LifeSpanHandler ();
                    
                public: // CefLifeSpanHandler Method(s) / Function(s)
                    
                    bool OnBeforePopup(
                                       CefRefPtr<CefBrowser> browser,
                                       CefRefPtr<CefFrame> frame,
                                       const CefString& target_url,
                                       const CefString& target_frame_name,
                                       CefLifeSpanHandler::WindowOpenDisposition target_disposition,
                                       bool user_gesture,
                                       const CefPopupFeatures& popupFeatures,
                                       CefWindowInfo& windowInfo,
                                       CefRefPtr<CefClient>& client,
                                       CefBrowserSettings& settings,
                                       CefRefPtr<CefDictionaryValue>& extra_info,
                                       bool* no_javascript_access) override;
                    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
                    bool DoClose(CefRefPtr<CefBrowser> browser) override;
                    void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;
                    
                protected:
                    
                    // Create a new popup window using the specified information. |is_devtools|
                    // will be true if the window will be used for DevTools. Return true to
                    // proceed with popup browser creation or false to cancel the popup browser.
                    // May be called on any thead.
                    bool CreatePopupWindow(CefRefPtr<CefBrowser> browser,
                                           bool is_devtools,
                                           const CefPopupFeatures& popupFeatures,
                                           CefWindowInfo& windowInfo,
                                           CefRefPtr<CefClient>& client,
                                           CefBrowserSettings& settings);
                    
                protected: // Execute Delegate notifications on the main thread.
            
                    void NotifyBrowserCreated(CefRefPtr<CefBrowser> browser);
                    void NotifyBrowserClosing(CefRefPtr<CefBrowser> browser);
                    void NotifyBrowserClosed(CefRefPtr<CefBrowser> browser);

                }; // end of class 'LifeSpanHandler'
                
            } // end of namespace 'common'
            
        } // end of namespace 'client'
        
    } // end of namespace 'cef3'
    
}  // end of namespace 'casper'

#endif // CASPER_CEF3_CLIENT_LIFE_SPAN_HANDLER_H_

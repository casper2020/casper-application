// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CASPER_CEF3_CLIENT_COMMON_CLIENT_HANDLER_H_
#define CASPER_CEF3_CLIENT_COMMON_CLIENT_HANDLER_H_

#pragma once

#include "include/cef_client.h" // CefClient

#include "cef3/client/common/life_span_handler.h"
#include "cef3/client/common/request_handler.h"
#include "cef3/client/common/context_menu_handler.h"
#include "cef3/client/common/download_handler.h"
#include "cef3/client/common/load_handler.h"
#include "cef3/client/common/display_handler.h"
#include "cef3/client/common/focus_handler.h"
#include "cef3/client/common/keyboard_handler.h"
#include "cef3/client/common/drag_handler.h"

#include "include/wrapper/cef_resource_manager.h"

#include <string>
#include <set>

namespace casper
{
    
    namespace cef3
    {   
        
        namespace client
        {
        
            namespace common
            {
                
                // Implement this interface to receive notification of ClientHandler
                // events. The methods of this class will be called on the main thread unless
                // otherwise indicated.
                class ClientHandlerDelegate
                {
                public:
                    // Called when the browser is created.
                    virtual void OnBrowserCreated(CefRefPtr<CefBrowser> browser) = 0;
                    
                    // Called when the browser is closing.
                    virtual void OnBrowserClosing(CefRefPtr<CefBrowser> browser) = 0;
                    
                    // Called when the browser has been closed.
                    virtual void OnBrowserClosed(CefRefPtr<CefBrowser> browser) = 0;
                    
                    // Set the window URL address.
                    virtual void OnSetAddress(const std::string& url) = 0;
                    
                    // Set the window title.
                    virtual void OnSetTitle(const std::string& title) = 0;
                    
                    // Set the Favicon image.
                    virtual void OnSetFavicon(CefRefPtr<CefImage> image) {}
                    
                    // Set fullscreen mode.
                    virtual void OnSetFullscreen(bool fullscreen) = 0;
                    
                    // Auto-resize contents.
                    virtual void OnAutoResize(const CefSize& new_size) = 0;
                    
                    // Set the loading state.
                    virtual void OnSetLoadingState(bool isLoading,
                                                   bool canGoBack,
                                                   bool canGoForward) = 0;
                    
                    // Set the draggable regions.
                    virtual void OnSetDraggableRegions(
                                                       const std::vector<CefDraggableRegion>& regions) = 0;
                    
                    // Set focus to the next/previous control.
                    virtual void OnTakeFocus(bool next) {}
                    
                    // Called on the UI thread before a context menu is displayed.
                    virtual void OnBeforeContextMenu(CefRefPtr<CefMenuModel> model) {}
                    
                protected:
                    
                    virtual ~ClientHandlerDelegate() {}
                    
                };
                
                class ClientHandler :  public CefClient /*, public LifeSpanHandler, public DisplayHandler,
                                                         public ContextMenuHandler,
                                                         public RequestHandler, public DownloadHandler,
                                                         public KeyboardHandler, public LoadHandler, public FocusHandler, public DragHandler*/
                {
                    
                private: // MAIN THREAD ACCESS ONLY
                    
                    ClientHandlerDelegate* delegate_;
                    
                private: //
                    
                    
                    CefRefPtr<CefResourceManager> resource_manager_;
                    CefRefPtr<BaseHandler>        base_handler_;                    
                    CefRefPtr<LifeSpanHandler>    life_span_manager_;
                    CefRefPtr<DisplayHandler>     display_handler_;
                    CefRefPtr<ContextMenuHandler> context_menu_handler_;
                    CefRefPtr<RequestHandler>     request_handler_;
                    CefRefPtr<DownloadHandler>    download_handler_;
                    CefRefPtr<KeyboardHandler>    keyboard_handler_;
                    CefRefPtr<LoadHandler>        load_handler_;
                    CefRefPtr<FocusHandler>       focus_handler_;
                    CefRefPtr<DragHandler>        drag_handler_;
                    
                public: // Constructor(s) / Destructor
                    
                    ClientHandler (const ::std::string& a_startup_url, const bool& a_is_osr,
                                   casper::cef3::client::common::ClientHandlerDelegate* a_delegate);
                    virtual ~ClientHandler();
                    
                public: // CefClient Method(s) / Function(s)
                    
                    CefRefPtr<CefLifeSpanHandler>    GetLifeSpanHandler    () OVERRIDE { return life_span_manager_; }
                    CefRefPtr<CefDisplayHandler>     GetDisplayHandler     () OVERRIDE { return display_handler_; }
                    CefRefPtr<CefContextMenuHandler> GetContextMenuHandler () OVERRIDE { return context_menu_handler_; }
                    
                    CefRefPtr<CefRequestHandler>    GetRequestHandler      () OVERRIDE { return request_handler_; }
                    CefRefPtr<CefDownloadHandler>   GetDownloadHandler     () OVERRIDE { return download_handler_; }
                    
                    CefRefPtr<CefDragHandler>       GetDragHandler         () OVERRIDE { return drag_handler_; }
                    CefRefPtr<CefKeyboardHandler>   GetKeyboardHandler     () OVERRIDE { return keyboard_handler_; }
                    CefRefPtr<CefFocusHandler>      GetFocusHandler        () OVERRIDE { return focus_handler_; }
                    CefRefPtr<CefLoadHandler>       GetLoadHandler         () OVERRIDE { return load_handler_; }
                
                public:
                    
                    // This object may outlive the Delegate object so it's necessary for the
                    // Delegate to detach itself before destruction.
                    void DetachDelegate();
                    
                public: // CW: CEF Factory Static Helpers - declaration
                    
                    static CefRefPtr<ClientHandler> Factory (const ::std::string& a_startup_url, const bool& a_is_osr,
                                                             void* a_delegate);

                    static CefRefPtr<ContextMenuHandler> ContextMenuHandlerFactory (CefRefPtr<BaseHandler> a_base_handler);

                    static void SetupResourceManager (CefRefPtr<CefResourceManager> a_resource_manager);

                private: // Include the default reference counting implementation.
                    
                    IMPLEMENT_REFCOUNTING(ClientHandler);
                    DISALLOW_COPY_AND_ASSIGN(ClientHandler);
                    
                }; // end of class 'ClientHandler'
                                
            } // end of namespace 'common'
            
        } // end of namespace 'client'
        
    } // end of namespace 'cef3'
    
}  // end of namespace 'casper'

#endif // CASPER_CEF3_CLIENT_COMMON_CLIENT_HANDLER_H_


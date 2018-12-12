// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef3/browser/mac/browser_window_std_mac.h"

#include "cef3/client/common/client_handler.h"
#include "cef3/client/common/main_context.h"

#include <Cocoa/Cocoa.h>

#include "include/base/cef_logging.h"
#include "include/wrapper/cef_helpers.h" // CEF_REQUIRE_UI_THREAD

#include "cef3/browser/main_message_loop.h"

casper::cef3::browser::BrowserWindowStdMAC::BrowserWindowStdMAC (casper::cef3::browser::BrowserWindowStdMAC::Delegate* a_delegate, const ::std::string& a_startup_url)
    : casper::cef3::browser::BrowserWindow(a_delegate) , startup_url_(a_startup_url)
{
    client_handler_ = casper::cef3::client::common::ClientHandler::Factory(a_startup_url, /* a_is_osr */ false, /* a_delegate */ this);
}

void casper::cef3::browser::BrowserWindowStdMAC::CreateBrowser (ClientWindowHandle parent_handle,
                                                                const CefRect& rect, const CefBrowserSettings& settings,
                                                                CefRefPtr<CefRequestContext> request_context)
{
    REQUIRE_MAIN_THREAD();
    
    CefWindowInfo window_info;
    window_info.SetAsChild(parent_handle, rect.x, rect.y, rect.width, rect.height);
    
    
    CefBrowserHost::CreateBrowser(window_info, client_handler_, startup_url_, settings, request_context);
}

void casper::cef3::browser::BrowserWindowStdMAC::GetPopupConfig(CefWindowHandle temp_handle,
                                                              CefWindowInfo& windowInfo,
                                                              CefRefPtr<CefClient>& client,
                                                              CefBrowserSettings& settings)
{
    CEF_REQUIRE_UI_THREAD();
    
    // The window will be properly sized after the browser is created.
    windowInfo.SetAsChild(temp_handle, 0, 0, 0, 0);
    client = client_handler_;
}

void casper::cef3::browser::BrowserWindowStdMAC::ShowPopup(ClientWindowHandle parent_handle,
                                                         int x,
                                                         int y,
                                                         size_t width,
                                                         size_t height)
{
    REQUIRE_MAIN_THREAD();
    
    NSView* browser_view = GetWindowHandle();
    
    // Re-parent |browser_view| to |parent_handle|.
    [browser_view removeFromSuperview];
    [parent_handle addSubview:browser_view];
    
    NSSize size = NSMakeSize(static_cast<int>(width), static_cast<int>(height));
    [browser_view setFrameSize:size];
}

void casper::cef3::browser::BrowserWindowStdMAC::Show ()
{
    REQUIRE_MAIN_THREAD();
    // Nothing to do here. Chromium internally handles window show/hide.
}

void casper::cef3::browser::BrowserWindowStdMAC::Hide ()
{
    REQUIRE_MAIN_THREAD();
    /* nothing to do here, chromium internally handles window focus assignment */
}

void casper::cef3::browser::BrowserWindowStdMAC::SetBounds (int /* x */, int /* y */, size_t /* width */, size_t /* height */)
{
    REQUIRE_MAIN_THREAD();
    /* nothing to do here, chromium internally handles window focus assignment */
}

void casper::cef3::browser::BrowserWindowStdMAC::SetFocus(bool /* a_focus */)
{
    REQUIRE_MAIN_THREAD();
    /* nothing to do here, chromium internally handles window focus assignment */
}

ClientWindowHandle casper::cef3::browser::BrowserWindowStdMAC::GetWindowHandle() const
{
    REQUIRE_MAIN_THREAD();
    
    if ( browser_ ) {
        return browser_->GetHost()->GetWindowHandle();
    }
    return nullptr;
}


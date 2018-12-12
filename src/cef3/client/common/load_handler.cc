// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef3/client/common/load_handler.h"

#include "include/wrapper/cef_helpers.h" // CEF_REQUIRE_UI_THREAD

#include "cef3/browser/main_message_loop.h" // CURRENTLY_ON_MAIN_THREAD, MAIN_POST_CLOSURE

#include "cef3/client/common/client_handler.h" // ClientHandlerDelegate

casper::cef3::client::common::LoadHandler::LoadHandler (CefRefPtr<casper::cef3::client::common::BaseHandler> a_base_handler)
    : base_handler_(a_base_handler)
{
    /* empty */
}

casper::cef3::client::common::LoadHandler::~LoadHandler ()
{
    /* empty */
}

#ifdef __APPLE__
#pragma mark - CefLoadHandler
#endif

void casper::cef3::client::common::LoadHandler::OnLoadingStateChange (CefRefPtr<CefBrowser> browser,
                                                                      bool isLoading, bool canGoBack, bool canGoForward)
{
    CEF_REQUIRE_UI_THREAD();
    
    NotifyLoadingState(isLoading, canGoBack, canGoForward);
}

void casper::cef3::client::common::LoadHandler::OnLoadError (CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                                             ErrorCode errorCode,
                                                             const CefString& errorText, const CefString& failedUrl)
{
    CEF_REQUIRE_UI_THREAD();
    
    // Don't display an error for downloaded files.
    if ( errorCode == ERR_ABORTED )
        return;
    
    // Don't display an error for external protocols that we allow the OS to
    // handle. See OnProtocolExecution().
    if ( errorCode == ERR_UNKNOWN_URL_SCHEME ) {
        // CW NOW - HANDLE WITH INTERNAL SCHEMES
        return;
    }
    
    // Load the error page.
    // TODO CW - DISPLAY ERROR : LoadErrorPage(frame, failedUrl, errorCode, errorText); ??
}


void casper::cef3::client::common::LoadHandler::NotifyLoadingState (bool isLoading, bool canGoBack, bool canGoForward)
{
    if (!CURRENTLY_ON_MAIN_THREAD()) {
        // Execute this method on the main thread.
        MAIN_POST_CLOSURE(base::Bind(&casper::cef3::client::common::LoadHandler::NotifyLoadingState, this, isLoading, canGoBack, canGoForward));
        return;
    }
    
    if ( nullptr != base_handler_->delegate_ptr_ ) {
        base_handler_->delegate_ptr_->OnSetLoadingState(isLoading, canGoBack, canGoForward);
    }
}

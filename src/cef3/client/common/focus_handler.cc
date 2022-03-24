// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef3/client/common/focus_handler.h"

#include "include/wrapper/cef_helpers.h" // CEF_REQUIRE_UI_THREAD

#include "cef3/browser/main_message_loop.h" // CURRENTLY_ON_MAIN_THREAD, MAIN_POST_CLOSURE

#include "cef3/client/common/client_handler.h" // ClientHandlerDelegate

casper::cef3::client::common::FocusHandler::FocusHandler (CefRefPtr<casper::cef3::client::common::BaseHandler> a_base_handler)
    : base_handler_(a_base_handler)
{
    /* empty */
}

casper::cef3::client::common::FocusHandler::~FocusHandler ()
{
    /* empty */
}

#ifdef __APPLE__
#pragma mark - CefFocusHandler
#endif

void casper::cef3::client::common::FocusHandler::OnTakeFocus(CefRefPtr<CefBrowser> browser, bool next)
{
    CEF_REQUIRE_UI_THREAD();
    
    NotifyTakeFocus(next);
}

#ifdef __APPLE__
#pragma mark -
#endif

void casper::cef3::client::common::FocusHandler::NotifyTakeFocus (bool next)
{
    if (!CURRENTLY_ON_MAIN_THREAD()) {
        // Execute this method on the main thread.
        MAIN_POST_CLOSURE(base::BindOnce(&casper::cef3::client::common::FocusHandler::NotifyTakeFocus, this, next));
        return;
    }
    
    if ( nullptr != base_handler_->delegate_ptr_ ) {
        base_handler_->delegate_ptr_->OnTakeFocus(next);
    }
}

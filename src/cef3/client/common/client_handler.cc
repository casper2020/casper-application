// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef3/client/common/client_handler.h"

#include "cef3/browser/main_message_loop.h"

#include "cef3/client/common/main_context.h"
#include "cef3/client/common/main_context.h"

#include "cef3/browser/root_window_manager.h"

#include "cef3/shared/browser/utils/extension_util.h"

#include "include/wrapper/cef_helpers.h" // CEF_REQUIRE_UI_THREAD

#include "cef3/common/client/switches.h"

/**
 * @brief Default constructor.
 *
 * @param a_startup_url
 * @param a_is_osr
 * @param a_delegate
 */
casper::cef3::client::common::ClientHandler::ClientHandler  (const ::std::string& a_startup_url, const bool& a_is_osr,
                                                             casper::cef3::client::common::ClientHandlerDelegate* a_delegate)
{
    resource_manager_     = new CefResourceManager(); SetupResourceManager(resource_manager_);
    base_handler_         = new casper::cef3::client::common::BaseHandler(/* a_message_router */ nullptr, resource_manager_, a_delegate);
    life_span_manager_    = new casper::cef3::client::common::LifeSpanHandler(a_is_osr, base_handler_);
    display_handler_      = new casper::cef3::client::common::DisplayHandler(base_handler_);
    context_menu_handler_ = ContextMenuHandlerFactory(base_handler_);
    request_handler_      = new casper::cef3::client::common::RequestHandler(a_startup_url, a_is_osr, base_handler_);
    download_handler_     = new casper::cef3::client::common::DownloadHandler();
    keyboard_handler_     = new casper::cef3::client::common::KeyboardHandler();
    load_handler_         = new casper::cef3::client::common::LoadHandler(base_handler_);
    focus_handler_        = new casper::cef3::client::common::FocusHandler(base_handler_);
    drag_handler_         = new casper::cef3::client::common::DragHandler(base_handler_);
    delegate_             = a_delegate;
}

/**
 * @brief Destructor.
 */
casper::cef3::client::common::ClientHandler::~ClientHandler ()
{
    /* empty */
}

#ifdef __APPLE__
#pragma mark -
#endif

void casper::cef3::client::common::ClientHandler::DetachDelegate ()
{
    if (!CURRENTLY_ON_MAIN_THREAD()) {
        // Execute this method on the main thread.
        MAIN_POST_CLOSURE(base::Bind(&casper::cef3::client::common::ClientHandler::DetachDelegate, this));
        return;
    }
    
    DCHECK(delegate_);
    base_handler_->delegate_ptr_ = nullptr;
    delegate_ = nullptr;
}

// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef3/client/common/life_span_handler.h"

#include "include/wrapper/cef_helpers.h" // CEF_REQUIRE_UI_THREAD

#include "cef3/browser/main_message_loop.h" // CURRENTLY_ON_MAIN_THREAD, MAIN_POST_CLOSURE

#include "include/cef_command_line.h"     // CefCommandLine

#include "cef3/client/common/client_handler.h" // ClientHandlerDelegate

#include "cef3/browser/main_context.h"
#include "cef3/browser/root_window_manager.h"

#include "cef3/common/client/switches.h"
#include "cef3/shared/browser/utils/extension_util.h"

casper::cef3::client::common::LifeSpanHandler::LifeSpanHandler (const bool& a_is_osr,
                                                                CefRefPtr<casper::cef3::client::common::BaseHandler> a_base_handler)
    : is_osr_(a_is_osr), base_handler_(a_base_handler)
{
    // Read command line settings.
    const CefRefPtr<CefCommandLine> command_line = CefCommandLine::GetGlobalCommandLine();

    browser_count_                = 0;
    mouse_cursor_change_disabled_ = command_line->HasSwitch(casper::cef3::common::client::switches::kMouseCursorChangeDisabled);
}

casper::cef3::client::common::LifeSpanHandler::~LifeSpanHandler ()
{
    /* empty */
}

#ifdef __APPLE__
#pragma mark - CefLifeSpanHandler
#endif

bool casper::cef3::client::common::LifeSpanHandler::OnBeforePopup (CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                                                   const CefString& target_url, const CefString& target_frame_name,
                                                                   CefLifeSpanHandler::WindowOpenDisposition target_disposition,
                                                                   bool user_gesture,
                                                                   const CefPopupFeatures& popupFeatures,
                                                                   CefWindowInfo& windowInfo,
                                                                   CefRefPtr<CefClient>& client,
                                                                   CefBrowserSettings& settings,
                                                                   bool* no_javascript_access)
{
    CEF_REQUIRE_UI_THREAD();
    
    // Return true to cancel the popup window.
    return !CreatePopupWindow(browser, false, popupFeatures, windowInfo, client, settings);
}

void casper::cef3::client::common::LifeSpanHandler::OnAfterCreated (CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();
    
    browser_count_++;
    
    if (!base_handler_->message_router_) {
        // Create the browser-side router for query handling.
        CefMessageRouterConfig config;
        base_handler_->message_router_ = CefMessageRouterBrowserSide::Create(config);
        
        // Register handlers with the router.
        MessageHandlerSet::const_iterator it = message_handler_set_.begin();
        for (; it != message_handler_set_.end(); ++it)
            base_handler_->message_router_->AddHandler(*(it), false);
    }
    
    // Disable mouse cursor change if requested via the command-line flag.
    if (mouse_cursor_change_disabled_)
        browser->GetHost()->SetMouseCursorChangeDisabled(true);
    
    // TODO CW - WINDOW SIZE
    if (browser->GetHost()->GetExtension()) {
        // Browsers hosting extension apps should auto-resize.
        browser->GetHost()->SetAutoResizeEnabled(true, CefSize(20, 20), CefSize(1000, 1000));
        
        CefRefPtr<CefExtension> extension = browser->GetHost()->GetExtension();
        if ( casper::cef3::shared::browser::utils::extension::IsInternalExtension(extension->GetPath()) ) {
            // Register the internal handler for extension resources.
            casper::cef3::shared::browser::utils::extension::AddInternalExtensionToResourceManager(extension, base_handler_->resource_manager_);
        }
    }
    
    NotifyBrowserCreated(browser);
}

bool casper::cef3::client::common::LifeSpanHandler::DoClose (CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();
    
    NotifyBrowserClosing(browser);
    
    // Allow the close. For windowed browsers this will result in the OS close
    // event being sent.
    return false;
}

void casper::cef3::client::common::LifeSpanHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();
    
    if (--browser_count_ == 0) {
        // Remove and delete message router handlers.
        MessageHandlerSet::const_iterator it = message_handler_set_.begin();
        for (; it != message_handler_set_.end(); ++it) {
            base_handler_->message_router_->RemoveHandler(*(it));
            delete *(it);
        }
        message_handler_set_.clear();
        base_handler_->message_router_ = NULL;
    }
    
    NotifyBrowserClosed(browser);
}

#ifdef __APPLE__
#pragma mark -
#endif

bool casper::cef3::client::common::LifeSpanHandler::CreatePopupWindow (CefRefPtr<CefBrowser> browser,
                                                                     bool is_devtools,
                                                                     const CefPopupFeatures& popupFeatures,
                                                                     CefWindowInfo& windowInfo,
                                                                     CefRefPtr<CefClient>& client,
                                                                     CefBrowserSettings& settings)
{
    CEF_REQUIRE_UI_THREAD();
    
    // The popup browser will be parented to a new native window.
    // Don't show URL bar and navigation buttons on DevTools windows.
    casper::cef3::browser::MainContext::Get()->GetRootWindowManager()->CreateRootWindowAsPopup(!is_devtools, is_osr_, popupFeatures, windowInfo, client, settings);
    
    return true;
}

#ifdef __APPLE__
#pragma mark -
#endif

void casper::cef3::client::common::LifeSpanHandler::NotifyBrowserCreated (CefRefPtr<CefBrowser> browser)
{
    if (!CURRENTLY_ON_MAIN_THREAD()) {
        // Execute this method on the main thread.
        MAIN_POST_CLOSURE(base::Bind(&casper::cef3::client::common::LifeSpanHandler::NotifyBrowserCreated, this, browser));
        return;
    }
    
    if ( base_handler_->delegate_ptr_ )
        base_handler_->delegate_ptr_->OnBrowserCreated(browser);
}


void casper::cef3::client::common::LifeSpanHandler::NotifyBrowserClosing (CefRefPtr<CefBrowser> browser)
{
    if (!CURRENTLY_ON_MAIN_THREAD()) {
        // Execute this method on the main thread.
        MAIN_POST_CLOSURE(base::Bind(&casper::cef3::client::common::LifeSpanHandler::NotifyBrowserClosing, this, browser));
        return;
    }
    
    if ( base_handler_->delegate_ptr_ )
        base_handler_->delegate_ptr_->OnBrowserClosing(browser);
}

void casper::cef3::client::common::LifeSpanHandler::NotifyBrowserClosed (CefRefPtr<CefBrowser> browser)
{
    if (!CURRENTLY_ON_MAIN_THREAD()) {
        // Execute this method on the main thread.
        MAIN_POST_CLOSURE(base::Bind(&casper::cef3::client::common::LifeSpanHandler::NotifyBrowserClosed, this, browser));
        return;
    }
    
    if ( base_handler_->delegate_ptr_ )
        base_handler_->delegate_ptr_ ->OnBrowserClosed(browser);
}

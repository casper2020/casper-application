// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef3/browser/extension_handler.h"

#include "cef3/browser/main_context.h"
#include "cef3/browser/root_window_manager.h"

#include "cef3/common/client/switches.h"

#include "include/wrapper/cef_helpers.h" // CEF_REQUIRE_UI_THREAD

#ifdef __APPLE__
#pragma mark - ExtensionHandler
#endif

/**
 * @brief Default constructor.
 */
casper::cef3::browser::ExtensionHandler::ExtensionHandler ()
{
    /* empty */
}

#ifdef __APPLE__
#pragma mark - CefExtensionHandler
#endif

void casper::cef3::browser::ExtensionHandler::OnExtensionLoaded (CefRefPtr<CefExtension> extension)
{
    CEF_REQUIRE_UI_THREAD();
    casper::cef3::browser::MainContext::Get()->GetRootWindowManager()->AddExtension(extension);
}

CefRefPtr<CefBrowser> casper::cef3::browser::ExtensionHandler::GetActiveBrowser (CefRefPtr<CefExtension> extension, CefRefPtr<CefBrowser> browser,
                                                                                 bool include_incognito)
{
    CEF_REQUIRE_UI_THREAD();

    // Return the browser for the active/foreground window.
    const CefRefPtr<CefBrowser> active_browser = casper::cef3::browser::MainContext::Get()->GetRootWindowManager()->GetActiveBrowser();
    if ( !active_browser ) {
        LOG(WARNING)
        << "No active browser available for extension "
        << browser->GetHost()->GetExtension()->GetIdentifier().ToString();
    } else {
        // The active browser should not be hosting an extension.
        DCHECK(!active_browser->GetHost()->GetExtension());
    }
    return active_browser;
}

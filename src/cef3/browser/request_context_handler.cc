// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef3/browser/request_context_handler.h"

#include "include/cef_command_line.h"

#include "cef3/common/client/switches.h"

#include "include/wrapper/cef_helpers.h" // CEF_REQUIRE_UI_THREAD

#include "cef3/browser/main_context.h"
#include "cef3/browser/root_window_manager.h"

#include "cef3/shared/browser/utils/extension_util.h"

#ifdef __APPLE__
#pragma mark - RequestContextHandler
#endif

/**
 * @brief Default constructor.
 */
casper::cef3::browser::RequestContextHandler::RequestContextHandler ()
{
//    const CefRefPtr<CefCommandLine> command_line = CefCommandLine::GetGlobalCommandLine();
//    if ( command_line->HasSwitch(casper::cef3::common::client::switches::kRequestContextBlockCookies ) ) {
//        // Use a cookie manager that neither stores nor retrieves cookies.
//        cookie_manager_ = CefCookieManager::GetBlockingManager();
//    } else {
//        // TODO CW
//        cookie_manager_ = CefCookieManager::CreateManager("/tmp/", /* persist_session_cookies */ true,
//                                                          /* CefRefPtr<CefCompletionCallback */ nullptr
//        );
//    }
}

#ifdef __APPLE__
#pragma mark - CefRequestContextHandler
#endif

//bool casper::cef3::browser::RequestContextHandler::OnBeforePluginLoad (const CefString& mime_type,
//                                                                              const CefString &plugin_url,
//                                                                              bool is_main_frame,
//                                                                              const CefString &top_origin_url,
//                                                                              CefRefPtr<CefWebPluginInfo> plugin_info, PluginPolicy *plugin_policy)
//{
//    // Always allow the PDF plugin to load.
//    if ( *plugin_policy != PLUGIN_POLICY_ALLOW && mime_type == "application/pdf" ) {
//        *plugin_policy = PLUGIN_POLICY_ALLOW;
//        return true;
//    }
//
//    return false;
//}


// The example extension loading implementation requires all browsers to
// share the same request context.

void casper::cef3::browser::RequestContextHandler::OnRequestContextInitialized (CefRefPtr<CefRequestContext> request_context)
{
    CEF_REQUIRE_UI_THREAD();
    
    const CefRefPtr<CefCommandLine> command_line = CefCommandLine::GetGlobalCommandLine();
    if ( command_line->HasSwitch(casper::cef3::common::client::switches::kLoadExtension) ) {
        
        if ( casper::cef3::browser::MainContext::Get()->GetRootWindowManager()->request_context_per_browser() ) {
            LOG(ERROR)
            << "Cannot mix --load-extension and --request-context-per-browser";
            return;
        }
        
        // Load one or more extension paths specified on the command-line and delimited with semicolon.
        const std::string& extension_path = command_line->GetSwitchValue(casper::cef3::common::client::switches::kLoadExtension);
        if ( !extension_path.empty() ) {
            std::string part;
            std::istringstream f(extension_path);
            while ( getline(f, part, ';') ) {
                if ( !part.empty() ) {
                    casper::cef3::shared::browser::utils::extension::LoadExtension(request_context, part, this);
                }
            }
        }
    }
}


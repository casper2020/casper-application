// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef3/client/common/context_menu_handler.h"

#include "include/wrapper/cef_helpers.h" // CEF_REQUIRE_UI_THREAD

casper::cef3::client::common::ContextMenuHandler::ContextMenuHandler (CefRefPtr<casper::cef3::client::common::BaseHandler> a_base_handler)
    : base_handler_(a_base_handler)
{
    /* empty */
}

casper::cef3::client::common::ContextMenuHandler::~ContextMenuHandler ()
{
  /* empty */
}

#ifdef __APPLE__
#pragma mark - CefContextMenuHandler
#endif

/**
 * @brief
 *
 * @param a_browser
 * @param a_frame
 * @param a_params
 * @param a_model
 */
void casper::cef3::client::common::ContextMenuHandler::OnBeforeContextMenu (CefRefPtr<CefBrowser> a_browser, CefRefPtr<CefFrame> /* a_frame */,
                                                                            CefRefPtr<CefContextMenuParams> a_params, CefRefPtr<CefMenuModel> a_model)
{
    CEF_REQUIRE_UI_THREAD();
    
    if ( ( a_params->GetTypeFlags() & (CM_TYPEFLAG_PAGE | CM_TYPEFLAG_FRAME) ) != 0 ) {
        
        while ( a_model->GetCount() ) {
            a_model->RemoveAt(0);
        }
              
    }    
    
    // TODO CW
    //    if ( nullptr != delegate_ ) {
    //        delegate_->OnBeforeContextMenu(a_model);
    //    }
}

/**
 * @brief
 *
 * @param a_browser
 * @param a_frame
 * @param a_params
 * @param a_command_id
 * @param a_event_flags
 */
bool casper::cef3::client::common::ContextMenuHandler::OnContextMenuCommand (CefRefPtr<CefBrowser> /* a_browser */, CefRefPtr<CefFrame> /* a_frame */,
                                                                             CefRefPtr<CefContextMenuParams> /* a_params */, int /* a_command_id */,
                                                                             EventFlags /* a_event_flags */)
{
    CEF_REQUIRE_UI_THREAD();
    
    return false;
}

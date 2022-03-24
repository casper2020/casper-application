// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef3/client/common/drag_handler.h"

#include "include/wrapper/cef_helpers.h" // CEF_REQUIRE_UI_THREAD

#include "cef3/browser/main_message_loop.h" // CURRENTLY_ON_MAIN_THREAD, MAIN_POST_CLOSURE

#include "cef3/client/common/client_handler.h" // ClientHandlerDelegate

casper::cef3::client::common::DragHandler::DragHandler (CefRefPtr<casper::cef3::client::common::BaseHandler> a_base_handler)
    : base_handler_(a_base_handler)
{
    /* empty */
}

casper::cef3::client::common::DragHandler::~DragHandler ()
{
    /* empty */
}

#ifdef __APPLE__
#pragma mark - CefDragHandler
#endif

bool casper::cef3::client::common::DragHandler::OnDragEnter (CefRefPtr<CefBrowser> browser,
                                                             CefRefPtr<CefDragData> dragData,
                                                             CefDragHandler::DragOperationsMask mask)
{
    CEF_REQUIRE_UI_THREAD();
    
    // Forbid dragging of URLs and files.
    if ( ( mask & DRAG_OPERATION_LINK) && !dragData->IsFragment() ) {
        return true;
    }
    
    return false;
}

void casper::cef3::client::common::DragHandler::OnDraggableRegionsChanged(CefRefPtr<CefBrowser> browser,
                                                                          CefRefPtr<CefFrame> frame,
                                                                          const std::vector<CefDraggableRegion>& regions)
{
    CEF_REQUIRE_UI_THREAD();
    
    NotifyDraggableRegions(regions);
}

#ifdef __APPLE__
#pragma mark -
#endif

void casper::cef3::client::common::DragHandler::NotifyDraggableRegions (const std::vector<CefDraggableRegion>& regions)
{
    if (!CURRENTLY_ON_MAIN_THREAD()) {
        // Execute this method on the main thread.
        MAIN_POST_CLOSURE(base::BindOnce(&casper::cef3::client::common::DragHandler::NotifyDraggableRegions, this, regions));
        return;
    }
    
    if ( nullptr != base_handler_->delegate_ptr_ ) {
        base_handler_->delegate_ptr_->OnSetDraggableRegions(regions);
    }
}

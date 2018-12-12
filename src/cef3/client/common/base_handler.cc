// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef3/client/common/base_handler.h"

casper::cef3::client::common::BaseHandler::BaseHandler (CefRefPtr<CefMessageRouterBrowserSide> a_message_router,
                                                        CefRefPtr<CefResourceManager> a_resource_manager,
                                                        casper::cef3::client::common::ClientHandlerDelegate* a_delegate_ptr)
{
    message_router_   = a_message_router;
    resource_manager_ = a_resource_manager;
    delegate_ptr_     = a_delegate_ptr;
}

casper::cef3::client::common::BaseHandler::~BaseHandler ()
{
    message_router_   = nullptr;
    resource_manager_ = nullptr;
    delegate_ptr_     = nullptr;
}

// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef3/common/app.h"

/**
 * @brief Default constructor.
 */
casper::cef3::common::App::App ()
{
    /* empty */
}

void casper::cef3::common::App::OnRegisterCustomSchemes (CefRawPtr<CefSchemeRegistrar> registrar)
{
    return;
}

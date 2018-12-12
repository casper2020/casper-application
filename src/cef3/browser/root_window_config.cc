// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef3/browser/root_window_config.h"

#include "cef3/browser/main_context.h"

casper::cef3::browser::RootWindowConfig::RootWindowConfig ()
    : with_controls(true), with_osr(false), with_extension(false), initially_hidden(false),
    url(casper::cef3::browser::MainContext::Get()->GetMainURL())
{
    /* empty */
}

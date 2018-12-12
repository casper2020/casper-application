// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef3/browser/main_context.h"

#include "include/base/cef_logging.h"

casper::cef3::browser::MainContext* g_main_context = NULL;

/**
 * @brief Default constructor.
 *
 * @param a_settings
 */
casper::cef3::browser::MainContext::MainContext (const casper::cef3::browser::Settings& a_settings)
    : settings_(a_settings)
{
    DCHECK(!g_main_context);
    g_main_context = this;
}

/**
 * @brief Destructor.
 */
casper::cef3::browser::MainContext::~MainContext ()
{
    g_main_context = NULL;
}

// static
casper::cef3::browser::MainContext* casper::cef3::browser::MainContext::Get ()
{
    DCHECK(g_main_context);
    return g_main_context;
}

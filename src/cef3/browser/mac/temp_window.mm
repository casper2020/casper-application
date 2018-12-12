// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#import <Cocoa/Cocoa.h>

#include "cef3/browser/mac/temp_window.h"

#include "include/base/cef_logging.h"
#include "include/cef_app.h"

casper::cef3::browser::TempWindowMAC* g_temp_window = NULL;

casper::cef3::browser::TempWindowMAC::TempWindowMAC ()
    : window_(nil)
{
    DCHECK(!g_temp_window);
    g_temp_window = this;
    
    // Create a borderless non-visible 1x1 window.
    window_ = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 1, 1)
                                          styleMask:NSBorderlessWindowMask
                                            backing:NSBackingStoreBuffered
                                              defer:NO];
    CHECK(window_);
}

casper::cef3::browser::TempWindowMAC::~TempWindowMAC ()
{
    g_temp_window = NULL;
    DCHECK(window_);
    [window_ close];
}

// static
CefWindowHandle casper::cef3::browser::TempWindowMAC::GetWindowHandle()
{
    DCHECK(g_temp_window);
    return [g_temp_window->window_ contentView];
}

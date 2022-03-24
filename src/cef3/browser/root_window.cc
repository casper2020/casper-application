// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef3/browser/root_window.h"

#include "cef3/browser/main_context.h"

#include "cef3/browser/root_window_manager.h"

#include "include/base/cef_callback_helpers.h"

/**
 * @brief Default constructor.
 */
casper::cef3::browser::RootWindow::RootWindow ()
    : delegate_(NULL)
{
    /* empty */
}

/**
 * @brief Destructor.
 */
casper::cef3::browser::RootWindow::~RootWindow ()
{
    /* empty */
}

// static
scoped_refptr<casper::cef3::browser::RootWindow> casper::cef3::browser::RootWindow::GetForBrowser (int browser_id)
{
    return casper::cef3::browser::MainContext::Get()->GetRootWindowManager()->GetWindowForBrowser(browser_id);
}

void casper::cef3::browser::RootWindow::OnExtensionsChanged (const casper::cef3::browser::ExtensionSet& extensions)
{
    REQUIRE_MAIN_THREAD();
    DCHECK(delegate_);
    DCHECK(!WithExtension());
    
    if (extensions.empty())
        return;
    
    ExtensionSet::const_iterator it = extensions.begin();
    for (; it != extensions.end(); ++it) {
        delegate_->CreateExtensionWindow(*it, CefRect(), nullptr, base::DoNothing(), WithWindowlessRendering());
    }
}

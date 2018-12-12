// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CASPER_CEF3_CLIENT_BROWSER_ROOT_WINDOW_CONFIG_H_
#define CASPER_CEF3_CLIENT_BROWSER_ROOT_WINDOW_CONFIG_H_
#pragma once

#include <string>

#include "include/base/cef_ref_counted.h" // CefRefPtr
#include "include/base/cef_callback.h"    // Closure

#include "include/views/cef_window.h"     // CefWindow, CefRect

namespace casper
{
    
    namespace cef3
    {
        
        namespace browser
        {
            
            // Used to configure how a RootWindow is created.
            struct RootWindowConfig
            {
                
                RootWindowConfig();
                
                // If true the window will always display above other windows.
                bool always_on_top;
                
                // If true the window will show controls.
                bool with_controls;
                
                // If true the window will use off-screen rendering.
                bool with_osr;
                
                // If true the window is hosting an extension app.
                bool with_extension;
                
                // If true the window will be created initially hidden.
                bool initially_hidden;
                
                // Requested window position. If |bounds| and |source_bounds| are empty the
                // default window size and location will be used.
                CefRect bounds;
                
                // Position of the UI element that triggered the window creation. If |bounds|
                // is empty and |source_bounds| is non-empty the new window will be positioned
                // relative to |source_bounds|. This is currently only implemented for Views-
                // based windows when |initially_hidden| is also true.
                CefRect source_bounds;
                
                // Parent window. Only used for Views-based windows.
                CefRefPtr<CefWindow> parent_window;
                
                // Callback to be executed when the window is closed. Will be executed on the
                // main thread. This is currently only implemented for Views-based windows.
                base::Closure close_callback;
                
                // Initial URL to load.
                std::string url;
            };
            
        } // end of namespace 'browser'
        
    } // end of namespace 'cef3'
    
} // end of namespace 'casper'

#endif // CASPER_CEF3_CLIENT_BROWSER_ROOT_WINDOW_CONFIG_H_


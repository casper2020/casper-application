// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CASPER_CEF3_BROWSER_MAC_TEMP_WINDOW_H_
#define CASPER_CEF3_BROWSER_MAC_TEMP_WINDOW_H_

#pragma once

#include "cef3/browser/browser_window.h"

namespace casper
{
    
    namespace cef3
    {
        
        namespace browser
        {
            class RootWindowManager;

            // Represents a singleton hidden window that acts as a temporary parent for popup browsers.
            // Only accessed on the UI thread.
            class TempWindowMAC
            {
                
            public:
                
                // Returns the singleton window handle.
                static CefWindowHandle GetWindowHandle();
                
            private:
                
                // A single instance will be created/owned by RootWindowManager.
                friend class RootWindowManager;
                // Allow deletion via scoped_ptr only.
                friend struct base::DefaultDeleter<TempWindowMAC>;
                
                TempWindowMAC();
                ~TempWindowMAC();
                
                NSWindow* window_;
                
                DISALLOW_COPY_AND_ASSIGN(TempWindowMAC);
                
            }; // end of class 'TempWindowMAC'
            
        } // end of namespace 'browser'
        
    } // end of namespace 'cef3'
    
} // end of namespace 'casper'

#endif // CASPER_CEF3_BROWSER_MAC_TEMP_WINDOW_H_



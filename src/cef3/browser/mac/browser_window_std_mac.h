// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CASPER_CEF3_BROWSER_BROWSER_WINDOW_STD_MAC_H_
#define CASPER_CEF3_BROWSER_BROWSER_WINDOW_STD_MAC_H_

#pragma once

#include "cef3/browser/browser_window.h"

namespace casper
{
    
    namespace cef3
    {
        
        namespace browser
        {
            
            // Represents a native child window hosting a single windowed browser instance.
            // The methods of this class must be called on the main thread unless otherwise
            // indicated.
            class BrowserWindowStdMAC : public casper::cef3::browser::BrowserWindow
            {
                
            protected: // Const Data
                
                const ::std::string startup_url_;
                
            public:
                
                // Constructor may be called on any thread.
                // |delegate| must outlive this object.
                BrowserWindowStdMAC (Delegate* a_delegate, const ::std::string& a_startup_url);
                
            public:
                // BrowserWindow methods.
                void CreateBrowser(ClientWindowHandle parent_handle,
                                   const CefRect& rect,
                                   const CefBrowserSettings& settings,
                                   CefRefPtr<CefRequestContext> request_context) OVERRIDE;
                void GetPopupConfig(CefWindowHandle temp_handle,
                                    CefWindowInfo& windowInfo,
                                    CefRefPtr<CefClient>& client,
                                    CefBrowserSettings& settings) OVERRIDE;
                void ShowPopup(ClientWindowHandle parent_handle,
                               int x,
                               int y,
                               size_t width,
                               size_t height) OVERRIDE;
                void Show() OVERRIDE;
                void Hide() OVERRIDE;
                void SetBounds(int x, int y, size_t width, size_t height) OVERRIDE;
                void SetFocus(bool focus) OVERRIDE;
                ClientWindowHandle GetWindowHandle() const OVERRIDE;
                
            private:
                DISALLOW_COPY_AND_ASSIGN(BrowserWindowStdMAC);
            };
            
        } // end of namespace 'browser'
        
    } // end of namespace 'cef3'
    
} // end of namespace 'casper'

#endif // CASPER_CEF3_BROWSER_BROWSER_WINDOW_STD_MAC_H_

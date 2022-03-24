// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CASPER_CEF3_BROWSER_MAC_ROOT_WINDOW_H_
#define CASPER_CEF3_BROWSER_MAC_ROOT_WINDOW_H_

#pragma once

#include "cef3/browser/root_window.h"

#include "cef3/browser/browser_window.h"

#include "cef3/browser/mac/browser_window_std_mac.h"

namespace casper
{
    
    namespace cef3
    {
     
        namespace browser
        {
            
            class RootWindowMAC : public casper::cef3::browser::RootWindow, public BrowserWindowStdMAC::Delegate
            {
                
            public: // Constructor(s) / Destructor
                
                RootWindowMAC ();
                
            protected:
                
                // Allow deletion via scoped_refptr only.
                friend struct DeleteOnMainThread;
                friend class base::RefCountedThreadSafe<RootWindowMAC, cef3::browser::DeleteOnMainThread>;
                
                ~RootWindowMAC();
                
            public: // Inherited Method(s) / Function(s) - casper::cef3::browser::RootWindow
                
                void Init (casper::cef3::browser::RootWindow::Delegate* delegate,
                           const casper::cef3::browser::RootWindowConfig& config, const CefBrowserSettings& settings) override;
                
                void InitAsPopup(casper::cef3::browser::RootWindow::Delegate* delegate,
                                 bool with_controls, bool with_osr,
                                 const CefPopupFeatures& popupFeatures,
                                 CefWindowInfo& windowInfo, CefRefPtr<CefClient>& client, CefBrowserSettings& settings) override;
                
                void                  Show                     (ShowMode mode) override;
                void                  Hide                     () override;
                void                  SetBounds                (int x, int y, size_t width, size_t height) override;
                void                  Close                    (bool force) override;
                void                  SetDeviceScaleFactor     (float device_scale_factor) override;
                float                 GetDeviceScaleFactor     () const override;
                CefRefPtr<CefBrowser> GetBrowser               () const override;
                ClientWindowHandle    GetWindowHandle          () const override;
                bool                  WithWindowlessRendering  () const override;
                bool                  WithExtension            () const override;
                
                // Called by RootWindowDelegate after the associated NSWindow has been
                // destroyed.
                void WindowDestroyed();
                
                casper::cef3::browser::BrowserWindow*        browser_window () const { return browser_window_.get(); }
                casper::cef3::browser::RootWindow::Delegate* delegate       () const { return delegate_; }
                
            private:
                
                void CreateBrowserWindow(const ::std::string& startup_url);
                void CreateRootWindow(const CefBrowserSettings& settings,
                                      bool initially_hidden);
                
            private: // Inherited Method(s) / Function(s) - ::client::BrowserWindow::Delegate
                
                void OnBrowserCreated         (CefRefPtr<CefBrowser> browser) override;
                void OnBrowserWindowDestroyed () override;
                
                void OnSetAddress             (const ::std::string& url) override;
                void OnSetTitle               (const ::std::string& title) override;
                void OnSetFullscreen          (bool fullscreen) override;
                void OnAutoResize             (const CefSize& new_size) override;
                void OnSetLoadingState        (bool isLoading, bool canGoBack, bool canGoForward) override;
                void OnSetDraggableRegions    (const ::std::vector<CefDraggableRegion>& regions) override;
                
                void NotifyDestroyedIfDone();
                
                // After initialization all members are only accessed on the main thread.
                // Members set during initialization.
                bool with_osr_;
                bool with_extension_;
                bool is_popup_;
                CefRect start_rect_;
                std::unique_ptr<casper::cef3::browser::BrowserWindow> browser_window_;
                bool initialized_;
                
                // Main window.
                NSWindow* window_;
                
                bool window_destroyed_;
                bool browser_destroyed_;
                
                DISALLOW_COPY_AND_ASSIGN(RootWindowMAC);
                
            };
        } // end of namespace 'browser'
        
    } // end of namespace 'cef3'
    
}  // end of namespace 'casper'

#endif // CASPER_CEF3_BROWSER_MAC_ROOT_WINDOW_H_

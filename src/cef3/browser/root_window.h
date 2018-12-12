// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CASPER_CEF3_BROWSER_ROOT_WINDOW_H_
#define CASPER_CEF3_BROWSER_ROOT_WINDOW_H_
#pragma once

#include <set>
#include <string>

#include "include/base/cef_callback_forward.h"
#include "include/base/cef_ref_counted.h"

#include "include/cef_browser.h"
#include "include/views/cef_window.h"

#include "cef3/browser/window_types.h"

#include "cef3/client/browser/image_cache.h"

#include "cef3/browser/main_message_loop.h"
#include "cef3/browser/root_window_config.h"

namespace casper
{
    
    namespace cef3
    {
        
        namespace browser
        {
            
            typedef std::set<CefRefPtr<CefExtension>> ExtensionSet;
            
            // Represents a top-level native window in the browser process. While references
            // to this object are thread-safe the methods must be called on the main thread
            // unless otherwise indicated.
            class RootWindow : public base::RefCountedThreadSafe<RootWindow, DeleteOnMainThread>
            {
            public:
                // This interface is implemented by the owner of the RootWindow. The methods
                // of this class will be called on the main thread.
                class Delegate {
                public:
                    // Called to retrieve the CefRequestContext for browser. Only called for
                    // non-popup browsers. May return NULL.
                    virtual CefRefPtr<CefRequestContext> GetRequestContext(RootWindow* root_window) = 0;
                    
                    // Returns the ImageCache.
                    virtual scoped_refptr<client::browser::ImageCache> GetImageCache() = 0;
                                        
                    // Called to exit the application.
                    virtual void OnExit(RootWindow* root_window) = 0;
                    
                    // Called when the RootWindow has been destroyed.
                    virtual void OnRootWindowDestroyed(RootWindow* root_window) = 0;
                    
                    // Called when the RootWindow is activated (becomes the foreground window).
                    virtual void OnRootWindowActivated(RootWindow* root_window) = 0;
                    
                    // Called when the browser is created for the RootWindow.
                    virtual void OnBrowserCreated(RootWindow* root_window, CefRefPtr<CefBrowser> browser) = 0;
                    
                    // Create a window for |extension|. |source_bounds| are the bounds of the
                    // UI element, like a button, that triggered the extension.
                    virtual void CreateExtensionWindow(CefRefPtr<CefExtension> extension,
                                                       const CefRect& source_bounds,
                                                       CefRefPtr<CefWindow> parent_window,
                                                       const base::Closure& close_callback,
                                                       bool with_osr) = 0;
                    
                protected:
                    virtual ~Delegate() {}
                };
                
            public: // CW: CEF Factory Static Helpers - declaration
                
                // Create a new RootWindow object. This method may be called on any thread.
                // Use RootWindowManager::CreateRootWindow() or CreateRootWindowAsPopup()
                // instead of calling this method directly. |use_views| will be true if the
                // Views framework should be used.
                static scoped_refptr<RootWindow> Factory(bool use_views);
                
                // Returns the RootWindow associated with the specified |browser_id|. Must be
                // called on the main thread.
                static scoped_refptr<RootWindow> GetForBrowser(int browser_id);

            public:
                
#if defined(OS_MACOSX)
                // Returns the RootWindow associated with the specified |window|. Must be
                // called on the main thread.
                static scoped_refptr<RootWindow> GetForNSWindow(NSWindow* window);
#endif
                
                // Initialize as a normal window. This will create and show a native window
                // hosting a single browser instance. This method may be called on any thread.
                // |delegate| must be non-NULL and outlive this object.
                // Use RootWindowManager::CreateRootWindow() instead of calling this method
                // directly.
                virtual void Init(RootWindow::Delegate* delegate,
                                  const RootWindowConfig& config,
                                  const CefBrowserSettings& settings) = 0;
                
                // Initialize as a popup window. This is used to attach a new native window to
                // a single browser instance that will be created later. The native window
                // will be created and shown once the browser is available. This method may be
                // called on any thread. |delegate| must be non-NULL and outlive this object.
                // Use RootWindowManager::CreateRootWindowAsPopup() instead of calling this
                // method directly. Called on the UI thread.
                virtual void InitAsPopup(RootWindow::Delegate* delegate,
                                         bool with_controls,
                                         bool with_osr,
                                         const CefPopupFeatures& popupFeatures,
                                         CefWindowInfo& windowInfo,
                                         CefRefPtr<CefClient>& client,
                                         CefBrowserSettings& settings) = 0;
                
                enum ShowMode {
                    ShowNormal,
                    ShowMinimized,
                    ShowMaximized,
                };
                
                // Show the window.
                virtual void Show(ShowMode mode) = 0;
                
                // Hide the window.
                virtual void Hide() = 0;
                
                // Set the window bounds in screen coordinates.
                virtual void SetBounds(int x, int y, size_t width, size_t height) = 0;
                
                // Close the window. If |force| is true onunload handlers will not be
                // executed.
                virtual void Close(bool force) = 0;
                
                // Set the device scale factor. Only used in combination with off-screen
                // rendering.
                virtual void SetDeviceScaleFactor(float device_scale_factor) = 0;
                
                // Returns the device scale factor. Only used in combination with off-screen
                // rendering.
                virtual float GetDeviceScaleFactor() const = 0;
                
                // Returns the browser that this window contains, if any.
                virtual CefRefPtr<CefBrowser> GetBrowser() const = 0;
                
                // Returns the native handle for this window, if any.
                virtual ClientWindowHandle GetWindowHandle() const = 0;
                
                // Returns true if this window is using windowless rendering (osr).
                virtual bool WithWindowlessRendering() const = 0;
                
                // Returns true if this window is hosting an extension app.
                virtual bool WithExtension() const = 0;
                
                // Called when the set of loaded extensions changes. The default
                // implementation will create a single window instance for each extension.
                virtual void OnExtensionsChanged(const ExtensionSet& extensions);
                
            protected:
                // Allow deletion via scoped_refptr only.
                friend struct DeleteOnMainThread;
                friend class base::RefCountedThreadSafe<RootWindow, DeleteOnMainThread>;
                
                RootWindow();
                virtual ~RootWindow();
                
                Delegate*    delegate_;
                cef_string_t name_;                
                
            }; // end of class 'RootWindow'
            
        } // end of namespace 'browser'
        
    } // end of namespace 'cef3'
    
} // end of namespace 'casper'

#endif  // CASPER_CEF3_BROWSER_ROOT_WINDOW_H_

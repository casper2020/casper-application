// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CASPER_CEF3_BROWSER_ROOT_WINDOW_MANAGER_H_
#define CASPER_CEF3_BROWSER_ROOT_WINDOW_MANAGER_H_

#pragma once

#include <set>

#include "include/base/cef_scoped_refptr.h"
#include "include/cef_command_line.h"

#include "cef3/client/browser/image_cache.h"

#include "cef3/browser/temp_window.h"

#include "cef3/browser/root_window.h"
#include "cef3/browser/root_window_config.h"

namespace casper
{
    
    namespace cef3
    {
        
        namespace browser
        {

            class RootWindowManager : public casper::cef3::browser::RootWindow::Delegate
            {
                
            private: // Const Data
                
                const bool terminate_when_all_windows_closed_;
                
            private: // Data
                
                bool request_context_per_browser_;
                bool request_context_shared_cache_;
                
                // Existing root windows. Only accessed on the main thread.
                typedef std::set<scoped_refptr<casper::cef3::browser::RootWindow>> RootWindowSet;
                RootWindowSet root_windows_;
                
                // The currently active/foreground RootWindow. Only accessed on the main
                // thread.
                scoped_refptr<casper::cef3::browser::RootWindow> active_root_window_;
                
                // The currently active/foreground browser. Access is protected by
                // |active_browser_lock_;
                mutable base::Lock active_browser_lock_;
                CefRefPtr<CefBrowser> active_browser_;
                
                // Singleton window used as the temporary parent for popup browsers.
                std::unique_ptr<casper::cef3::browser::TempWindow> temp_window_;
                
                CefRefPtr<CefRequestContext> shared_request_context_;
                
                // Loaded extensions. Only accessed on the main thread.
                casper::cef3::browser::ExtensionSet extensions_;

                scoped_refptr<client::browser::ImageCache> image_cache_;
                
            public: // Constructor
                
                explicit RootWindowManager (bool terminate_when_all_windows_closed);
                
            private: // Destructor
                
                // Allow deletion via scoped_ptr only.
                friend struct std::default_delete<RootWindowManager>;
                
                ~RootWindowManager();
                
            public: // Method(s) / Function(s)
                
                // Create a new top-level native window. This method can be called from
                // anywhere.
                scoped_refptr<casper::cef3::browser::RootWindow> CreateRootWindow(const casper::cef3::browser::RootWindowConfig& config);
                
                // Create a new native popup window.
                // If |with_controls| is true the window will show controls.
                // If |with_osr| is true the window will use off-screen rendering.
                // This method is called from ClientHandler::CreatePopupWindow() to
                // create a new popup or DevTools window. Must be called on the UI thread.
                scoped_refptr<casper::cef3::browser::RootWindow> CreateRootWindowAsPopup(
                                                                                                 bool with_controls,
                                                                                                 bool with_osr,
                                                                                                 const CefPopupFeatures& popupFeatures,
                                                                                                 CefWindowInfo& windowInfo,
                                                                                                 CefRefPtr<CefClient>& client,
                                                                                                 CefBrowserSettings& settings);
                
                // Create a new top-level native window to host |extension|.
                // If |with_controls| is true the window will show controls.
                // If |with_osr| is true the window will use off-screen rendering.
                // This method can be called from anywhere.
                scoped_refptr<casper::cef3::browser::RootWindow> CreateRootWindowAsExtension(CefRefPtr<CefExtension> extension,
                                                                                             const CefRect& source_bounds,
                                                                                             CefRefPtr<CefWindow> parent_window,
                                                                                             base::OnceClosure close_callback,
                                                                                             bool with_controls,
                                                                                             bool with_osr);
                
                // Returns true if a window hosting |extension| currently exists. Must be
                // called on the main thread.
                bool HasRootWindowAsExtension(CefRefPtr<CefExtension> extension);
                
                // Returns the RootWindow associated with the specified browser ID. Must be
                // called on the main thread.
                scoped_refptr<casper::cef3::browser::RootWindow> GetWindowForBrowser(int browser_id) const;
                
                // Returns the currently active/foreground RootWindow. May return NULL. Must
                // be called on the main thread.
                scoped_refptr<casper::cef3::browser::RootWindow> GetActiveRootWindow() const;
                
                // Returns the currently active/foreground browser. May return NULL. Safe to
                // call from any thread.
                CefRefPtr<CefBrowser> GetActiveBrowser() const;
                
                // Close all existing windows. If |force| is true onunload handlers will not
                // be executed.
                void CloseAllWindows(bool force);
                
                // Manage the set of loaded extensions. RootWindows will be notified via the
                // OnExtensionsChanged method.
                void AddExtension(CefRefPtr<CefExtension> extension);
                
                bool request_context_per_browser() const {
                    return request_context_per_browser_;
                }
                
            private: // Overriden Method(s) / Function(s) - from casper::cef3::browser::RootWindow::Delegate methods.
                
                CefRefPtr<CefRequestContext> GetRequestContext (casper::cef3::browser::RootWindow* root_window) override;
                
                scoped_refptr<client::browser::ImageCache> GetImageCache() override;
                void OnExit(casper::cef3::browser::RootWindow* root_window) override;
                void OnRootWindowDestroyed(casper::cef3::browser::RootWindow* root_window) override;
                void OnRootWindowActivated(casper::cef3::browser::RootWindow* root_window) override;
                void OnBrowserCreated(casper::cef3::browser::RootWindow* root_window,
                                      CefRefPtr<CefBrowser> browser) override;
                void CreateExtensionWindow(CefRefPtr<CefExtension> extension,
                                           const CefRect& source_bounds,
                                           CefRefPtr<CefWindow> parent_window,
                                           base::OnceClosure close_callback,
                                           bool with_osr) override;
                
            private:
                
                void OnRootWindowCreated (scoped_refptr<casper::cef3::browser::RootWindow> root_window);
                void NotifyExtensionsChanged();
                void CleanupOnUIThread();
                
            private:
                
                DISALLOW_COPY_AND_ASSIGN(RootWindowManager);
                
            }; // end of class 'RootWindowManager'
            
        } // end of namespace 'browser'
        
    } // end of namespace 'cef3'
    
}  // end of namespace 'casper'

#endif // CASPER_CEF3_BROWSER_ROOT_WINDOW_MANAGER_H_

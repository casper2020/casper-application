/**
 * @file root_window_manager.h
 *
 * Copyright (c) 2010-2017 Neto Ranito & Seabra LDA. All rights reserved.
 *
 * This file is part of casper.
 *
 * casper is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * casper  is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with casper.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cef3/browser/root_window_manager.h"

#include "cef3/browser/main_context.h"

#include "cef3/browser/request_context_handler.h"

#include "cef3/shared/browser/utils/extension_util.h"

#include "cef3/common/client/switches.h"

casper::cef3::browser::RootWindowManager::RootWindowManager (bool terminate_when_all_windows_closed)
    : terminate_when_all_windows_closed_(terminate_when_all_windows_closed),
    image_cache_(new casper::cef3::client::browser::ImageCache())
{
    CefRefPtr<CefCommandLine> command_line = CefCommandLine::GetGlobalCommandLine();
    DCHECK(command_line.get());
    
    request_context_per_browser_  = command_line->HasSwitch(casper::cef3::common::client::switches::kRequestContextPerBrowser);
    request_context_shared_cache_ = command_line->HasSwitch(casper::cef3::common::client::switches::kRequestContextSharedCache);
}

casper::cef3::browser::RootWindowManager::~RootWindowManager ()
{
    // All root windows should already have been destroyed.
    DCHECK(root_windows_.empty());
}

scoped_refptr<casper::cef3::browser::RootWindow> casper::cef3::browser::RootWindowManager::CreateRootWindow (const casper::cef3::browser::RootWindowConfig& config)
{
    CefBrowserSettings settings;
    
    auto context = casper::cef3::browser::MainContext::Get();
    
    context->PopulateBrowserSettings(&settings);
    
    scoped_refptr<casper::cef3::browser::RootWindow> root_window = casper::cef3::browser::RootWindow::Factory(context->UseViews());
    root_window->Init(this, config, settings);
    
    // Store a reference to the root window on the main thread.
    OnRootWindowCreated(root_window);
    
    return root_window;
}

scoped_refptr<casper::cef3::browser::RootWindow> casper::cef3::browser::RootWindowManager::CreateRootWindowAsPopup (bool with_controls, bool with_osr, const CefPopupFeatures& popupFeatures,
                                                                     CefWindowInfo& windowInfo,
                                                                     CefRefPtr<CefClient>& client,
                                                                     CefBrowserSettings& settings) {
    CEF_REQUIRE_UI_THREAD();
    
    if (!temp_window_) {
        // TempWindow must be created on the UI thread.
        temp_window_.reset(new casper::cef3::browser::TempWindow());
    }
    
    auto context = casper::cef3::browser::MainContext::Get();
    
    context->PopulateBrowserSettings(&settings);
    
    scoped_refptr<casper::cef3::browser::RootWindow> root_window = casper::cef3::browser::RootWindow::Factory(context->UseViews());

    root_window->InitAsPopup(this, with_controls, with_osr, popupFeatures, windowInfo, client, settings);
    
    // Store a reference to the root window on the main thread.
    OnRootWindowCreated(root_window);
    
    return root_window;
}

scoped_refptr<casper::cef3::browser::RootWindow> casper::cef3::browser::RootWindowManager::CreateRootWindowAsExtension (CefRefPtr<CefExtension> extension,
                                                                                                        const CefRect& source_bounds,
                                                                                                        CefRefPtr<CefWindow> parent_window,
                                                                                                        const base::Closure& close_callback,
                                                                                                        bool with_controls,
                                                                                                        bool with_osr) {
    const std::string& extension_url = casper::cef3::shared::browser::utils::extension::GetExtensionURL(extension);
    if (extension_url.empty()) {
        NOTREACHED() << "Extension cannot be loaded directly.";
        return NULL;
    }

    // Create an initially hidden browser window that loads the extension URL.
    // We'll show the window when the desired size becomes available via
    // ClientHandler::OnAutoResize.
    casper::cef3::browser::RootWindowConfig config;
    config.with_controls = with_controls;
    config.with_osr = with_osr;
    config.with_extension = true;
    config.initially_hidden = true;
    config.source_bounds = source_bounds;
    config.parent_window = parent_window;
    config.close_callback = close_callback;
    config.url = extension_url;
    return CreateRootWindow(config);
    
}

bool casper::cef3::browser::RootWindowManager::HasRootWindowAsExtension (CefRefPtr<CefExtension> extension)
{
    REQUIRE_MAIN_THREAD();
    
    RootWindowSet::const_iterator it = root_windows_.begin();
    for (; it != root_windows_.end(); ++it) {
        const casper::cef3::browser::RootWindow* root_window = (*it);
        if (!root_window->WithExtension())
            continue;
        
        CefRefPtr<CefBrowser> browser = root_window->GetBrowser();
        if (!browser)
            continue;
        
        CefRefPtr<CefExtension> browser_extension =
        browser->GetHost()->GetExtension();
        DCHECK(browser_extension);
        if (browser_extension->GetIdentifier() == extension->GetIdentifier())
            return true;
    }
    
    return false;
}

scoped_refptr<casper::cef3::browser::RootWindow> casper::cef3::browser::RootWindowManager::GetWindowForBrowser (int browser_id) const
{
    REQUIRE_MAIN_THREAD();
    
    RootWindowSet::const_iterator it = root_windows_.begin();
    for (; it != root_windows_.end(); ++it) {
        CefRefPtr<CefBrowser> browser = (*it)->GetBrowser();
        if (browser.get() && browser->GetIdentifier() == browser_id)
            return *it;
    }
    return NULL;
}

scoped_refptr<casper::cef3::browser::RootWindow> casper::cef3::browser::RootWindowManager::GetActiveRootWindow() const
{
    REQUIRE_MAIN_THREAD();
    return active_root_window_;
}

CefRefPtr<CefBrowser> casper::cef3::browser::RootWindowManager::GetActiveBrowser() const
{
    base::AutoLock lock_scope(active_browser_lock_);
    return active_browser_;
}

void casper::cef3::browser::RootWindowManager::CloseAllWindows (bool force)
{
    if ( !CURRENTLY_ON_MAIN_THREAD() ) {
        // Execute this method on the main thread.
        MAIN_POST_CLOSURE(base::Bind(&casper::cef3::browser::RootWindowManager::CloseAllWindows, base::Unretained(this), force));
        return;
    }
    
    if (root_windows_.empty())
        return;
    
    RootWindowSet::const_iterator it = root_windows_.begin();
    for (; it != root_windows_.end(); ++it)
        (*it)->Close(force);
}

void casper::cef3::browser::RootWindowManager::AddExtension (CefRefPtr<CefExtension> extension)
{
    if ( !CURRENTLY_ON_MAIN_THREAD() ) {
        // Execute this method on the main thread.
        MAIN_POST_CLOSURE(base::Bind(&casper::cef3::browser::RootWindowManager::AddExtension, base::Unretained(this), extension));
        return;
    }

    // Don't track extensions that can't be loaded directly.
    if ( casper::cef3::shared::browser::utils::extension::GetExtensionURL(extension).empty() )
        return;

    // Don't add the same extension multiple times.
    casper::cef3::browser::ExtensionSet::const_iterator it = extensions_.begin();
    for (; it != extensions_.end(); ++it) {
        if ((*it)->GetIdentifier() == extension->GetIdentifier())
            return;
    }

    extensions_.insert(extension);
    NotifyExtensionsChanged();
}

void casper::cef3::browser::RootWindowManager::OnRootWindowCreated (scoped_refptr<casper::cef3::browser::RootWindow> root_window)
{
    if (!CURRENTLY_ON_MAIN_THREAD()) {
        // Execute this method on the main thread.
        MAIN_POST_CLOSURE(base::Bind(&RootWindowManager::OnRootWindowCreated,
                                     base::Unretained(this), root_window));
        return;
    }
    
    root_windows_.insert(root_window);
    if (!root_window->WithExtension()) {
        root_window->OnExtensionsChanged(extensions_);
        
        if (root_windows_.size() == 1U) {
            // The first non-extension root window should be considered the active
            // window.
            OnRootWindowActivated(root_window);
        }
    }
}

void casper::cef3::browser::RootWindowManager::NotifyExtensionsChanged()
{
    REQUIRE_MAIN_THREAD();
    
    RootWindowSet::const_iterator it = root_windows_.begin();
    for (; it != root_windows_.end(); ++it) {
        casper::cef3::browser::RootWindow* root_window = *it;
        if (!root_window->WithExtension())
            root_window->OnExtensionsChanged(extensions_);
    }
}

CefRefPtr<CefRequestContext> casper::cef3::browser::RootWindowManager::GetRequestContext (casper::cef3::browser::RootWindow* root_window)
{
    REQUIRE_MAIN_THREAD();
    
    if (request_context_per_browser_) {
        // Create a new request context for each browser.
        CefRequestContextSettings settings;
        
        CefRefPtr<CefCommandLine> command_line = CefCommandLine::GetGlobalCommandLine();
        if ( command_line->HasSwitch(casper::cef3::common::client::switches::kCachePath ) ) {
            if (request_context_shared_cache_) {
                // Give each browser the same cache path. The resulting context objects
                // will share the same storage internally.
                CefString(&settings.cache_path) =
                command_line->GetSwitchValue(casper::cef3::common::client::switches::kCachePath);
            } else {
                // Give each browser a unique cache path. This will create completely
                // isolated context objects.
                std::stringstream ss;
                ss << command_line->GetSwitchValue(casper::cef3::common::client::switches::kCachePath).ToString()
                << time(NULL);
                CefString(&settings.cache_path) = ss.str();
            }
        }
                
        return CefRequestContext::CreateContext(settings, new casper::cef3::browser::RequestContextHandler());
    }
    
    // All browsers will share the global request context.
    if (!shared_request_context_.get()) {
        shared_request_context_ = CefRequestContext::CreateContext(CefRequestContext::GetGlobalContext(), new casper::cef3::browser::RequestContextHandler());
    }
    return shared_request_context_;
}

scoped_refptr<casper::cef3::client::browser::ImageCache> casper::cef3::browser::RootWindowManager::GetImageCache()
{
    REQUIRE_MAIN_THREAD();
    
    return image_cache_;
}

void casper::cef3::browser::RootWindowManager::OnExit (casper::cef3::browser::RootWindow* root_window)
{
    REQUIRE_MAIN_THREAD();
    
    CloseAllWindows(false);
}

void casper::cef3::browser::RootWindowManager::OnRootWindowDestroyed (casper::cef3::browser::RootWindow* root_window)
{
    REQUIRE_MAIN_THREAD();
    
    RootWindowSet::iterator it = root_windows_.find(root_window);
    DCHECK(it != root_windows_.end());
    if (it != root_windows_.end())
        root_windows_.erase(it);
    
    if (root_window == active_root_window_) {
        active_root_window_ = NULL;
        
        base::AutoLock lock_scope(active_browser_lock_);
        active_browser_ = NULL;
    }
    
    if (terminate_when_all_windows_closed_ && root_windows_.empty()) {
        // All windows have closed. Clean up on the UI thread.
        CefPostTask(TID_UI, base::Bind(&casper::cef3::browser::RootWindowManager::CleanupOnUIThread, base::Unretained(this)));
    }
}

void casper::cef3::browser::RootWindowManager::OnRootWindowActivated (casper::cef3::browser::RootWindow* root_window)
{
    REQUIRE_MAIN_THREAD();
    
    if (root_window->WithExtension()) {
        // We don't want extension apps to become the active RootWindow.
        return;
    }
    
    if (root_window == active_root_window_)
        return;
    
    active_root_window_ = root_window;
    
    {
        base::AutoLock lock_scope(active_browser_lock_);
        // May be NULL at this point, in which case we'll make the association in
        // OnBrowserCreated.
        active_browser_ = active_root_window_->GetBrowser();
    }
}

void casper::cef3::browser::RootWindowManager::OnBrowserCreated (casper::cef3::browser::RootWindow* root_window,
                                         CefRefPtr<CefBrowser> browser) {
    REQUIRE_MAIN_THREAD();
    
    if (root_window == active_root_window_) {
        base::AutoLock lock_scope(active_browser_lock_);
        active_browser_ = browser;
    }
}

void casper::cef3::browser::RootWindowManager::CreateExtensionWindow(
                                              CefRefPtr<CefExtension> extension,
                                              const CefRect& source_bounds,
                                              CefRefPtr<CefWindow> parent_window,
                                              const base::Closure& close_callback,
                                              bool with_osr) {
    REQUIRE_MAIN_THREAD();
    
    if (!HasRootWindowAsExtension(extension)) {
        CreateRootWindowAsExtension(extension, source_bounds, parent_window,
                                    close_callback, false, with_osr);
    }
}

void casper::cef3::browser::RootWindowManager::CleanupOnUIThread() {
    CEF_REQUIRE_UI_THREAD();
    
    if (temp_window_) {
        // TempWindow must be destroyed on the UI thread.
        temp_window_.reset(nullptr);
    }
    
    // Quit the main message loop.
    browser::MainMessageLoop::Get()->Quit();
}


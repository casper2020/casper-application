// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef3/client/browser/client_app_browser.h"

#include "include/base/cef_logging.h"
#include "include/cef_cookie.h"

#include "cef3/browser/main_message_loop_external_pump.h"

#include "cef3/common/client/switches.h"

casper::cef3::client::browser::ClientAppBrowser::ClientAppBrowser()
{
    CreateDelegates(delegates_);
}

void casper::cef3::client::browser::ClientAppBrowser::OnBeforeCommandLineProcessing (const CefString& process_type,
                                                                                     CefRefPtr<CefCommandLine> command_line)
{
    // Pass additional command-line flags to the browser process.
    if (process_type.empty()) {
        // Pass additional command-line flags when off-screen rendering is enabled.
        if (command_line->HasSwitch(casper::cef3::common::client::switches::kOffScreenRenderingEnabled)) {
            // If the PDF extension is enabled then cc Surfaces must be disabled for
            // PDFs to render correctly.
            // See https://bitbucket.org/chromiumembedded/cef/issues/1689 for details.
            if (!command_line->HasSwitch("disable-extensions") &&
                !command_line->HasSwitch("disable-pdf-extension")) {
                command_line->AppendSwitch("disable-surfaces");
            }
            
            // Use software rendering and compositing (disable GPU) for increased FPS
            // and decreased CPU usage. This will also disable WebGL so remove these
            // switches if you need that capability.
            // See https://bitbucket.org/chromiumembedded/cef/issues/1257 for details.
            if (!command_line->HasSwitch(casper::cef3::common::client::switches::kEnableGPU)) {
                command_line->AppendSwitch("disable-gpu");
                command_line->AppendSwitch("disable-gpu-compositing");
            }
        }
        
        if (command_line->HasSwitch(casper::cef3::common::client::switches::kUseViews) &&
            !command_line->HasSwitch("top-chrome-md")) {
            // Use non-material mode on all platforms by default. Among other things
            // this causes menu buttons to show hover state. See usage of
            // MaterialDesignController::IsModeMaterial() in Chromium code.
            command_line->AppendSwitchWithValue("top-chrome-md", "non-material");
        }
        
        if (!command_line->HasSwitch(casper::cef3::common::client::switches::kCachePath) &&
            !command_line->HasSwitch("disable-gpu-shader-disk-cache")) {
            // Don't create a "GPUCache" directory when cache-path is unspecified.
            command_line->AppendSwitch("disable-gpu-shader-disk-cache");
        }
        
        DelegateSet::iterator it = delegates_.begin();
        for (; it != delegates_.end(); ++it)
            (*it)->OnBeforeCommandLineProcessing(this, command_line);
    }
}

void casper::cef3::client::browser::ClientAppBrowser::OnContextInitialized ()
{
    // Register cookieable schemes with the global cookie manager.
    CefRefPtr<CefCookieManager> manager =
    CefCookieManager::GetGlobalManager(NULL);
    DCHECK(manager.get());
    manager->SetSupportedSchemes(cookieable_schemes_, NULL);
    
    DelegateSet::iterator it = delegates_.begin();
    for (; it != delegates_.end(); ++it)
        (*it)->OnContextInitialized(this);
}

void casper::cef3::client::browser::ClientAppBrowser::OnBeforeChildProcessLaunch (CefRefPtr<CefCommandLine> command_line)
{
    DelegateSet::iterator it = delegates_.begin();
    for (; it != delegates_.end(); ++it)
        (*it)->OnBeforeChildProcessLaunch(this, command_line);
}

void casper::cef3::client::browser::ClientAppBrowser::OnRenderProcessThreadCreated (CefRefPtr<CefListValue> extra_info)
{
    DelegateSet::iterator it = delegates_.begin();
    for (; it != delegates_.end(); ++it)
        (*it)->OnRenderProcessThreadCreated(this, extra_info);
}

void casper::cef3::client::browser::ClientAppBrowser::OnScheduleMessagePumpWork(int64 delay)
{
    // Only used when `--external-message-pump` is passed via the command-line.
    casper::cef3::browser::MainMessageLoopExternalPump* message_pump = casper::cef3::browser::MainMessageLoopExternalPump::Get();
    if ( message_pump ) {
        message_pump->OnScheduleMessagePumpWork(delay);
    }
}

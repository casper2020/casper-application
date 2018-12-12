// Copyright (c) 2016 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef3/client/browser/client_browser_delegate.h"

#include "cef3/common/client/switches.h"

casper::cef3::client::browser::ClientBrowserDelegate::ClientBrowserDelegate ()
{
    
}

void casper::cef3::client::browser::ClientBrowserDelegate::OnContextInitialized (CefRefPtr<ClientAppBrowser> app)
{
    if (CefCrashReportingEnabled()) {
        // Set some crash keys for testing purposes. Keys must be defined in the
        // "crash_reporter.cfg" file. See cef_crash_util.h for details.
        CefSetCrashKeyValue("testkey_small1", "value1_small_browser");
        CefSetCrashKeyValue("testkey_small2", "value2_small_browser");
        CefSetCrashKeyValue("testkey_medium1", "value1_medium_browser");
        CefSetCrashKeyValue("testkey_medium2", "value2_medium_browser");
        CefSetCrashKeyValue("testkey_large1", "value1_large_browser");
        CefSetCrashKeyValue("testkey_large2", "value2_large_browser");
    }
    
    const std::string& crl_sets_path =
    CefCommandLine::GetGlobalCommandLine()->GetSwitchValue(casper::cef3::common::client::switches::kCRLSetsPath);
    if (!crl_sets_path.empty()) {
        // Load the CRLSets file from the specified path.
        CefLoadCRLSetsFile(crl_sets_path);
    }
}

// static
void casper::cef3::client::browser::ClientAppBrowser::CreateDelegates(casper::cef3::client::browser::ClientAppBrowser::DelegateSet& delegates) {
    delegates.insert(new casper::cef3::client::browser::ClientBrowserDelegate);
}


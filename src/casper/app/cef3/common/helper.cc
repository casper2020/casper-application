// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "casper/app/cef3/common/helper.h"

#include "include/cef_client.h"
#include "include/cef_parser.h" // CefBase64Encode

#include "cef3/browser/main_context.h"
#include "cef3/browser/root_window_manager.h"

#include "include/base/cef_string16.h"

bool casper::app::cef3::common::Helper::Reload (CefRefPtr<CefBrowser> a_browser)
{
    if ( ! a_browser.get() ) {
        return false;
    }
    a_browser->ReloadIgnoreCache();
    return true;
}

void casper::app::cef3::common::Helper::ZoomIn (CefRefPtr<CefBrowser> a_browser)
{
    SetZoom(a_browser, 0.5);
}

void casper::app::cef3::common::Helper::ZoomOut (CefRefPtr<CefBrowser> a_browser)
{
   SetZoom(a_browser, -0.5);
}

void casper::app::cef3::common::Helper::ZoomReset (CefRefPtr<CefBrowser> a_browser)
{
    if ( ! a_browser.get() ) {
        return;
    }
    if ( !CefCurrentlyOn(TID_UI) ) {
        CefPostTask(TID_UI, base::Bind(&casper::app::cef3::common::Helper::ZoomReset, a_browser));
    } else {
        a_browser->GetHost()->SetZoomLevel(0.0);
    }
}

void casper::app::cef3::common::Helper::SetZoom (CefRefPtr<CefBrowser> a_browser, double a_delta)
{
    if ( ! a_browser.get() ) {
        return;
    }
    if ( !CefCurrentlyOn(TID_UI) ) {
        CefPostTask(TID_UI, base::Bind(&casper::app::cef3::common::Helper::SetZoom, a_browser, a_delta));
    } else {
        a_browser->GetHost()->SetZoomLevel(a_browser->GetHost()->GetZoomLevel() + a_delta);
    }
}

bool casper::app::cef3::common::Helper::ShowDeveloperTools (CefRefPtr<CefBrowser> a_browser, const CefPoint a_point)
{
 
    if ( ! a_browser.get() ) {
        return false;
    }
    
    CefWindowInfo windowInfo;
    
    
    const std::string name = "Developer Tools";
    
    cef_string_utf8_to_utf16(name.c_str(), name.length(), &windowInfo.window_name);
    
    CefRefPtr<CefClient> client;
    CefBrowserSettings settings;
    
    
    CefRefPtr<CefBrowserHost> host = a_browser->GetHost();
    
    bool has_devtools = host->HasDevTools();
    
    if ( false == has_devtools ) {
        // Create a new RootWindow for the DevTools browser that will be created
        // by ShowDevTools().
        has_devtools = CreatePopupWindow(a_browser, true, CefPopupFeatures(), windowInfo, client, settings);
    }
    
    if ( true == has_devtools ) {
        host->ShowDevTools(windowInfo, client, settings, a_point);
    }
    
    
    return true;
}

void casper::app::cef3::common::Helper::HideDeveloperTools (CefRefPtr<CefBrowser> a_browser)
{
    CEF_REQUIRE_UI_THREAD();
    
    CefRefPtr<CefBrowserHost> host = a_browser->GetHost();
    if ( true == host->HasDevTools() ) {
        host->CloseDevTools();
    }
}

// Create a new popup window using the specified information. |is_devtools|
// will be true if the window will be used for DevTools. Return true to
// proceed with popup browser creation or false to cancel the popup browser.
// May be called on any thead.
bool casper::app::cef3::common::Helper::CreatePopupWindow(CefRefPtr<CefBrowser> a_browser,
                                                          bool a_is_dev_tools,
                                                          const CefPopupFeatures& a_popup_features,
                                                          CefWindowInfo& a_window_info,
                                                          CefRefPtr<CefClient>& a_client,
                                                          CefBrowserSettings& a_settings)
{
    CEF_REQUIRE_UI_THREAD();
    
    // The popup browser will be parented to a new native window.
    // Don't show URL bar and navigation buttons on DevTools windows.
    casper::cef3::browser::MainContext::Get()->GetRootWindowManager()->CreateRootWindowAsPopup(
        false == a_is_dev_tools, /* TODO NOW is_osr() */ false, a_popup_features, a_window_info, a_client, a_settings
    );
    
    return true;
}

void casper::app::cef3::common::Helper::ShowInfo (const std::string& a_mime_type, const std::string& a_data,
                                                  const bool a_is_osr)
{
    CEF_REQUIRE_UI_THREAD();

    casper::cef3::browser::RootWindowConfig config;
    config.with_controls = false;
    config.with_osr      = a_is_osr;
    config.url           = "data:" + a_mime_type + ";base64," + CefURIEncode(CefBase64Encode(a_data.data(), a_data.size()), false) .ToString();
    
    casper::cef3::browser::MainContext::Get()->GetRootWindowManager()->CreateRootWindow(config);
}

const cef_string_utf8_t& casper::app::cef3::common::Helper::ToUTF8String (const cef_string_t& a_string, cef_string_utf8_t& o_string)
{
    o_string.str    = nullptr;
    o_string.length = 0;
    cef_string_utf16_to_utf8(a_string.str, a_string.length, &o_string);
    return o_string;
}

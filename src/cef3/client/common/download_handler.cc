// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef3/client/common/download_handler.h"

#include "include/wrapper/cef_helpers.h" // CEF_REQUIRE_UI_THREAD

#ifdef __APPLE__
#pragma mark - CefContextMenuHandler
#endif

void casper::cef3::client::common::DownloadHandler::OnBeforeDownload (CefRefPtr<CefBrowser> a_browser,
                                                                      CefRefPtr<CefDownloadItem> a_download_item, const CefString& a_suggested_name,
                                                                      CefRefPtr<CefBeforeDownloadCallback> a_callback)
{
    CEF_REQUIRE_UI_THREAD();
    // TODO CW
    //    std::string uri;
    //    auto context = casper::cef3::browser::MainContext::Get();
    //    if ( false == context->GetDownloadURI(a_suggested_name, uri) ) {
    //        return;
    //    }
    //    a_callback->Continue(uri, /* a_show_dialog */ true);
}

void casper::cef3::client::common::DownloadHandler::OnDownloadUpdated (CefRefPtr<CefBrowser> a_browser,
                                                                       CefRefPtr<CefDownloadItem> a_download_item,
                                                                       CefRefPtr<CefDownloadItemCallback> a_callback)
{
    CEF_REQUIRE_UI_THREAD();
    if ( true == a_download_item->IsComplete() ) {
        const std::string uri = a_download_item->GetFullPath().ToString();
        fprintf(stdout, "File downloaded to: %s\n", uri.c_str());
    }
}

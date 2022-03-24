// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CASPER_CEF3_CLIENT_COMMON_DOWNLOAD_HANDLER_H_
#define CASPER_CEF3_CLIENT_COMMON_DOWNLOAD_HANDLER_H_

#pragma once

#include "include/cef_download_handler.h"     // CefDownloadHandler

#include "include/base/cef_ref_counted.h"     // CefRefPtr

#include "cef3/client/common/base_handler.h"

namespace casper
{
    
    namespace cef3
    {
        
        namespace client
        {
            
            namespace common
            {
                
                class DownloadHandler : public CefDownloadHandler
                {
                    
                    IMPLEMENT_REFCOUNTING(DownloadHandler);
                    
                public: // CefDownloadHandler Method(s) / Function(s)
                    
                    void OnBeforeDownload (CefRefPtr<CefBrowser> a_browser,
                                           CefRefPtr<CefDownloadItem> a_download_item, const CefString& a_suggested_name,
                                           CefRefPtr<CefBeforeDownloadCallback> a_callback) override;
                    
                    void OnDownloadUpdated(CefRefPtr<CefBrowser> a_browser,
                                           CefRefPtr<CefDownloadItem> a_download_item,
                                           CefRefPtr<CefDownloadItemCallback> a_callback) override;

                }; // end of class 'DownloadHandler'
                
            } // end of namespace 'common'
            
        } // end of namespace 'client'
        
    } // end of namespace 'cef3'
    
}  // end of namespace 'casper'

#endif // CASPER_CEF3_CLIENT_COMMON_DOWNLOAD_HANDLER_H_

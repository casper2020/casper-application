// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CASPER_CEF3_CLIENT_COMMON_HELPER_H_
#define CASPER_CEF3_CLIENT_COMMON_HELPER_H_
#pragma once

#include "include/base/cef_ref_counted.h"

#include "include/cef_browser.h"

#include "include/cef_menu_model.h"

#include <vector>

namespace casper
{
    
    namespace app
    {
        
        namespace cef3
        {
            
            namespace common
            {
                
                class Helper final
                {
                    
                public:
                    
                    static bool Reload             (CefRefPtr<CefBrowser> a_browser);
                    static void ZoomIn             (CefRefPtr<CefBrowser> a_browser);
                    static void ZoomOut            (CefRefPtr<CefBrowser> a_browser);
                    static void ZoomReset          (CefRefPtr<CefBrowser> a_browser);
                    static void SetZoom            (CefRefPtr<CefBrowser> a_browser, double a_delta);
                    static bool ShowDeveloperTools (CefRefPtr<CefBrowser> a_browser,
                                                    const CefPoint a_point = CefPoint());
                    static void HideDeveloperTools (CefRefPtr<CefBrowser> a_browser);
                    
                    static bool CreatePopupWindow(CefRefPtr<CefBrowser> a_browser,
                                                  bool a_is_dev_tools,
                                                  const CefPopupFeatures& a_popup_features,
                                                  CefWindowInfo& a_window_info,
                                                  CefRefPtr<CefClient>& a_client,
                                                  CefBrowserSettings& a_settings
                                                  );
                    
                    static void ShowInfo (const std::string& a_mime_type, const std::string& a_data,
                                          const bool a_is_osr);
                    
                    static const cef_string_utf8_t& ToUTF8String(const cef_string_t& a_string, cef_string_utf8_t& o_string);

                    
                }; // end of class 'Helper'
                
            } // end of namespace 'common'
            
        } // end of namespace 'cef3'
        
    } // end of namespace 'app'
    
} // end of namespace 'casper'

#endif // CASPER_CEF3_CLIENT_COMMON_HELPER_H_


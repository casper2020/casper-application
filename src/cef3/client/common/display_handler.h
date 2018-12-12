// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CASPER_CEF3_CLIENT_DISPLAY_HANDLER_H_
#define CASPER_CEF3_CLIENT_DISPLAY_HANDLER_H_

#pragma once

#include "include/cef_display_handler.h"  // CefDisplayHandler

#include "include/base/cef_ref_counted.h" // CefRefPtr

#include "cef3/client/common/base_handler.h"

namespace casper
{
    
    namespace cef3
    {
        
        namespace client
        {
            
            namespace common
            {
                class DisplayHandler : public CefDisplayHandler
                {
                
                    IMPLEMENT_REFCOUNTING(DisplayHandler);
                    
                protected: // Ptrs
                    
                    CefRefPtr<BaseHandler> base_handler_;                    
                    
                private: // Const Data
                    
                    // Console logging state.
                    const std::string console_log_file_;
                    
                public: // Constructor(s) / Destructor
                    
                    DisplayHandler (CefRefPtr<BaseHandler> a_base_handler);
                    virtual ~DisplayHandler ();

                public: // CefDisplayHandler Method(s) / Function(s)
                    
                    bool OnConsoleMessage       (CefRefPtr<CefBrowser> a_browser,
                                                 cef_log_severity_t a_level, const CefString& a_message, const CefString& a_source, int a_line) OVERRIDE;
                    
                protected: // Virtual Method(s) / Function(s)
                    
                    virtual bool WriteToConsoleLog (const std::string& a_message);

                    
                private: // Execute Delegate notifications on the main thread.
                    
                    void NotifyAddress(const CefString& url);
                    void NotifyTitle(const CefString& title);
                    void NotifyFavicon(CefRefPtr<CefImage> image);
                    void NotifyFullscreen(bool fullscreen);
                    void NotifyAutoResize(const CefSize& new_size);
                    
                }; // end of class 'DisplayHandler'
                
            } // end of namespace 'common'
            
        } // end of namespace 'client'
        
    } // end of namespace 'cef3'
    
}  // end of namespace 'casper'

#endif // CASPER_CEF3_CLIENT_DISPLAY_HANDLER_H_


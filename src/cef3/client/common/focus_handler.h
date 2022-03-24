// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CASPER_CEF3_CLIENT_FOCUS_HANDLER_H_
#define CASPER_CEF3_CLIENT_FOCUS_HANDLER_H_

#pragma once

#include "include/cef_focus_handler.h"    // CefFocusHandler

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
                
                class FocusHandler : public CefFocusHandler
                {
                    
                    IMPLEMENT_REFCOUNTING(FocusHandler);
                    
                protected: // Ptrs
                    
                    CefRefPtr<BaseHandler> base_handler_;
                    
                public: // Constructor(s) / Destructor
                    
                    FocusHandler (CefRefPtr<BaseHandler> a_base_handler);
                    virtual ~FocusHandler ();

                public: // CefFocusHandler Method(s) / Function(s)
                    
                    void OnTakeFocus(CefRefPtr<CefBrowser> browser, bool next) override;
                    
                protected:
                    
                    void NotifyTakeFocus (bool next);
                    
                }; // end of class 'FocusHandler'
                
            } // end of namespace 'common'
            
        } // end of namespace 'client'
        
    } // end of namespace 'cef3'
    
}  // end of namespace 'casper'

#endif // CASPER_CEF3_CLIENT_FOCUS_HANDLER_H_

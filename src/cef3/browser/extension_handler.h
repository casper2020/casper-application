// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CASPER_CEF3_BROWSER_EXTENSION_HANDLER_H_
#define CASPER_CEF3_BROWSER_EXTENSION_HANDLER_H_

#pragma once

#include "include/cef_extension_handler.h"       // CefExtensionHandler

namespace casper
{
    
    namespace cef3
    {
        
        namespace browser
        {
            
            class ExtensionHandler : public CefExtensionHandler
            {
                
            public: // Constructor(s) / Destructor
                
                ExtensionHandler ();
                
            private:
                
                IMPLEMENT_REFCOUNTING(ExtensionHandler);
                DISALLOW_COPY_AND_ASSIGN(ExtensionHandler);
                
            public: // Overriden Method(s) / Function(s) - from CefExtensionHandler
                
                void OnExtensionLoaded(CefRefPtr<CefExtension> extension) override;
                
                CefRefPtr<CefBrowser> GetActiveBrowser(CefRefPtr<CefExtension> extension,
                                                       CefRefPtr<CefBrowser> browser,
                                                       bool include_incognito) override;
                
            }; // end of class 'ExtensionHandler'
            
        } // end of namespace 'client'
        
    } // end of namespace 'cef3'
    
}  // end of namespace 'casper'

#endif // CASPER_CEF3_BROWSER_EXTENSION_HANDLER_H_


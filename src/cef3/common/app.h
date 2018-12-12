// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CASPER_CEF3_COMMON_APP_H_
#define CASPER_CEF3_COMMON_APP_H_

#pragma once

#include "include/cef_app.h"

namespace casper
{
    
    namespace cef3
    {
        
        namespace common
        {
            
            class App : public CefApp
            {

            protected:
                // Schemes that will be registered with the global cookie manager.
                std::vector<CefString> cookieable_schemes_;

            public: // Constructor(s) / Destructor(s)
                
                App();
                
            public: // Static Method(s) / Function(s)
                
                void OnRegisterCustomSchemes (CefRawPtr<CefSchemeRegistrar> registrar) OVERRIDE;
                
            private:
                
                IMPLEMENT_REFCOUNTING(App);
                DISALLOW_COPY_AND_ASSIGN(App);
                
            }; // end of class 'App'

        } // end of namespace 'common'
        
    } // end of namespace 'cef3'
    
}  // end of namespace 'casper'

#endif // CASPER_CEF3_COMMON_APP_H_

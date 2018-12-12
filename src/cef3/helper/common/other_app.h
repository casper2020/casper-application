// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CASPER_CEF3_HELPER_COMMON_OTHER_APP_H_
#define CASPER_CEF3_HELPER_COMMON_OTHER_APP_H_

#pragma once

#include "cef3/common/app.h"

namespace casper
{
    
    namespace cef3
    {
        
        namespace helper
        {
            
            namespace common
            {
                
                class OtherApp : public casper::cef3::common::App
                {
                    
                public: // Constructor(s) / Destructor(s)
                    
                    OtherApp();
                    
                private:
                    
                    IMPLEMENT_REFCOUNTING(OtherApp);
                    DISALLOW_COPY_AND_ASSIGN(OtherApp);
                    
                }; // end of class 'OtherApp'
                
            } // end of namespace 'common'
            
        } // end of namespace 'helper'
        
    } // end of namespace 'cef3'
    
}  // end of namespace 'casper'

#endif // CASPER_CEF3_HELPER_COMMON_OTHER_APP_H_

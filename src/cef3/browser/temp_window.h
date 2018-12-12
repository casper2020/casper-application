// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CASPER_CEF3_BROWSER_TEMP_WINDOW_H_
#define CASPER_CEF3_BROWSER_TEMP_WINDOW_H_
#pragma once

#if defined(OS_MACOSX)
    #include "cef3/browser/mac/temp_window.h"
#endif


namespace casper
{
    
    namespace cef3
    {
        
        namespace browser
        {
            
#if defined(OS_MACOSX)
            typedef casper::cef3::browser::TempWindowMAC TempWindow;
#endif

        } // end of namespace 'browser'
        
    } // end of namespace 'cef3'
    
} // end of namespace 'casper'

#endif // CASPER_CEF3_BROWSER_TEMP_WINDOW_H_

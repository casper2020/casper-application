// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CASPER_CEF3_COMMON_MAIN_H_
#define CASPER_CEF3_COMMON_MAIN_H_

#pragma once

#include "include/base/cef_ref_counted.h" // CefRefPtr
#include "include/cef_command_line.h"     // CefCommandLine

namespace casper
{
    
    namespace cef3
    {
        
        namespace common
        {
            
            class Main
            {
                
            public: // Enum(s)
                
                enum ProcessType {
                    Browser,
                    Renderer,
                    Other,
                };
                
            public: // Static Method(s) / Function(s)
                
                static ProcessType GetProcessType (const CefRefPtr<CefCommandLine> a_command_line);
                
            }; // end of class 'Main'
            
        } // end of namespace 'common'
        
    } // end of namespace 'cef3'
    
}  // end of namespace 'casper'

#endif // CASPER_CEF3_COMMON_MAIN_H_

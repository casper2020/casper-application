// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CASPER_CEF3_BROWSER_MAIN_MESSAGE_LOOP_STD_H_
#define CASPER_CEF3_BROWSER_MAIN_MESSAGE_LOOP_STD_H_

#pragma once

#include "cef3/browser/main_message_loop.h"

namespace casper
{
    
    namespace cef3
    {
        
        namespace browser
        {
            
            // Represents the main message loop in the browser process.
            // This implementation is a light-weight wrapper around the Chromium UI thread.
            class MainMessageLoopStd : public MainMessageLoop
            {
                
            public: // Consructor/s) / Destructor
                
                MainMessageLoopStd();
                
            public: // Overriden Method(s) / Function(s) - from MainMessageLoop
                
                // MainMessageLoop methods.
                int  Run                      ()                        OVERRIDE;
                void Quit                     ()                        OVERRIDE;
                void PostTask                 (CefRefPtr<CefTask> task) OVERRIDE;
                bool RunsTasksOnCurrentThread () const                  OVERRIDE;
                
            private:
                
                DISALLOW_COPY_AND_ASSIGN(MainMessageLoopStd);
                
            };
            
        } // end of namespace 'browser'
        
    } // end of namespace 'cef3'
    
} // end of namespace 'casper'

#endif // CASPER_CEF3_BROWSER_MAIN_MESSAGE_LOOP_STD_H_

// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CASPER_CEF3_CLIENT_KEYBOARD_HANDLER_H_
#define CASPER_CEF3_CLIENT_KEYBOARD_HANDLER_H_

#pragma once

#include "include/cef_keyboard_handler.h" // CefKeyboardHandler

#include "include/base/cef_ref_counted.h" // CefRefPtr

namespace casper
{
    
    namespace cef3
    {
        
        namespace client
        {
            
            namespace common
            {
                
                class KeyboardHandler : public CefKeyboardHandler
                {
                    
                    IMPLEMENT_REFCOUNTING(KeyboardHandler);
                    
                public: // CefFocusHandler Method(s) / Function(s)
                    
                    bool OnPreKeyEvent (CefRefPtr<CefBrowser> browser,
                                        const CefKeyEvent& event, CefEventHandle os_event,
                                        bool* is_keyboard_shortcut) override;
                    
                }; // end of class 'KeyboardHandler'
                
            } // end of namespace 'common'
            
        } // end of namespace 'client'
        
    } // end of namespace 'cef3'
    
}  // end of namespace 'casper'

#endif // CASPER_CEF3_CLIENT_KEYBOARD_HANDLER_H_

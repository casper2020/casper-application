// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef3/client/common/keyboard_handler.h"

#ifdef __APPLE__
#pragma mark - CefKeyboardHandler
#endif

#include "include/wrapper/cef_helpers.h" // CEF_REQUIRE_UI_THREAD

bool casper::cef3::client::common::KeyboardHandler::OnPreKeyEvent (CefRefPtr<CefBrowser> browser,
                                                                   const CefKeyEvent& event, CefEventHandle os_event,
                                                                   bool* is_keyboard_shortcut)
{
    CEF_REQUIRE_UI_THREAD();
    
    if ( !event.focus_on_editable_field && event.windows_key_code == 0x20 ) {
        // Special handling for the space character when an input element does not
        // have focus. Handling the event in OnPreKeyEvent() keeps the event from
        // being processed in the renderer. If we instead handled the event in the
        // OnKeyEvent() method the space key would cause the window to scroll in
        // addition to showing the alert box.
        return true;
    }
    
    return false;
}

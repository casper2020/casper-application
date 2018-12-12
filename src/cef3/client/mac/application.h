// Copyright (c) 2017 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#import <Cocoa/Cocoa.h>

#import "include/cef_application_mac.h"

@interface Application : NSApplication<CefAppProtocol>
{
    @private
        BOOL handlingSendEvent_;
}
@end

// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#import <Cocoa/Cocoa.h>

#include "cef3/browser/mac/root_window.h"

// Receives notifications from controls and the browser window. Will delete
// itself when done.
@interface RootWindowDelegate : NSObject<NSWindowDelegate> {
@private
    NSWindow*                             window_;
    casper::cef3::browser::RootWindowMAC* root_window_;
    bool                                  force_close_;
}

@property(nonatomic, readonly)  casper::cef3::browser::RootWindowMAC* root_window;
@property(nonatomic, readwrite) bool                                  force_close;

- (id)initWithWindow:(NSWindow*)window
       andRootWindow:(casper::cef3::browser::RootWindowMAC*)root_window;
- (IBAction)stopLoading:(id)sender;
- (IBAction)reload:(id)sender;
@end

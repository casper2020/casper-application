// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#import "cef3/browser/mac/root_window_delegate.h"

@implementation RootWindowDelegate

@synthesize root_window = root_window_;
@synthesize force_close = force_close_;

- (id)initWithWindow:(NSWindow*)window
       andRootWindow:(casper::cef3::browser::RootWindowMAC*)root_window
{
    if (self = [super init]) {
        window_ = window;
        [window_ setDelegate:self];
        root_window_ = root_window;
        force_close_ = false;
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(applicationDidHide:)
                                                     name:NSApplicationDidHideNotification
                                                   object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(applicationDidUnhide:)
                                                     name:NSApplicationDidUnhideNotification
                                                   object:nil];
    }
    return self;
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    
    [super dealloc];
}

- (IBAction)reload:(id)sender
{
    CefRefPtr<CefBrowser> browser = root_window_->GetBrowser();
    if (browser.get())
        browser->Reload();
}

- (IBAction)stopLoading:(id)sender
{
    CefRefPtr<CefBrowser> browser = root_window_->GetBrowser();
    if (browser.get())
        browser->StopLoad();
}

// Called when we are activated (when we gain focus).
- (void)windowDidBecomeKey:(NSNotification*)notification
{
    casper::cef3::browser::BrowserWindow* browser_window = root_window_->browser_window();
    if (browser_window)
        browser_window->SetFocus(true);
    root_window_->delegate()->OnRootWindowActivated(root_window_);
}

// Called when we are deactivated (when we lose focus).
- (void)windowDidResignKey:(NSNotification*)notification
{
    casper::cef3::browser::BrowserWindow* browser_window = root_window_->browser_window();
    if (browser_window)
        browser_window->SetFocus(false);
}

// Called when we have been minimized.
- (void)windowDidMiniaturize:(NSNotification*)notification
{
    casper::cef3::browser::BrowserWindow* browser_window = root_window_->browser_window();
    if (browser_window)
        browser_window->Hide();
}

// Called when we have been unminimized.
- (void)windowDidDeminiaturize:(NSNotification*)notification
{
    casper::cef3::browser::BrowserWindow* browser_window = root_window_->browser_window();
    if (browser_window)
        browser_window->Show();
}

// Called when the application has been hidden.
- (void)applicationDidHide:(NSNotification*)notification
{
    // If the window is miniaturized then nothing has really changed.
    if (![window_ isMiniaturized]) {
        casper::cef3::browser::BrowserWindow* browser_window = root_window_->browser_window();
        if (browser_window)
            browser_window->Hide();
    }
}

// Called when the application has been unhidden.
- (void)applicationDidUnhide:(NSNotification*)notification
{
    // If the window is miniaturized then nothing has really changed.
    if (![window_ isMiniaturized]) {
        casper::cef3::browser::BrowserWindow* browser_window = root_window_->browser_window();
        if (browser_window)
            browser_window->Show();
    }
}

// Called when the window is about to close. Perform the self-destruction
// sequence by getting rid of the window. By returning YES, we allow the window
// to be removed from the screen.
- (BOOL)windowShouldClose:(id)window
{
    if (!force_close_) {
        casper::cef3::browser::BrowserWindow* browser_window = root_window_->browser_window();
        if (browser_window && !browser_window->IsClosing()) {
            CefRefPtr<CefBrowser> browser = browser_window->GetBrowser();
            if (browser.get()) {
                // Notify the browser window that we would like to close it. This
                // will result in a call to ClientHandler::DoClose() if the
                // JavaScript 'onbeforeunload' event handler allows it.
                browser->GetHost()->CloseBrowser(false);
                
                // Cancel the close.
                return NO;
            }
        }
    }
    
    // Try to make the window go away.
    [window autorelease];
    
    // Clean ourselves up after clearing the stack of anything that might have the
    // window on it.
    [self performSelectorOnMainThread:@selector(cleanup:)
                           withObject:window
                        waitUntilDone:NO];
    
    // Allow the close.
    return YES;
}

// Deletes itself.
- (void)cleanup:(id)window
{
    root_window_->WindowDestroyed();
    
    // Don't want any more delegate callbacks after we destroy ourselves.
    [window setDelegate:nil];
    
    [self release];
}

@end


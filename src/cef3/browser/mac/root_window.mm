// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef3/browser/mac/root_window.h"

#import "cef3/browser/mac/root_window_delegate.h"

#include "include/base/cef_bind.h"
#include "include/cef_app.h"
#include "include/cef_application_mac.h"

#include "cef3/browser/mac/browser_window_std_mac.h"

#include "cef3/browser/temp_window.h"

#include "cef3/common/client/switches.h"

#include "cef3/browser/main_context.h"
#include "cef3/browser/main_message_loop.h"

#include "cef3/browser/root_window_manager.h"

#include "casper/app/cef3/common/helper.h"

/**
 * @brief Default constructor.
 */
casper::cef3::browser::RootWindowMAC::RootWindowMAC ()
    : with_osr_(false),
      is_popup_(false),
      initialized_(false),
      window_(nil),
      window_destroyed_(false),
      browser_destroyed_(false)
{
    name_.str    = nullptr;
    name_.length = 0;
}

casper::cef3::browser::RootWindowMAC::~RootWindowMAC ()
{
    REQUIRE_MAIN_THREAD();
    
    // The window and browser should already have been destroyed.
    DCHECK(window_destroyed_);
    DCHECK(browser_destroyed_);
}

void casper::cef3::browser::RootWindowMAC::Init (casper::cef3::browser::RootWindow::Delegate* delegate,
                                             const casper::cef3::browser::RootWindowConfig& config,
                                             const CefBrowserSettings& settings)
{
    DCHECK(delegate);
    DCHECK(!initialized_);
    
    delegate_ = delegate;
    with_osr_ = config.with_osr;
    with_extension_ = config.with_extension;
    start_rect_ = config.bounds;
    
    CreateBrowserWindow(config.url);
    
    initialized_ = true;
    
    // Create the native root window on the main thread.
    if (CURRENTLY_ON_MAIN_THREAD()) {
        CreateRootWindow(settings, config.initially_hidden);
    } else {
        MAIN_POST_CLOSURE(base::Bind(&::casper::cef3::browser::RootWindowMAC::CreateRootWindow, this, settings, config.initially_hidden));
    }
}

void casper::cef3::browser::RootWindowMAC::InitAsPopup (casper::cef3::browser::RootWindow::Delegate* delegate,
                                                    bool with_controls,
                                                    bool with_osr,
                                                    const CefPopupFeatures& popupFeatures,
                                                    CefWindowInfo& windowInfo,
                                                    CefRefPtr<CefClient>& client,
                                                    CefBrowserSettings& settings)
{
    DCHECK(delegate);
    DCHECK(!initialized_);
    
    delegate_ = delegate;
    with_osr_ = with_osr;
    is_popup_ = true;
        
    name_ = windowInfo.window_name;
    
    if (popupFeatures.xSet)
        start_rect_.x = popupFeatures.x;
    if (popupFeatures.ySet)
        start_rect_.y = popupFeatures.y;
    if (popupFeatures.widthSet)
        start_rect_.width = popupFeatures.width;
    if (popupFeatures.heightSet)
        start_rect_.height = popupFeatures.height;
    
    CreateBrowserWindow(::std::string());
    
    initialized_ = true;
    
    // The new popup is initially parented to a temporary window. The native root
    // window will be created after the browser is created and the popup window
    // will be re-parented to it at that time.
    browser_window_->GetPopupConfig(casper::cef3::browser::TempWindow::GetWindowHandle(), windowInfo, client, settings);
}

void casper::cef3::browser::RootWindowMAC::Show(ShowMode mode) {
    REQUIRE_MAIN_THREAD();
    
    if (!window_)
        return;
    
    const bool is_visible = [window_ isVisible];
    const bool is_minimized = [window_ isMiniaturized];
    const bool is_maximized = [window_ isZoomed];
    
    if ((mode == ShowMinimized && is_minimized) ||
        (mode == ShowMaximized && is_maximized) ||
        (mode == ShowNormal && is_visible)) {
        // The window is already in the desired state.
        return;
    }
    
    // Undo the previous state since it's not the desired state.
    if (is_minimized)
        [window_ deminiaturize:nil];
    else if (is_maximized)
        [window_ performZoom:nil];
    
    // Window visibility may change after (for example) deminiaturizing the
    // window.
    if (![window_ isVisible])
        [window_ makeKeyAndOrderFront:nil];
    
    if (mode == ShowMinimized)
        [window_ performMiniaturize:nil];
    else if (mode == ShowMaximized)
        [window_ performZoom:nil];
}

void casper::cef3::browser::RootWindowMAC::Hide() {
    REQUIRE_MAIN_THREAD();
    
    if (!window_)
        return;
    
    // Undo miniaturization, if any, so the window will actually be hidden.
    if ([window_ isMiniaturized])
        [window_ deminiaturize:nil];
    
    // Hide the window.
    [window_ orderOut:nil];
}

void casper::cef3::browser::RootWindowMAC::SetBounds (int x, int y, size_t width, size_t height)
{
    REQUIRE_MAIN_THREAD();
    
    if ( !window_ ) {
        return;
    }
    
    // Desired content rectangle.
    NSRect content_rect;
    content_rect.size.width  = static_cast<int>(width);
    content_rect.size.height = static_cast<int>(height);
    
    // Convert to a frame rectangle.
    NSRect frame_rect = [window_ frameRectForContentRect:content_rect];
    frame_rect.origin.x = x;
    frame_rect.origin.y = y;
    
    [window_ setFrame:frame_rect display:YES];
}

void casper::cef3::browser::RootWindowMAC::Close (bool force)
{
    REQUIRE_MAIN_THREAD();
    
    if ( window_ ) {
        
        const NSRect   frame       = [window_ frame];
        const NSArray* windowFrame = [[NSArray alloc]
                                      initWithObjects: [NSNumber numberWithUnsignedInteger:frame.origin.x],
                                                       [NSNumber numberWithUnsignedInteger:frame.origin.y],
                                                       [NSNumber numberWithUnsignedInteger:frame.size.width],
                                                       [NSNumber numberWithUnsignedInteger:frame.size.height],
                                                        nil
        ];
        
        [[NSUserDefaults standardUserDefaults]setObject: windowFrame forKey: @"WindowFrame"];
        static_cast<RootWindowDelegate*>([window_ delegate]).force_close = force;
        [window_ performClose:nil];
    }
}

void casper::cef3::browser::RootWindowMAC::SetDeviceScaleFactor(float device_scale_factor) {
    REQUIRE_MAIN_THREAD();
    
    if (browser_window_ && with_osr_)
        browser_window_->SetDeviceScaleFactor(device_scale_factor);
}

float casper::cef3::browser::RootWindowMAC::GetDeviceScaleFactor() const {
    REQUIRE_MAIN_THREAD();
    
    if (browser_window_ && with_osr_)
        return browser_window_->GetDeviceScaleFactor();
    
    NOTREACHED();
    return 0.0f;
}

CefRefPtr<CefBrowser> casper::cef3::browser::RootWindowMAC::GetBrowser() const {
    REQUIRE_MAIN_THREAD();
    
    if (browser_window_)
        return browser_window_->GetBrowser();
    return NULL;
}

ClientWindowHandle casper::cef3::browser::RootWindowMAC::GetWindowHandle() const {
    REQUIRE_MAIN_THREAD();
    return [window_ contentView];
}

bool casper::cef3::browser::RootWindowMAC::WithWindowlessRendering() const {
    REQUIRE_MAIN_THREAD();
    return with_osr_;
}

bool casper::cef3::browser::RootWindowMAC::WithExtension() const {
    REQUIRE_MAIN_THREAD();
    return with_extension_;
}

void casper::cef3::browser::RootWindowMAC::WindowDestroyed() {
    window_ = nil;
    window_destroyed_ = true;
    NotifyDestroyedIfDone();
}

void casper::cef3::browser::RootWindowMAC::CreateBrowserWindow (const ::std::string& startup_url)
{
    if ( true == with_osr_ ) {
        // TODO CW
//        OsrRenderer::Settings settings = {};
//        MainContext::Get()->PopulateOsrSettings(&settings);
//        browser_window_.reset(new BrowserWindowOsrMac(this, startup_url, settings));
    } else {
        browser_window_.reset(new casper::cef3::browser::BrowserWindowStdMAC(this, startup_url));
    }
}

void casper::cef3::browser::RootWindowMAC::CreateRootWindow (const CefBrowserSettings& settings, bool initially_hidden)
{
    
    REQUIRE_MAIN_THREAD();
    DCHECK(!window_);

    auto context = casper::cef3::browser::MainContext::Get();
    
    const NSSize minimumSize = NSMakeSize(800, 600);
    
    NSRect  window_rect = NSZeroRect;
    Boolean center      = NO;
    
    const NSArray* windowBounds = [[NSUserDefaults standardUserDefaults]arrayForKey:@"WindowFrame"];
    if ( nil != windowBounds && 4 == [windowBounds count] ) {
        window_rect.origin.x    = [(NSNumber*)[windowBounds objectAtIndex: 0] unsignedIntValue];
        window_rect.origin.y    = [(NSNumber*)[windowBounds objectAtIndex: 1] unsignedIntValue];
        window_rect.size.width  = [(NSNumber*)[windowBounds objectAtIndex: 2] unsignedIntValue];
        window_rect.size.height = [(NSNumber*)[windowBounds objectAtIndex: 3] unsignedIntValue];
    } else if ( false == start_rect_.IsEmpty() ) {
        window_rect.origin.x    = 0;
        window_rect.origin.y    = 0;
        window_rect.size.width  = minimumSize.width;
        window_rect.size.height = minimumSize.height;
        center = YES;
    } else {
        window_rect.origin.x    = static_cast<NSUInteger>(start_rect_.x);
        window_rect.origin.y    = static_cast<NSUInteger>(start_rect_.y);
        window_rect.size.width  = static_cast<NSUInteger>(start_rect_.width);
        window_rect.size.height = static_cast<NSUInteger>(start_rect_.height);
    }
    
    // The CEF framework library is loaded at runtime so we need to use this
    // mechanism for retrieving the class.
    Class window_class = NSClassFromString(@"UnderlayOpenGLHostingWindow");
    CHECK(window_class);
    
    window_ = [[window_class alloc]
               initWithContentRect:window_rect
               styleMask:(NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask | NSUnifiedTitleAndToolbarWindowMask)
               backing:NSBackingStoreBuffered
               defer:NO
            ];
    
    NSString* windowTitle =  [[NSString alloc]initWithUTF8String: context->settings().product_.name_.c_str()];;
    if ( name_.length > 0 ) {
        cef_string_utf8_t title;
        windowTitle = [windowTitle
                       stringByAppendingFormat:@"- %@",
                       [[NSString alloc]initWithUTF8String: casper::app::cef3::common::Helper::ToUTF8String(name_, title).str]
        ];
    }
    [window_ setTitle: windowTitle];
    [window_ setMinSize: minimumSize];
    
    // Create the delegate for control and browser window events - very very important!
    [[RootWindowDelegate alloc] initWithWindow:window_ andRootWindow:this];
    
    // Rely on the window delegate to clean us up rather than immediately
    // releasing when the window gets closed. We use the delegate to do
    // everything from the autorelease pool so the window isn't on the stack
    // during cleanup (ie, a window close from javascript).
    [window_ setReleasedWhenClosed:NO];
    
    
    const cef_color_t background_color = context->settings().application_.window_.background_color_;
    
    if ( 0 != background_color ) {
        [window_ setBackgroundColor:[NSColor
                                     colorWithCalibratedRed:float(CefColorGetR(background_color)) / 255.0f
                                     green:float(CefColorGetG(background_color)) / 255.0f
                                     blue:float(CefColorGetB( background_color)) / 255.0f
                                     alpha:1.f]
         ];
    }
    
    NSView* contentView  = [window_ contentView];
    NSRect contentBounds = [contentView bounds];
    
    if (!with_osr_) {
        // Make the content view for the window have a layer. This will make all
        // sub-views have layers. This is necessary to ensure correct layer
        // ordering of all child views and their layers.
        [contentView setWantsLayer:YES];
    }
    
    if ( false == is_popup_ ) {
        // Create the browser window.
        browser_window_->CreateBrowser(contentView, CefRect(0, 0, static_cast<int>(window_rect.size.width), static_cast<int>(window_rect.size.height)),
                                       settings,
                                       delegate_->GetRequestContext(this));
    } else {
        // With popups we already have a browser window. Parent the browser window
        // to the root window and show it in the correct location.
        browser_window_->ShowPopup(contentView, 0, 0, contentBounds.size.width, contentBounds.size.height);
    }
    
    if (!initially_hidden) {
        // Show the window.
        Show(ShowNormal);
        
        // Size the window.
        SetBounds(static_cast<int>(window_rect.origin.x), static_cast<int>(window_rect.origin.y),
                  static_cast<int>(window_rect.size.width), static_cast<int>(window_rect.size.height)
        );
    }
    
    if ( YES == center ) {
        [window_ center];
    }
}

void casper::cef3::browser::RootWindowMAC::OnBrowserCreated(CefRefPtr<CefBrowser> browser) {
    REQUIRE_MAIN_THREAD();
    
    // For popup browsers create the root window once the browser has been
    // created.
    if (is_popup_)
        CreateRootWindow(CefBrowserSettings(), false);
    
    delegate_->OnBrowserCreated(this, browser);
}

void casper::cef3::browser::RootWindowMAC::OnBrowserWindowDestroyed() {
    REQUIRE_MAIN_THREAD();
    
    browser_window_.reset();
    
    if (!window_destroyed_) {
        // The browser was destroyed first. This could be due to the use of
        // off-screen rendering or execution of JavaScript window.close().
        // Close the RootWindow.
        Close(true);
    }
    
    browser_destroyed_ = true;
    NotifyDestroyedIfDone();
}

void casper::cef3::browser::RootWindowMAC::OnSetAddress (const ::std::string& /* a_url */)
{
    REQUIRE_MAIN_THREAD();
}

void casper::cef3::browser::RootWindowMAC::OnSetDraggableRegions (const ::std::vector<CefDraggableRegion>& regions)
{
    REQUIRE_MAIN_THREAD();
    // TODO(cef): Implement support for draggable regions on this platform.
}

void casper::cef3::browser::RootWindowMAC::OnSetTitle (const ::std::string& a_title)
{
    REQUIRE_MAIN_THREAD();
    if ( nullptr == window_ ) {
        return;
    }
    // TODO CW
#if 0
    auto context = static_cast<casper::cef3::client::mac::MainContext*>(casper::cef3::client::mac::MainContext::Get());
    if ( true == context->settings().application_.window_.allow_title_changes_ ) {
        [window_ setTitle: [NSString stringWithUTF8String: a_title.c_str()]];
    }
#endif
}

void casper::cef3::browser::RootWindowMAC::OnSetFullscreen(bool fullscreen)
{
    REQUIRE_MAIN_THREAD();
    
//    CefRefPtr<CefBrowser> browser = GetBrowser();
//    if (browser) {
//        scoped_ptr<window_test::WindowTestRunnerMac> test_runner(
//                                                                 new window_test::WindowTestRunnerMac());
//        if (fullscreen)
//            test_runner->Maximize(browser);
//        else
//            test_runner->Restore(browser);
//    }
}

void casper::cef3::browser::RootWindowMAC::OnAutoResize (const CefSize& new_size)
{
    REQUIRE_MAIN_THREAD();
    
    if (!window_)
        return;
    
    // Desired content rectangle.
    NSRect content_rect;
    content_rect.size.width  = static_cast<int>(new_size.width);
    content_rect.size.height = static_cast<int>(new_size.height);
    
    // Convert to a frame rectangle.
    NSRect frame_rect = [window_ frameRectForContentRect:content_rect];
    // Don't change the origin.
    frame_rect.origin = window_.frame.origin;
    
    [window_ setFrame:frame_rect display:YES];
    
    // Make sure the window is visible.
    Show(ShowNormal);
}

void casper::cef3::browser::RootWindowMAC::OnSetLoadingState(bool isLoading,
                                                              bool canGoBack,
                                                              bool canGoForward) {
    REQUIRE_MAIN_THREAD();
    
    // After Loading is done, check if voiceover is running and accessibility
    // should be enabled.
    if (!isLoading) {
        Boolean keyExists = false;
        // On OSX there is no API to query if VoiceOver is active or not. The value
        // however is stored in preferences that can be queried.
        if (CFPreferencesGetAppBooleanValue(CFSTR("voiceOverOnOffKey"),
                                            CFSTR("com.apple.universalaccess"),
                                            &keyExists)) {
            GetBrowser()->GetHost()->SetAccessibilityState(STATE_ENABLED);
        }
    }
}

void casper::cef3::browser::RootWindowMAC::NotifyDestroyedIfDone() {
    // Notify once both the window and the browser have been destroyed.
    if (window_destroyed_ && browser_destroyed_)
        delegate_->OnRootWindowDestroyed(this);
}

// static
scoped_refptr<casper::cef3::browser::RootWindow> casper::cef3::browser::RootWindow::GetForNSWindow(NSWindow* window) {
    RootWindowDelegate* delegate = static_cast<RootWindowDelegate*>([window delegate]);
    return [delegate root_window];
}

#import "casper/mac/app_delegate.h"

#import <Quartz/Quartz.h> // PDFDocument

#include "cef3/browser/root_window.h"
#include "cef3/browser/root_window_manager.h"

#include "cef3/browser/resource.h"

#include "include/cef_client.h"

#include "casper/app/cef3/common/context_menu_handler.h"
#include "casper/app/cef3/common/helper.h"

#include "cef3/client/mac/main_context.h"

namespace {
    
    // Returns the top menu bar with the specified |tag|.
    NSMenuItem* GetMenuBarMenuWithTag(NSInteger tag) {
        NSMenu* main_menu = [[NSApplication sharedApplication] mainMenu];
        NSInteger found_index = [main_menu indexOfItemWithTag:tag];
        if (found_index >= 0)
            return [main_menu itemAtIndex:found_index];
        return nil;
    }
    
    // Returns the item in |menu| that has the specified |action_selector|.
    NSMenuItem* GetMenuItemWithAction(NSMenu* menu, SEL action_selector) {
        for (NSInteger i = 0; i < menu.numberOfItems; ++i) {
            NSMenuItem* item = [menu itemAtIndex:i];
            if (item.action == action_selector)
                return item;
        }
        return nil;
    }
    
}  // namespace

@implementation AppDelegate

- (id)initWithControls:(bool)with_controls andOsr:(bool)with_osr {
    if (self = [super init]) {
        with_controls_ = with_controls;
        with_osr_ = with_osr;
    }
    return self;
}

// Create the application on the UI thread.
- (void)createApplication:(id)object {
    NSApplication* application = [NSApplication sharedApplication];
    
    // The top menu is configured using Interface Builder (IB). To modify the menu
    // start by loading MainMenu.xib in IB.
    //
    // To associate MainMenu.xib with AppDelegate:
    // 1. Select "File's Owner" from the "Placeholders" section in the left side
    //    pane.
    // 2. Load the "Identity inspector" tab in the top-right side pane.
    // 3. In the "Custom Class" section set the "Class" value to
    //    "AppDelegate".
    // 4. Pass an instance of AppDelegate as the |owner| parameter to
    //    loadNibNamed:.
    //
    // To create a new top menu:
    // 1. Load the "Object library" tab in the bottom-right side pane.
    // 2. Drag a "Submenu Menu Item" widget from the Object library to the desired
    //    location in the menu bar shown in the center pane.
    // 3. Select the newly created top menu by left clicking on it.
    // 4. Load the "Attributes inspector" tab in the top-right side pane.
    // 5. Under the "Menu Item" section set the "Tag" value to a unique integer.
    //    This is necessary for the GetMenuBarMenuWithTag function to work
    //    properly.
    //
    // To create a new menu item in a top menu:
    // 1. Add a new receiver method in AppDelegate (e.g. menuTestsDoStuff:).
    // 2. Load the "Object library" tab in the bottom-right side pane.
    // 3. Drag a "Menu Item" widget from the Object library to the desired
    //    location in the menu bar shown in the center pane.
    // 4. Double-click on the new menu item to set the label.
    // 5. Right click on the new menu item to show the "Get Source" dialog.
    // 6. In the "Sent Actions" section drag from the circle icon and drop on the
    //    new receiver method in the AppDelegate source code file.
    //
    // Load the top menu from MainMenu.xib.
    [[NSBundle mainBundle] loadNibNamed:@"MainMenu"
                                  owner:self
                        topLevelObjects:nil];
    
    // Set the delegate for application events.
    [application setDelegate:self];
    
    if (!with_osr_) {
        // Remove the OSR-related menu items when OSR is disabled.
        NSMenuItem* tests_menu = GetMenuBarMenuWithTag(8);
        if (tests_menu) {
            NSMenuItem* set_fps_item = GetMenuItemWithAction(
                                                             tests_menu.submenu, @selector(menuTestsSetFPS:));
            if (set_fps_item)
                [tests_menu.submenu removeItem:set_fps_item];
            NSMenuItem* set_scale_factor_item = GetMenuItemWithAction(
                                                                      tests_menu.submenu, @selector(menuTestsSetScaleFactor:));
            if (set_scale_factor_item)
                [tests_menu.submenu removeItem:set_scale_factor_item];
        }
    }
    
    casper::cef3::browser::RootWindowConfig window_config;
    window_config.with_controls = with_controls_;
    window_config.with_osr      = with_osr_;
    
    window_config.bounds.Set(0, 0, 1024, 768);
    
    // Create the first window.
    casper::cef3::browser::MainContext::Get()->GetRootWindowManager()->CreateRootWindow(window_config);
}

- (void)tryToTerminateApplication:(NSApplication*)app {
    casper::cef3::browser::MainContext::Get()->GetRootWindowManager()->CloseAllWindows(false);
}

- (void)orderFrontStandardAboutPanel:(id)sender {
    [[NSApplication sharedApplication] orderFrontStandardAboutPanel:nil];
}

- (IBAction)reload:(id)sender {
    NSWindow* key_window = [[NSApplication sharedApplication] keyWindow];
    if ( nil == key_window ) {
        return;
    }
    casper::app::cef3::common::Helper::Reload(casper::cef3::browser::RootWindow::GetForNSWindow(key_window)->GetBrowser());
}

- (IBAction)zoomIn:(id)sender
{
    NSWindow* key_window = [[NSApplication sharedApplication] keyWindow];
    if ( nil == key_window ) {
        return;
    }
    casper::app::cef3::common::Helper::ZoomIn(casper::cef3::browser::RootWindow::GetForNSWindow(key_window)->GetBrowser());
}

- (IBAction)zoomOut:(id)sender
{
    NSWindow* key_window = [[NSApplication sharedApplication] keyWindow];
    if ( nil == key_window ) {
        return;
    }
    casper::app::cef3::common::Helper::ZoomOut(casper::cef3::browser::RootWindow::GetForNSWindow(key_window)->GetBrowser());
}

- (IBAction)zoomReset:(id)sender
{
    NSWindow* key_window = [[NSApplication sharedApplication] keyWindow];
    if ( nil == key_window ) {
        return;
    }
    casper::app::cef3::common::Helper::ZoomReset(casper::cef3::browser::RootWindow::GetForNSWindow(key_window)->GetBrowser());
}

- (IBAction)showDeveloperTools:(id)sender {
    NSWindow* key_window = [[NSApplication sharedApplication] keyWindow];
    if ( nil == key_window ) {
        return;
    }
    casper::app::cef3::common::Helper::ShowDeveloperTools(casper::cef3::browser::RootWindow::GetForNSWindow(key_window)->GetBrowser());
}

- (IBAction)hideDeveloperTools:(id)sender {
    NSWindow* key_window = [[NSApplication sharedApplication] keyWindow];
    if ( nil == key_window ) {
        return;
    }
    casper::app::cef3::common::Helper::HideDeveloperTools (casper::cef3::browser::RootWindow::GetForNSWindow(key_window)->GetBrowser());
}

-(void)printPDFByURL:(NSString*)urlString direclty:(BOOL)direclty
{
    NSURL *url = [[NSURL alloc]initWithString:urlString];
    
    PDFDocument* document = [[PDFDocument alloc]initWithURL:url];
    NSPrintInfo* printInfo = [[NSPrintInfo alloc]init];
    
    
    NSPrintOperation* op = [document printOperationForPrintInfo:printInfo scalingMode:kPDFPrintPageScaleDownToFit autoRotate:NO];
    
    [op setShowsPrintPanel: NO == direclty];
    
    
    BOOL success = [op runOperation];
    
    NSLog(@"success = %@", success ? @"YES" : @"NO");
}

- (IBAction)printDemoPDFDirectly:(id)sender
{
    // TODO CW
    [self printPDFByURL:@"file:///tmp/sample.pdf" direclty: YES];
}

- (IBAction)printDemoPDF:(id)sender
{
    // TODO CW
    [self printPDFByURL:@"file:///tmp/sample.pdf" direclty: NO];
}


- (void)enableAccessibility:(bool)bEnable
{
    NSWindow* key_window = [[NSApplication sharedApplication] keyWindow];
    if ( nil == key_window ) {
        return;
    }
    
    scoped_refptr<casper::cef3::browser::RootWindow> root_window = casper::cef3::browser::RootWindow::GetForNSWindow(key_window);
    
    CefRefPtr<CefBrowser> browser = root_window->GetBrowser();
    if ( nullptr != browser.get() ) {
        browser->GetHost()->SetAccessibilityState(bEnable ? STATE_ENABLED : STATE_DISABLED);
    }
}

- (NSApplicationTerminateReply)applicationShouldTerminate:
(NSApplication*)sender {
    return NSTerminateNow;
}

@end

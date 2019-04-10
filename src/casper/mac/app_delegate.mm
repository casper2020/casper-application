#import "casper/mac/app_delegate.h"

#import <Quartz/Quartz.h> // PDFDocument

#import "Alerts.h"

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
        relaunch = NO;
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
    
    const BOOL appIsAgent = [[[NSBundle mainBundle] objectForInfoDictionaryKey:@"LSUIElement"] boolValue];
    if ( YES == appIsAgent ) {
        // ... create status bar menu ...
        NSStatusBar *statusBar = [NSStatusBar systemStatusBar];
        
        statusItem = [statusBar statusItemWithLength:NSSquareStatusItemLength];
        statusItem.image =  [NSImage imageNamed:@"StatusBar"];
        [statusItem.image setTemplate:YES];
        [statusItem retain];
        
        statusItem.menu = [[[NSMenu alloc]init]retain];
        
        if ( nil == statusMenuItem ) {
            statusMenuItem = [[NSMenuItem alloc] initWithTitle:@"Monitor" action:nil keyEquivalent:@""];
        }
        aboutMenuItem           = [[NSMenuItem alloc] initWithTitle:@"About"                action:@selector(about:) keyEquivalent:@""];
        
        showWindowMenuItem      = [[NSMenuItem alloc] initWithTitle:@"Show Window"          action:@selector(showWindow:) keyEquivalent:@"1"];
        [showWindowMenuItem setKeyEquivalentModifierMask: ( NSShiftKeyMask | NSCommandKeyMask )];
        
        preferencesMenuItem     = [[NSMenuItem alloc] initWithTitle:@"Preferences..."       action:@selector(showPreferences:) keyEquivalent:@","];
        consoleMenuItem         = [[NSMenuItem alloc] initWithTitle:@"Console..."           action:@selector(showConsole:) keyEquivalent:@"k"];
        quitMenuItem            = [[NSMenuItem alloc] initWithTitle:@"Quit"                 action:@selector(quit:) keyEquivalent:@"Q"];
        
        [statusItem.menu addItem:aboutMenuItem];
        [statusItem.menu addItem:[NSMenuItem separatorItem]];
        [statusItem.menu addItem:statusMenuItem];
        [statusItem.menu addItem:[NSMenuItem separatorItem]];
        [statusItem.menu addItem:showWindowMenuItem];
        [statusItem.menu addItem:[NSMenuItem separatorItem]];
        [statusItem.menu addItem:preferencesMenuItem];
        [statusItem.menu addItem:consoleMenuItem];
        [statusItem.menu addItem:[NSMenuItem separatorItem]];
        [statusItem.menu addItem:quitMenuItem];
    } else {
        [self showWindow:self];
    }
    
    updater = [[SUUpdater alloc]initForBundle:[NSBundle mainBundle]];
}

#pragma mark -

- (void)tryToTerminateApplication:(NSApplication*)app {
    casper::cef3::browser::MainContext::Get()->GetRootWindowManager()->CloseAllWindows(false);
}

- (void)orderFrontStandardAboutPanel:(id)sender {
    [NSApp activateIgnoringOtherApps:YES];
    [[NSApplication sharedApplication] orderFrontStandardAboutPanel: self];
}

- (IBAction)checkForUpdates:(id)sender
{
    [updater checkForUpdates:self];
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

- (void)showPreferences:(id)sender
{
    [NSApp activateIgnoringOtherApps:YES];
    
    if ( nil == preferencesWindowController ) {
        preferencesWindowController = [[PreferencesWindowController alloc]initWithSparkle: updater andWithListener: self];
    }
    
    [preferencesWindowController.window makeKeyAndOrderFront:nil];
}

- (void)showConsole:(id)sender
{
    [[NSWorkspace sharedWorkspace]launchApplication:@"/Applications/Utilities/Console.app"];
}

- (void)about:(id)sender
{
    [self orderFrontStandardAboutPanel: sender];
}

+ (NSImage*)tintImage:(NSImage*)image withColor:(NSColor*)color
{
    NSImage* newImage = [[NSImage imageNamed:@"StatusBar"] copy];
    
    [newImage lockFocus];
    
    [color set];
    
    NSRectFillUsingOperation(NSMakeRect(0, 0, image.size.width, image.size.height), NSCompositingOperationSourceAtop);
    
    [newImage unlockFocus];
    
    [newImage setTemplate:NO];

    return newImage;
}

- (void)setRunningProcesses:(const Json::Value&)list
{
    if ( nil == monitorMenuItems ) {
        monitorMenuItems = [[NSMutableDictionary alloc]init];
    }

    if ( nil == statusMenuItem ) {
        statusMenuItem = [[NSMenuItem alloc] initWithTitle:@"Monitor" action:nil keyEquivalent:@""];
    }

    if ( nil == statusMenuItem.submenu ) {
        statusMenuItem.submenu = [[NSMenu alloc]init];
    }
    
    if ( true == list.isNull() ) {
        statusItem.image = [AppDelegate tintImage:[NSImage imageNamed:@"StatusBar"] withColor:[NSColor redColor]];
    } else {
        
        Json::ArrayIndex cnt = 0;
        
        for ( Json::ArrayIndex idx = 0 ; idx < list.size() ; ++idx ) {
            
            const Json::Value& process = list[idx];

            const auto pid = process.get("pid", 0).asInt();
            if ( pid > 0 ) {
                cnt++;
            }

            NSString* key   = [NSString stringWithUTF8String: process["id"].asCString()];
            NSString* title = [NSString stringWithFormat: @"%@ ( %@ )",
                               [NSString stringWithUTF8String: process["id"].asCString()],
                               ( 0 != pid  ? [NSNumber numberWithUnsignedInteger: pid] : @"not running" )
            ];
            
            NSMenuItem* item = (NSMenuItem*)[monitorMenuItems objectForKey: key];
            if ( nil == item ) {
                item = [[NSMenuItem alloc]initWithTitle: title action: nil keyEquivalent: @""];
                [monitorMenuItems setObject: item forKey: key];
                [statusMenuItem.submenu addItem: item];
            } else {
                [item setTitle:title];
            }
            
        }
        
        if ( list.size() != cnt ) {
            if ( 0 == cnt ) {
                statusItem.image = [AppDelegate tintImage:[NSImage imageNamed:@"StatusBar"] withColor:[NSColor redColor]];
            } else {
                statusItem.image = [AppDelegate tintImage:[NSImage imageNamed:@"StatusBar"] withColor:[NSColor yellowColor]];
            }
        } else {
            statusItem.image = [NSImage imageNamed:@"StatusBar"];
        }
        
    }

}

- (void)showError:(const Json::Value&)error andRelaunch:(BOOL)relaunch
{
    const NSModalResponse r = [Alerts showCriticalMessage: @""
                                          informativeText: [NSString stringWithCString: error["msg"].asCString() encoding: NSUTF8StringEncoding]
                                               andButtons: @[@"Relaunch", @"Quit", @"Console"]
    ];
    if ( r == NSAlertThirdButtonReturn ) {
        [self showConsole: nil];
    } else {
        if ( r == NSAlertFirstButtonReturn ) {
            self->relaunch = YES;
        } else {
            self->relaunch = NO;
        }
        [self quit: nil];
    }
}

- (void)showException:(const std::exception&)exception delayFor:(NSTimeInterval)seconds andQuit:(BOOL)quit
{
    
    __block std::exception __exception = exception;
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, seconds * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
        const NSModalResponse r = [Alerts showCriticalMessage: @"An std::exception Occurred"
                                              informativeText: [NSString stringWithCString: __exception.what() encoding: NSUTF8StringEncoding]
                                                   andButtons: @[@"Ok"]
        ];
        if ( r == NSAlertFirstButtonReturn ) {
            if ( YES == quit ) {
                [self quit: nil];
            }
        }
    });
}

- (void)startProcess:(const Json::Value&)process notifyWhenStarted:(void(^)(pid_t))startedCallback andWhenFinished:(void(^)(int,Json::Value))finishedCallback
{
    
    __block const Json::Value config = process;
    
    dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0ul);
    dispatch_async(queue, ^{

        pid_t pid = 0;

        const Json::Value& arguments  = config["monitor"]["arguments"];
        
        NSString*       __launch_path = [NSString stringWithUTF8String: config["monitor"]["path"].asCString()];
        NSMutableArray* __arguments   = [[NSMutableArray alloc]init];

        @try {
            
            for ( Json::ArrayIndex idx = 0 ; idx < arguments.size() ; ++idx ) {
                [__arguments addObject:[NSString stringWithUTF8String: arguments[idx].asCString()]];
            }
            
            NSTask* task = [[NSTask alloc]init];
            task.launchPath = __launch_path;
            task.arguments  = __arguments;
            [task launch];
            
            pid = static_cast<pid_t>(task.processIdentifier);
            
            running_tasks_[pid] = task;
            
            startedCallback(pid);
            
            [task waitUntilExit];
            
            const auto it = running_tasks_.find(pid);
            if ( running_tasks_.end() != it ) {
                running_tasks_.erase(it);
            }
            
            finishedCallback(EXIT_SUCCESS, Json::Value::null);
            
        } @catch (NSException* a_exception) {

            const auto it = running_tasks_.find(pid);
            if ( running_tasks_.end() != it ) {
                [it->second terminate];
                running_tasks_.erase(it);
            }
            
            Json::Value error = Json::Value(Json::ValueType::objectValue);
            error["msg"]   = [[NSString stringWithFormat:@"An error occurred while starting process\n\n%@\n\n%@",
                               __launch_path, a_exception.reason
                               ] cStringUsingEncoding:NSUTF8StringEncoding];
            error["fatal"] = true;

            finishedCallback(EXIT_FAILURE, error);

        }
        
    });
}

- (void)stopProcess:(pid_t)pid notifyWhenFinished:(void(^)(int,Json::Value))finishedCallback
{
    __block const pid_t __pid = pid;
    
    dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0ul);
    dispatch_async(queue, ^{
        
        const auto it = running_tasks_.find(__pid);
        if ( running_tasks_.end() != it ) {
            
            [it->second terminate];
            running_tasks_.erase(it);
        }
        
        finishedCallback(EXIT_SUCCESS, Json::Value::null);
        
    });
}

- (BOOL)shouldRelaunch
{
    return ( YES == relaunch );
}

- (void)quit:(id)sender
{
    // .. close open window(s) ( if any ) ...
    casper::cef3::browser::MainContext::Get()->GetRootWindowManager()->CloseAllWindows(true);
    // ... and the main message loop ...
    casper::cef3::browser::MainMessageLoop::Get()->Quit();
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

#pragma mark -

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender {
    return NSTerminateNow;
}

#pragma mark -

- (void)showWindow:(id)sender
{
    [NSApp activateIgnoringOtherApps:YES];

    auto root_window_manager = casper::cef3::browser::MainContext::Get()->GetRootWindowManager();
    scoped_refptr<casper::cef3::browser::RootWindow> root_window = root_window_manager->GetActiveRootWindow();
    if ( nullptr != root_window ) {
        root_window->Show(casper::cef3::browser::RootWindow::ShowMode::ShowNormal);
    } else {
        // Create the first window.
        casper::cef3::browser::RootWindowConfig window_config;
        window_config.with_controls = with_controls_;
        window_config.with_osr      = with_osr_;
        
        window_config.bounds.Set(0, 0, 1024, 768);
        root_window_manager->CreateRootWindow(window_config);
    }
}

#pragma mark - PreferencesWindowListener

- (void)onSettingsChangedRelaunchRequired:(BOOL)relaunch now:(BOOL)now
{
    self->relaunch = relaunch;
    if ( NO == self->relaunch ) {
        return;
    }
    const NSModalResponse r = [Alerts showWarningMessage: @"Settings Changed"
                                         informativeText: (YES == now
                                                            ?
                                                                @"In order to apply new settings, this application will restart."
                                                            :
                                                                @"In order to apply new settings, this application needs to restart.\nRestart now?"
                                                           )
                                              andButtons: ( YES == now
                                                            ?
                                                                @[@"Restart"]
                                                            :
                                                                @[@"Restart", @"Cancel"]
                                                           )
                               ];
    if ( NSAlertFirstButtonReturn == r ) {
        [PreferencesWindowController setConfigured: YES];
        [self quit: nil];
    } else {
        self->relaunch = NO;
    }
}

-(void)onSettingsWindowDidClose
{
    preferencesWindowController = nil;
}

@end

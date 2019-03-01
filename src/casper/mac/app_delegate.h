
#import <Cocoa/Cocoa.h>

#import <Sparkle/Sparkle.h> // SUUpdater

#import "PreferencesWindowController.h"

#include "json/json.h"

// Receives notifications from the application. Will delete itself when done.
@interface AppDelegate : NSObject<NSApplicationDelegate, PreferencesWindowListener> {
    @private
    bool with_controls_;
    bool with_osr_;
    NSStatusItem *statusItem;
    NSMenuItem   *statusMenuItem;
    NSMenuItem   *aboutMenuItem;
    NSMenuItem   *showWindowMenuItem;
    NSMenuItem   *preferencesMenuItem;
    NSMenuItem   *consoleMenuItem;
    NSMenuItem   *quitMenuItem;
    
    NSMutableDictionary* monitorMenuItems;
    
    @private
    
    SUUpdater    *updater;
    BOOL         relaunch;
    
    @private
    
    PreferencesWindowController* preferencesWindowController;
    
    std::map<pid_t, NSTask*> running_tasks_;
    
}
    
- (id)initWithControls:(bool)with_controls andOsr:(bool)with_osr;
- (void)createApplication:(id)object;
- (void)tryToTerminateApplication:(NSApplication*)app;
- (void)enableAccessibility:(bool)bEnable;
    
- (IBAction)checkForUpdates:(id)sender;
    
- (IBAction)reload:(id)sender;
- (IBAction)zoomIn:(id)sender;
- (IBAction)zoomOut:(id)sender;
- (IBAction)zoomReset:(id)sender;
- (IBAction)showDeveloperTools:(id)sender;
- (IBAction)hideDeveloperTools:(id)sender;
- (IBAction)printDemoPDF:(id)sender;
- (IBAction)printDemoPDFDirectly:(id)sender;

- (void)about:(id)sender;
- (void)showPreferences:(id)sender;
- (void)setRunningProcesses:(const Json::Value&)list;
- (void)showError:(const Json::Value&)error andRelaunch:(BOOL)relaunch;
- (void)showException:(const std::exception&)exception delayFor:(NSTimeInterval)seconds andQuit:(BOOL)quit;
- (void)startProcess:(const Json::Value&)process notifyWhenStarted:(void(^)(pid_t))startedCallback andWhenFinished:(void(^)(int,Json::Value))finishedCallback;
- (void)stopProcess:(pid_t)pid notifyWhenFinished:(void(^)(int,Json::Value))finishedCallback;
- (BOOL)shouldRelaunch;
- (void)quit:(id)sender;
    
@end


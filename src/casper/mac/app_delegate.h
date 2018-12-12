
#import <Cocoa/Cocoa.h>

// Receives notifications from the application. Will delete itself when done.
@interface AppDelegate : NSObject<NSApplicationDelegate> {
@private
    bool with_controls_;
    bool with_osr_;
}

- (id)initWithControls:(bool)with_controls andOsr:(bool)with_osr;
- (void)createApplication:(id)object;
- (void)tryToTerminateApplication:(NSApplication*)app;
- (void)enableAccessibility:(bool)bEnable;

- (IBAction)reload:(id)sender;
- (IBAction)zoomIn:(id)sender;
- (IBAction)zoomOut:(id)sender;
- (IBAction)zoomReset:(id)sender;
- (IBAction)showDeveloperTools:(id)sender;
- (IBAction)hideDeveloperTools:(id)sender;
- (IBAction)printDemoPDF:(id)sender;
- (IBAction)printDemoPDFDirectly:(id)sender;

@end

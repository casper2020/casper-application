//
//  Alerts.m
//  casper
//
//  Created by Américo Gomes on 28/02/2019.
//

#import "Alerts.h"

@implementation Alerts

+(NSModalResponse)showInformationalMessage:(NSString*)message informativeText:(NSString*)text andButtons:(NSArray*)buttons
{
    return [Alerts showAlertStyle: NSAlertStyleInformational withMessage:message informativeText:text andButtons:buttons];
}

+(NSModalResponse)showWarningMessage:(NSString*)message informativeText:(NSString*)text andButtons:(NSArray*)buttons
{
    return [Alerts showAlertStyle: NSAlertStyleWarning withMessage:message informativeText:text andButtons:buttons];
}

+(NSModalResponse)showCriticalMessage:(NSString*)message informativeText:(NSString*)text andButtons:(NSArray*)buttons
{
    return [Alerts showAlertStyle: NSAlertStyleCritical withMessage:message informativeText:text andButtons:buttons];
}

+(NSModalResponse)showAlertStyle:(NSAlertStyle)style withMessage:(NSString*)message informativeText:(NSString*)text andButtons:(NSArray*)buttons
{
    NSAlert* alert = [[NSAlert alloc]init];
    [alert setAlertStyle: style];
    [alert setMessageText:message];
    [alert setInformativeText:text];
    
    for ( id button in buttons ) {
        [alert addButtonWithTitle: (NSString*)button];
    }
    
    return [alert runModal];
}

@end

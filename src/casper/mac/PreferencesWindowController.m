/**
 * @file PreferencesWindowController.m
 *
 * Copyright (c) 2011-2018 Cloudware S.A. All rights reserved.
 *
 * This file is part of casper.
 *
 * casper is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * casper is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with osal.  If not, see <http://www.gnu.org/licenses/>.
 */
#import "PreferencesWindowController.h"

@implementation PreferencesWindowController

- (instancetype)init
{
    self = [self initWithWindowNibName:@"PreferencesWindow"];
    if ( nil != self ) {
        [self.window setTitle:@"Preferences"];
        [self.window setDelegate:self];
        [self bindURLSetting: &configDirectoryPrefixSetting   withField: configDirectoryPrefixTextField
             andSettingValue: [PreferencesWindowController configDirectoryPrefixURL]
        ];
        [self bindURLSetting: &runtimeDirectoryPrefixSetting  withField: runtimeDirectoryPrefixTextField
             andSettingValue: [PreferencesWindowController runtimeDirectoryPrefixURL]
        ];
        [self bindURLSetting: &postgresqlDataDirectorySetting withField: postgreSQLDataDirectoryTextField
             andSettingValue:[PreferencesWindowController postgreSQLDataDirectoryURL]
        ];
        [self bindStringSetting: &postgreSQLArgumentsSetting withField: postgreSQLArgumentsTextField
                andSettingValue: [PreferencesWindowController postgreSQLArguments]
        ];
        [postgreSQLArgumentsTextField setDelegate:self];
        [self bindControlSetting: &automaticallyCheckForUpdatesSetting withControl: automaticallyCheckForUpdatesButton
                 andSettingValue: NSControlStateValueOff
        ];
        [self bindControlSetting: &automaticallyDownloadUpdatesSetting withControl: automaticallyDownloadUpdatesButton
                 andSettingValue: NSControlStateValueOff
        ];
    }
    return self;
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
    if ( self.window == notification.object ) {
        [[NSApplication sharedApplication] setActivationPolicy:NSApplicationActivationPolicyRegular];
    }
}

- (void)windowDidBecomeMain:(NSNotification *)notification
{
    if ( self.window == notification.object ) {
        [[NSApplication sharedApplication] setActivationPolicy:NSApplicationActivationPolicyRegular];
    }
}

- (void)windowWillClose:(NSNotification *)notification
{
    if ( self.window == notification.object ) {
        const int windowCount = [[[NSApplication sharedApplication] windows] count];
        // TODO CW 2 ==
        if ( 2 == ( windowCount - 1 ) ) {
            [[NSApplication sharedApplication] setActivationPolicy:NSApplicationActivationPolicyAccessory];
        }
    }
}

- (instancetype)initWithSparkle:(SUUpdater*)updater
{
    self = [self init];
    if ( nil != self ) {
        self->updater = updater;
        [self bindControlSetting: &automaticallyCheckForUpdatesSetting withControl: automaticallyCheckForUpdatesButton
                 andSettingValue: ( YES == updater.automaticallyChecksForUpdates ? NSControlStateValueOn : NSControlStateValueOff )
        ];
        [self bindControlSetting: &automaticallyDownloadUpdatesSetting withControl: automaticallyDownloadUpdatesButton
                 andSettingValue: ( YES == updater.automaticallyDownloadsUpdates ? NSControlStateValueOn : NSControlStateValueOff )
        ];
    }
    return self;
}

- (IBAction)showGeneralPreferences:(id)sender
{
    return;
}

- (IBAction)changeRuntimeDirectoryPrefix:(id)sender
{
    [self changeDirectory:^(const NSURL *url) {
        [self setURLSetting: &runtimeDirectoryPrefixSetting withValue:url];
    }];
}
    
- (IBAction)changeConfigDirectoryPrefix:(id)sender
{
    [self changeDirectory:^(const NSURL *url) {
        [self setURLSetting: &configDirectoryPrefixSetting withValue:url];
    }];
}

- (IBAction)changePostgreSQLDataDirectory:(id)sender
{
    [self changeDirectory:^(const NSURL *url) {
        [self setURLSetting: &postgresqlDataDirectorySetting withValue:url];
    }];
}

- (IBAction)reset:(id)sender
{
    [configDirectoryPrefixTextField setStringValue:@""];
    [runtimeDirectoryPrefixTextField setStringValue:@""];
    [postgreSQLDataDirectoryTextField setStringValue:@""];
    [postgreSQLArgumentsTextField setStringValue:@""];
    
    [self setURLSetting:     &configDirectoryPrefixSetting   withValue: nil];
    [self setURLSetting:     &runtimeDirectoryPrefixSetting  withValue: nil];
    [self setURLSetting:     &postgresqlDataDirectorySetting withValue: nil];
    [self setStringSetting:  &postgreSQLArgumentsSetting     withValue: nil];

    [automaticallyCheckForUpdatesButton setState:NSControlStateValueOn];
    [automaticallyDownloadUpdatesButton setState:NSControlStateValueOff];

    [self setControlSetting: &automaticallyCheckForUpdatesSetting withValue: NSControlStateValueOn];
    [self setControlSetting: &automaticallyDownloadUpdatesSetting withValue: NSControlStateValueOff];
}

- (IBAction)cancel:(id)sender
{
    [self close];
}

- (IBAction)apply:(id)sender
{
    if ( YES == runtimeDirectoryPrefixSetting.set ) {
        if ( nil != runtimeDirectoryPrefixSetting.value ) {
            [PreferencesWindowController setRuntimeDirectoryPrefixURL:runtimeDirectoryPrefixSetting.value.path];
        } else {
            [PreferencesWindowController setRuntimeDirectoryPrefixURL: @""];
        }
    }
    if ( YES == configDirectoryPrefixSetting.set ) {
        if ( nil != configDirectoryPrefixSetting.value ) {
            [PreferencesWindowController setConfigDirectoryPrefixURL:configDirectoryPrefixSetting.value.path];
        } else {
            [PreferencesWindowController setConfigDirectoryPrefixURL: @""];
        }
    }
    if ( YES == postgresqlDataDirectorySetting.set ) {
        if ( nil != runtimeDirectoryPrefixSetting.value ) {
            [PreferencesWindowController setPostgreSQLDataDirectoryURL:postgresqlDataDirectorySetting.value.path];
        } else {
            [PreferencesWindowController setPostgreSQLDataDirectoryURL: @""];
        }
    }
    if ( YES == postgreSQLArgumentsSetting.set ) {
        if ( nil != postgreSQLArgumentsSetting.value ) {
            [PreferencesWindowController setPostgreSQLArguments: postgreSQLArgumentsSetting.value];
        } else {
            [PreferencesWindowController setPostgreSQLArguments: @""];
        }
    }
    if ( YES == automaticallyCheckForUpdatesSetting.set ) {
        self->updater.automaticallyChecksForUpdates = automaticallyCheckForUpdatesSetting.value;
    }
    
    if ( YES == automaticallyDownloadUpdatesSetting.set ) {
        self->updater.automaticallyDownloadsUpdates = automaticallyDownloadUpdatesSetting.value;
    }
    [self close];
}

+ (void)setRuntimeDirectoryPrefixURL:(NSString*)url
{
    [[NSUserDefaults standardUserDefaults]setObject:url forKey: @"runtimeDirectoryPrefixURL"];
}

+ (NSString*)runtimeDirectoryPrefixURL
{
     return [[NSUserDefaults standardUserDefaults]objectForKey:@"runtimeDirectoryPrefixURL"];
}

+ (void)setConfigDirectoryPrefixURL:(NSString*)url
{
    [[NSUserDefaults standardUserDefaults]setObject:url forKey: @"configDirectoryPrefixURL"];
}

+ (NSString*)configDirectoryPrefixURL
{
    return [[NSUserDefaults standardUserDefaults]objectForKey:@"configDirectoryPrefixURL"];
}

+ (void)setPostgreSQLDataDirectoryURL:(NSString*)url
{
    [[NSUserDefaults standardUserDefaults]setObject:url forKey: @"postgreSQLDataDirectoryURL"];
}

+ (NSString*)postgreSQLDataDirectoryURL
{
    return [[NSUserDefaults standardUserDefaults]objectForKey:@"postgreSQLDataDirectoryURL"];
}

+ (void)setPostgreSQLArguments:(NSString*)arguments
{
    [[NSUserDefaults standardUserDefaults]setObject:arguments forKey: @"postgreSQLArguments"];
}

+ (NSString*)postgreSQLArguments
{
    return [[NSUserDefaults standardUserDefaults]objectForKey:@"postgreSQLArguments"];
}

#pragma mark - PRIVATE

- (void)controlTextDidChange:(NSNotification *)notification
{
    NSTextField* field = [notification object];
    if ( field == postgreSQLArgumentsTextField ) {
        [self setStringSetting:&postgreSQLArgumentsSetting withValue: [field stringValue]];
    }
    return;
}

#pragma mark - PRIVATE
    
- (void)changeDirectory:(void (^)(const NSURL* url))handler
{
    NSOpenPanel* panel = [NSOpenPanel openPanel];
    
    [panel setCanChooseFiles:NO];
    
    [panel setCanChooseDirectories:YES];
    [panel setCanCreateDirectories:NO];
    
    [panel setAllowsMultipleSelection:NO];
    
    [panel beginSheetModalForWindow:self.window completionHandler:^(NSInteger result){
        if (result == NSFileHandlingPanelOKButton) {
            NSArray* urls = [panel URLs];
            NSURL* url = [urls objectAtIndex:0];
            if ( YES == url.isFileURL  ) {
                BOOL isDirectory = NO;
                if ( [[NSFileManager defaultManager] fileExistsAtPath: url.path isDirectory: &isDirectory] && YES == isDirectory ) {
                    handler(url);
                }
            }
        }
    }];
}

- (void)setURLSetting:(struct URLSetting*)setting withValue:(const NSURL*)value
{
    setting->set   = YES;
    setting->value = ( nil != value ? [[NSURL alloc]initWithString:[value path]] : nil );
    if ( nil != setting->value ) {
        [setting->field setStringValue: setting->value.path];
    } else {
        [setting->field setStringValue: @""];
    }
}

- (void)bindURLSetting:(struct URLSetting*)setting withField:(NSTextField*)field andSettingValue:(NSString*)path
{
    setting->field = field;
    setting->set   = NO;
    setting->value = ( nil != path ? [[NSURL alloc]initWithString:path] : nil );
    if ( nil != setting->value && nil != setting->value.path ) {
        [setting->field setStringValue: setting->value.path];
    } else {
        [setting->field setStringValue: @""];
    }
}

- (void)setStringSetting:(struct StringSetting*)setting withValue:(const NSString*)value
{
    setting->set   = YES;
    setting->value = ( nil != value ? value.mutableCopy : nil );
    if ( nil != setting->value ) {
        [setting->field setStringValue: setting->value];
    } else {
        [setting->field setStringValue: @""];
    }
}

- (void)bindStringSetting:(struct StringSetting*)setting withField:(NSTextField*)field andSettingValue:(const NSString*)value
{
    setting->field = field;
    setting->set   = NO;
    setting->value = ( nil != value ? value.mutableCopy : nil );
    if ( nil != setting->value ) {
        [setting->field setStringValue: setting->value];
    } else {
        [setting->field setStringValue: @""];
    }
}

- (void)setControlSetting:(struct ControlStateSetting*)setting withValue:(const NSControlStateValue)value
{
    setting->set   = YES;
    setting->value = value;
    if ( nil != setting->control ) {
        [setting->control setState: value];
    }
}

- (void)bindControlSetting:(struct ControlStateSetting*)setting withControl:(NSButton*)control andSettingValue:(const NSControlStateValue)value
{
    setting->control = control;
    setting->set     = NO;
    setting->value   = value;
    if ( nil != setting->control ) {
        [setting->control setState: value];
    }
}

@end

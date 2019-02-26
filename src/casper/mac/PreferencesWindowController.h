/**
 * @file PreferencesWindowController.h
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

#import <Cocoa/Cocoa.h>

#import <Sparkle/Sparkle.h>

@protocol PreferencesWindowListener <NSObject>

-(void)onSettingsChangedRelaunchRequired:(BOOL)relaunch;

@end

@interface PreferencesWindowController : NSWindowController<NSWindowDelegate, NSTextFieldDelegate> {

    IBOutlet NSToolbar*     toolbar;
    IBOutlet NSToolbarItem* generalToolbarItem;
    IBOutlet NSTextField*   configDirectoryPrefixTextField;
    IBOutlet NSTextField*   runtimeDirectoryPrefixTextField;
    IBOutlet NSTextField*   postgreSQLDataDirectoryTextField;
    IBOutlet NSTextField*   postgreSQLArgumentsTextField;

    IBOutlet NSButton*      automaticallyCheckForUpdatesButton;
    IBOutlet NSButton*      automaticallyDownloadUpdatesButton;
    IBOutlet NSButton*      checkForUpdatesButton;
    IBOutlet NSButton*      resetButton;
    IBOutlet NSButton*      cancelButton;
    IBOutlet NSButton*      applyButton;
    
    @private
    
    struct URLSetting {
        NSTextField* field;
        NSURL*       value;
        BOOL         set;
        BOOL         changed;
    } ;

    struct StringSetting {
        NSTextField* field;
        NSString*    value;
        BOOL         set;
        BOOL         changed;
    } ;

    struct ControlStateSetting {
        NSButton*           control;
        NSControlStateValue value;
        BOOL                set;
        BOOL                changed;
    } ;

    struct URLSetting          configDirectoryPrefixSetting;
    struct URLSetting          runtimeDirectoryPrefixSetting;
    struct URLSetting          postgresqlDataDirectorySetting;
    struct StringSetting       postgreSQLArgumentsSetting;
    SUUpdater*                 updater;
    struct ControlStateSetting automaticallyCheckForUpdatesSetting;
    struct ControlStateSetting automaticallyDownloadUpdatesSetting;
    
    @private
    
    BOOL                          settingsDidChange;
    BOOL                          settingsApplied;
    id<PreferencesWindowListener> listener;
    
}

- (instancetype)initWithSparkle:(SUUpdater*)updater andWithListener:(id<PreferencesWindowListener>)listener;

- (IBAction)showGeneralPreferences:(id)sender;

- (IBAction)reset:(id)sender;
- (IBAction)cancel:(id)sender;
- (IBAction)apply:(id)sender;

 + (void)setRuntimeDirectoryPrefixURL:(NSString*)url;
 + (NSString*)runtimeDirectoryPrefixURL;

+ (void)setConfigDirectoryPrefixURL:(NSString*)url;
+ (NSString*)configDirectoryPrefixURL;

+ (void)setPostgreSQLDataDirectoryURL:(NSString*)url;
+ (NSString*)postgreSQLDataDirectoryURL;

+ (void)setPostgreSQLArguments:(NSString*)arguments;
+ (NSString*)postgreSQLArguments;

@end

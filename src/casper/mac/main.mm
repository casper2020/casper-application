/**
 * @file main.mm
 *
 * Copyright (c) 2011-2018 Cloudware S.A. All rights reserved.
 *
 * This file is part of casper-app.
 *
 * casper-app is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * casper-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with casper.  If not, see <http://www.gnu.org/licenses/>.
 */

#import <Cocoa/Cocoa.h>

#import "casper/mac/app_delegate.h"

#include "include/cef_app.h"
#include "include/wrapper/cef_library_loader.h"

#include "cef3/common/client/switches.h"
#include "cef3/browser/root_window.h"

#include "cef3/client/browser/client_app_browser.h"

#include "cef3/browser/main_message_loop_external_pump.h"
#include "cef3/browser/main_message_loop_std.h"

#include "cef3/common/client/switches.h"

#include "cef3/common/main.h"

#import "cef3/client/mac/application.h"
#import "cef3/client/mac/main_context.h"

#include <stdarg.h> // va_list, va_start, va_end

static void NormalizePath (const char* const a_path, std::string& o_path)
{
    o_path = a_path;
    std::replace(o_path.begin(), o_path.end(), ' ', '_');
    if ( '/' != o_path[o_path.length()-1] ) {
        o_path += '/';
    }
    std::size_t pos = o_path.find("file://");
    if ( std::string::npos != pos ) {
        o_path.erase(pos, sizeof(char)*7);
    }
}

static bool GetPath (NSSearchPathDirectory a_path, const std::vector<std::string>& a_append, std::string& o_path)
{
    NSArray<NSURL*>* urls = [[NSFileManager defaultManager]URLsForDirectory:a_path inDomains:NSUserDomainMask];
    if ( nil == urls || 0 == [urls count] || nil == [urls firstObject] ) {
        return false;
    }
    const NSString* url = [[urls firstObject] absoluteString];
    if ( 0 == [url length] ) {
        return false;
    }
    NormalizePath([url cStringUsingEncoding:NSUTF8StringEncoding], o_path);
    for ( auto path : a_append ) {
        o_path += path + '/';
    }
    NSError*   error   = nil;
    const BOOL success = ( YES == [[NSFileManager defaultManager]createDirectoryAtPath:[NSString stringWithUTF8String: o_path.c_str()]
                                                           withIntermediateDirectories:YES
                                                                            attributes:@{}
                                                                                 error: &error]
    );
    if ( NO == success ) {
        NSLog(@"Unable to create directory: %@", error);
    }
    return ( YES == success );
}

/**
 * @brief casper-app entry point
 *
 * @param a_argc
 * @parma a_arvg
 */
int main (int a_argc, char* a_argv[])
{
    
    //... load the CEF framework library at runtime instead of linking directly ...
    // ... as required by the macOS sandbox implementation ...
    CefScopedLibraryLoader library_loader;
    if ( false == library_loader.LoadInMain() ) {
        return 1;
    }
    
    const CefMainArgs main_args(a_argc, a_argv);
    
    //... initialize the AutoRelease pool ...
    NSAutoreleasePool* autopool = [[NSAutoreleasePool alloc] init];
    
    // ... initialize the application instance ...
    [Application sharedApplication];
    
    // ... parse command-line arguments ...
    CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
    command_line->InitFromArgv(a_argc, a_argv);
    
    // Create a ClientApp of the correct type.
    CefRefPtr<CefApp> app;
    
    const ::casper::cef3::common::Main::ProcessType process_type = ::casper::cef3::common::Main::GetProcessType(command_line);
    if ( ::casper::cef3::common::Main::ProcessType::Browser == process_type ) {
        app = new casper::cef3::client::browser::ClientAppBrowser();
    }
    
    const NSBundle*      bundle            = [NSBundle mainBundle];
    const NSString*      bundleIdenfitifer = (NSString*)[bundle objectForInfoDictionaryKey:@"CFBundleIdentifier"];
    const NSString*      productVersion    = (NSString*)[bundle objectForInfoDictionaryKey:@"CFBundleShortVersionString"];
    const NSString*      productName       = (NSString*)[bundle objectForInfoDictionaryKey:@"CFBundleName"];
    const NSProcessInfo* processInfo       = [NSProcessInfo processInfo];
    const NSString*      userAgent         = [[NSString alloc]initWithFormat: @"%@ / %@ ( macOS %@", productName, productVersion, [processInfo operatingSystemVersionString]];
    const NSString*      initialURL        = (NSString*)[bundle objectForInfoDictionaryKey:@"CASPERInitialURL"];
    
    casper::cef3::browser::Settings context_settings = {
        /* os_          */ {
            /* name_    */ "macOS",
            /* version_ */ [[processInfo operatingSystemVersionString] cStringUsingEncoding:NSUTF8StringEncoding]
        },
        /* product_     */ {
            /* name_    */ [productName cStringUsingEncoding:NSUTF8StringEncoding],
            /* version_ */ [productVersion cStringUsingEncoding:NSUTF8StringEncoding]
        },
        /* application_ */ {
          /* main_url_                          */ [initialURL cStringUsingEncoding:NSUTF8StringEncoding],
          /* user_agent_                        */ [userAgent cStringUsingEncoding:NSUTF8StringEncoding],
          /* window_      */ {
              /* background_color_         */ 0, //  default color
              /* use_views_                */ false,
              /* use_windowless_rendering_ */ false,
              /* minimum_size_             */ { 1024, 768 },
              /* allow_title_changes_      */ false
           },
          /* terminate_when_all_windows_closed_ */ true
        },
        /* paths_ */ {
            /* app_working_directory_ */ "",
            /* logs_path_             */ "",
            /* downloads_path_        */ "",
            /* cache_path_            */ ""
        }
    };
    
    if ( false == GetPath(NSApplicationDirectory, {}, context_settings.paths_.app_working_directory_) ) {
        return 1;
    }
    if ( false == GetPath(NSCachesDirectory, { [bundleIdenfitifer cStringUsingEncoding:NSUTF8StringEncoding] }, context_settings.paths_.cache_path_) ) {
        return 1;
    }
    if ( false == GetPath(NSDownloadsDirectory, {}, context_settings.paths_.downloads_path_) ) {
        return 1;
    }
    if ( false == GetPath(NSCachesDirectory, { [bundleIdenfitifer cStringUsingEncoding:NSUTF8StringEncoding], "logs"}, context_settings.paths_.logs_path_) ) {
        return 1;
    }
    
    // ... reate the main context object ...
    scoped_ptr<casper::cef3::client::mac::MainContext> context(new casper::cef3::client::mac::MainContext(context_settings, command_line));
    
    CefSettings settings;
            
    // ... populate the settings based on command line arguments ...
    context->PopulateSettings(&settings);
    
    // ... create the main message loop object ...
    scoped_ptr<casper::cef3::browser::MainMessageLoop> message_loop;
    if ( settings.external_message_pump ) {
        message_loop = casper::cef3::browser::MainMessageLoopExternalPump::Factory();
    } else {
        message_loop.reset(new casper::cef3::browser::MainMessageLoopStd());
    }
    
    // ... initialize CEF ...
    context->Initialize(main_args, settings, app, NULL);
        
    // ... create the application delegate and window ...
    AppDelegate* delegate = [[AppDelegate alloc]
                             initWithControls:!command_line->HasSwitch(casper::cef3::common::client::switches::kHideControls)
                             andOsr:settings.windowless_rendering_enabled ? true : false];
    [delegate performSelectorOnMainThread:@selector(createApplication:)
                               withObject:nil
                            waitUntilDone:NO];
    
    // ... run the message loop...
    // ... this will block until Quit() is called ...
    const int result = message_loop->Run();
    
    // ... shut down CEF ...
    context->Shutdown();
    
    // ... release objects ( in reverse order of creation ) ...
    [delegate release];
    message_loop.reset();
    context.reset();
    [autopool release];
    
    return result;
}


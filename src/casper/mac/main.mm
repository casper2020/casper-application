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

#include "casper/app/version.h"
#include "casper/app/monitor/monitor.h"

#include "casper/app/logger.h"

#include "osal/osalite.h"

#define READ_LINK(a_link, a_fallback)[&] () { \
    char tmp[PATH_MAX];  tmp[0] = 0; \
    const size_t len = ( sizeof(tmp) / sizeof(tmp[0]) ); \
    const int count  = readlink(a_link.c_str(), tmp, len); \
    if ( -1 == count ) { \
        const int err_no = errno;\
        if ( 22 == err_no ) { \
            strncpy(tmp, a_link.c_str(), len); \
        } else { \
            if ( true == a_fallback ) { \
                return a_link; \
            } else { \
                CASPER_APP_LOG("error", "readlink: %3d - %s", err_no, strerror(err_no)); \
            } \
            return std::string(""); \
        } \
    } \
    return std::string(tmp); \
} () \

#define REAL_PATH(a_path, a_fallback)[&] () { \
    char tmp[PATH_MAX];  tmp[0] = 0; \
    const char* const rp = realpath(a_path.c_str(), tmp); \
    if ( nullptr == rp ) { \
        if ( true == a_fallback ) { \
            return a_path; \
        } else { \
            CASPER_APP_LOG("error", "realpath: %3d - %s", errno, strerror(errno)); \
        } \
        return std::string(""); \
    } \
    return std::string(rp); \
} ()\

#define NORMALIZE_PATH(a_path)[&]() { \
    std::string path = REAL_PATH(READ_LINK(a_path, true), true); \
    if ( path.length() > 0 && path.c_str()[path.length()-1] != '/' ) { \
        path += '/'; \
    } \
    return path; \
}()

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

static int ShowFatalException (const std::exception& a_std_exception)
{
    NSAlert* alert = [[NSAlert alloc]init];
    [alert setAlertStyle:NSAlertStyleCritical];
    [alert setMessageText:[NSString stringWithCString:a_std_exception.what() encoding:NSUTF8StringEncoding]];
    [alert addButtonWithTitle:@"OK"];
    [alert runModal];
    
    return 1;
}

/**
 * @brief casper-app entry point
 *
 * @param a_argc
 * @parma a_arvg
 */
int main (int a_argc, char* a_argv[])
{
    osal::debug::Trace::GetInstance().Register("status", stdout);

    osal::debug::Trace::GetInstance().Log("status", "\n* %s - starting up %s process w/pid %u...\n",
                                          CASPER_INFO, "main", getpid()
    );
#if defined(NDEBUG) && !( defined(DEBUG) || defined(_DEBUG) || defined(ENABLE_DEBUG) )
    osal::debug::Trace::GetInstance().Log("status", "\t- %-12s: %s\n", "Target", "release");
#else
    osal::debug::Trace::GetInstance().Log("status", "\t- %-12s: %s\n", "Target", "debug");
    osal::debug::Trace::GetInstance().Log("status", "\t- %-12s: %s\n", "SIZET_FMT" , SIZET_FMT);
    osal::debug::Trace::GetInstance().Log("status", "\t- %-12s: %s\n", "INT8_FMT"  , INT8_FMT);
    osal::debug::Trace::GetInstance().Log("status", "\t- %-12s: %s\n", "UINT8_FMT" , UINT8_FMT);
    osal::debug::Trace::GetInstance().Log("status", "\t- %-12s: %s\n", "INT16_FMT" , INT16_FMT);
    osal::debug::Trace::GetInstance().Log("status", "\t- %-12s: %s\n", "UINT16_FMT", UINT16_FMT);
    osal::debug::Trace::GetInstance().Log("status", "\t- %-12s: %s\n", "INT32_FMT" , INT32_FMT);
    osal::debug::Trace::GetInstance().Log("status", "\t- %-12s: %s\n", "UINT32_FMT", UINT32_FMT);
    osal::debug::Trace::GetInstance().Log("status", "\t- %-12s: %s\n", "INT64_FMT" , INT64_FMT);
    osal::debug::Trace::GetInstance().Log("status", "\t- %-12s: %s\n", "UINT64_FMT", UINT64_FMT);
#endif
    osal::debug::Trace::GetInstance().Log("status", "* %s - %s process w/pid %u configured...\n",
                                          CASPER_INFO, "main", getpid()
    );

    //... load the CEF framework library at runtime instead of linking directly ...
    // ... as required by the macOS sandbox implementation ...
    CefScopedLibraryLoader library_loader;
    if ( false == library_loader.LoadInMain() ) {
        CASPER_APP_LOG("status",
                       "%s", "Unable to load CEF framework library!"
        );
        return 1;
    }
    
    const NSBundle*      bundle            = [NSBundle mainBundle];
    const NSString*      bundleIdenfitifer = (NSString*)[bundle objectForInfoDictionaryKey:@"CFBundleIdentifier"];
    const NSString*      productVersion    = (NSString*)[bundle objectForInfoDictionaryKey:@"CFBundleShortVersionString"];
    const NSString*      productName       = (NSString*)[bundle objectForInfoDictionaryKey:@"CFBundleName"];
    const NSProcessInfo* processInfo       = [NSProcessInfo processInfo];
    const NSString*      userAgent         = [[NSString alloc]initWithFormat: @"%@ / %@ ( macOS %@", productName, productVersion, [processInfo operatingSystemVersionString]];
    const NSString*      initialURL        = (NSString*)[bundle objectForInfoDictionaryKey:@"CASPERInitialURL"];
    const NSString*      cacheDirectory    = [((NSURL*)[[NSFileManager defaultManager] URLForDirectory:NSCachesDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:NO error:nil]) path] ;
    
    const NSString*      userDirectory     = NSHomeDirectory();
    
    const BOOL           appIsAgent        = [[bundle objectForInfoDictionaryKey:@"LSUIElement"] boolValue];
    
    const std::string logs_dir = NORMALIZE_PATH((
                                                std::string([userDirectory cStringUsingEncoding:NSUTF8StringEncoding])
                                                    +
                                                std::string("/Library/Logs/") + [productName cStringUsingEncoding:NSUTF8StringEncoding]
                                               ));
    
    // TODO CW
    [[NSFileManager defaultManager]createDirectoryAtPath:[NSString stringWithCString:logs_dir.c_str() encoding:NSUTF8StringEncoding]
                             withIntermediateDirectories:YES attributes:nil error:nil];
    
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
          /* terminate_when_all_windows_closed_ */ ( NO == appIsAgent )
        },
        /* paths_ */ {
            /* app_working_directory_ */ "",
            /* logs_path_             */ logs_dir,
            /* downloads_path_        */ "",
            /* cache_path_            */ ""
        }
    };

    // ... startup logs ...
    try {
        
        casper::app::Logger::GetInstance().Startup(logs_dir, CASPER_INFO);
        
    } catch (const std::exception* a_std_exception) {
        return ShowFatalException(*a_std_exception);
    } catch (const std::exception& a_std_exception) {
        return ShowFatalException(a_std_exception);
    }
    
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
    
    CASPER_APP_DEBUG_LOG("status", "%s", "CEF creating main context...");
    
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
    CASPER_APP_DEBUG_LOG("status", "%s", "CEF initializing...");
    
    context->Initialize(main_args, settings, app, NULL);
        
    // ... create the application delegate and window ...
    AppDelegate* delegate = [[AppDelegate alloc]
                             initWithControls:!command_line->HasSwitch(casper::cef3::common::client::switches::kHideControls)
                             andOsr:settings.windowless_rendering_enabled ? true : false];
    [delegate performSelectorOnMainThread:@selector(createApplication:)
                               withObject:nil
                            waitUntilDone:NO];
    
    
    class Listener final : public casper::app::monitor::Minimalist::Listener
    {
        
    private: // Ptrs
        
        AppDelegate* delegate_;
        
    public: // Constructor(s) / Destructor
        
        Listener (AppDelegate* a_delegate)
        {
            delegate_ = a_delegate;
        }
        
        virtual ~Listener ()
        {
            /* empty */
        }
        
    public: // API Inherited Pure Virtual Method(s) / Function(s)
        
        virtual void OnRunningProcessesUpdated (const casper::app::monitor::Process::List& a_list)
        {
            [delegate_ setRunningProcesses: a_list];
        }
        
    }; // end of class 'Listener'

    Listener l(delegate);
    int      result = 0;
    
//    const NSString* runtimeDirectoryPrefixURL = [PreferencesWindowController runtimeDirectoryPrefixURL];
    const NSString* configDirectoryPrefixURL    = [PreferencesWindowController configDirectoryPrefixURL];
    
    const NSString* postgresSQLDataDirectoryURL = [PreferencesWindowController postgreSQLDataDirectoryURL];
    const NSString* postgreSQLArguments         = [PreferencesWindowController postgreSQLArguments];
    
    const std::string working_dir    = NORMALIZE_PATH((
                                                       std::string([cacheDirectory cStringUsingEncoding:NSUTF8StringEncoding])
                                                       + '/' +
                                                       std::string([bundleIdenfitifer cStringUsingEncoding:NSUTF8StringEncoding])
                                                      ));

//    const std::string runtime_path    = NORMALIZE_PATH(
//                                         ((
//                                            nil != runtimeDirectoryPrefixURL
//                                                ?
//                                                    std::string([runtimeDirectoryPrefixURL cStringUsingEncoding:NSUTF8StringEncoding])
//                                                :
//                                                    working_dir
//                                           ) + std::string("/var/run"))
//                                        );
    
    const std::string runtime_path        = "/usr/local/var/run/casper/";
    
#ifdef DEBUG
    const char* const rt_app_dir_prefix = getenv("XCODE_RUNTIME_APP_PATH");
#else
    const char* const rt_app_dir_prefix = nullptr;
#endif

    const std::string app_dir_prefix    = NORMALIZE_PATH(
                                            ((
                                                nil != rt_app_dir_prefix
                                                    ?
                                                        rt_app_dir_prefix
                                                    :
                                                       ( std::string([[bundle executablePath] cStringUsingEncoding:NSUTF8StringEncoding]) + "/.." )
                                            ))
                                        );

    const std::string config_dir_prefix = NORMALIZE_PATH(
                                                       (
                                                        nil != configDirectoryPrefixURL
                                                            ?
                                                                std::string([configDirectoryPrefixURL cStringUsingEncoding:NSUTF8StringEncoding])
                                                            :
                                                                std::string([[bundle resourcePath] cStringUsingEncoding:NSUTF8StringEncoding]) + "/.."
                                                        )
                                    );
    const std::string resouces_dir   = NORMALIZE_PATH(std::string([[bundle resourcePath] cStringUsingEncoding:NSUTF8StringEncoding]));
    
    
    const std::string postgresql_data_dir = NORMALIZE_PATH(
                                          ((
                                            nil != postgresSQLDataDirectoryURL
                                                ?
                                                    std::string([postgresSQLDataDirectoryURL cStringUsingEncoding:NSUTF8StringEncoding])
                                                :
                                                    "/usr/local/data/postgresql/"
                                          ))
    );
    
    const std::string postgresql_arguments = ( nil != postgreSQLArguments ? [postgreSQLArguments cStringUsingEncoding:NSUTF8StringEncoding] : "" );
    
    const casper::app::monitor::Minimalist::Settings monitor_settings = {
        /* runtime_directory_       */ NORMALIZE_PATH(runtime_path),
        /* resources_directory_     */ resouces_dir,
        /* working_directory_       */ working_dir,
        /* logs_directory_          */ logs_dir,
        /* app_directory_prefix_    */ app_dir_prefix,
        /* config_directory_prefix_ */ config_dir_prefix,
        /* postgresql               */ {
            /* data_directory_ */ postgresql_data_dir,
            /* arguments_      */ postgresql_arguments
        }
    };
    
    CASPER_APP_DEBUG_LOG("status", "%-30s:"   , "Settings"                                                                    );
    CASPER_APP_DEBUG_LOG("status", "%-30s: %s", "\truntime directory"      , monitor_settings.runtime_directory_.c_str()      );
    CASPER_APP_DEBUG_LOG("status", "%-30s: %s", "\tresources directory"    , monitor_settings.resources_directory_.c_str()    );
    CASPER_APP_DEBUG_LOG("status", "%-30s: %s", "\tworking directory"      , monitor_settings.working_directory_.c_str()      );
    CASPER_APP_DEBUG_LOG("status", "%-30s: %s", "\tlogs directory"         , monitor_settings.logs_directory_.c_str()         );
    CASPER_APP_DEBUG_LOG("status", "%-30s: %s", "\tapp directory prefix"   , monitor_settings.app_directory_prefix_.c_str()   );
    CASPER_APP_DEBUG_LOG("status", "%-30s: %s", "\tconfig directory prefix", monitor_settings.config_directory_prefix_.c_str());
    
    CASPER_APP_DEBUG_LOG("status", "%s", "Initializing monitoring process...");
    if ( false == casper::app::monitor::Minimalist::GetInstance().Start(&l, monitor_settings) ) {
        const auto last_error = casper::app::monitor::Minimalist::GetInstance().LastError();
        // ... failure ...
        CASPER_APP_LOG("status", "Failed to start casper app monitoring process: %s!",
                               last_error->message().c_str()
        );
        result = -1;
    }

    CASPER_APP_DEBUG_LOG("status", "%s", "CEF run loop...");
    // ... run the message loop?
    // ... this will block until Quit() is called ...
    result = message_loop->Run();

    // ... finally shut down CEF ...
    CASPER_APP_DEBUG_LOG("status", "%s", "CEF shutdown...");
    context->Shutdown();

    // ... release objects ( in reverse order of creation ) ...
    [delegate release];
    message_loop.reset();
    context.reset();
    [autopool release];

    casper::app::monitor::Minimalist::GetInstance().Stop();
    casper::app::monitor::Minimalist::Destroy();
        
    CASPER_APP_LOG("status", "%s", "Stopped");
    
    // ... done ...
    return result;
}


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

#import "Alerts.h"

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

#include "casper/mac/monitor.h"

#include <stdarg.h> // va_list, va_start, va_end

#include "casper/app/version.h"

#include "casper/app/logger.h"

#include "osal/osalite.h"
#include "osal/osal_file.h"

#include "ev/signals.h"

#include "cc/b64.h"

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

static bool EnsurePath (NSSearchPathDirectory a_path, const std::vector<std::string>& a_append, std::string& o_path)
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
 * @brief Call this function tto start a child process to launch and monitor other processes.
 *
 * param a_settings
 *
 * return
 */

typedef struct {
    const std::string data_dir_;
    const std::string arguments_;
} PostgreSQLSettings;

typedef struct {
    const std::string runtime_directory_;
    const std::string executable_directory_;
    const std::string config_directory_;
    const std::string working_directory_;
    const std::string cache_directory_;
    const std::string logs_directory_;
} AppSettings;

typedef struct {
    AppSettings        app_;
    PostgreSQLSettings postgreSQL_;
    const std::string  config_dir_prefix_;
    const std::string  apps_dir_prefix_;
    const std::string  working_directory_;
} MonitorSettings;


//static int StartMonitor (const MonitorSettings& a_settings)
//{
//
//    fprintf(stdout, "EXECVP 2!!!!!\n");
//
//    return 0;
//}

class CEF2APPHook : public CefRefCount
{
    
private:
    
    std::mutex mutex_;
    
private: //
    
    std::function<void()> callback_;
    
private: // Ptrs
    
    AppDelegate* app_delegate_;
    
public:
    
    inline void Set (AppDelegate* a_delegate)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        app_delegate_ = a_delegate;
    }
    
    inline AppDelegate* Bind (std::function<void()> a_callback)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        callback_ = a_callback;
        
        return app_delegate_;
    }
    
public: // Static Method(s) / Function(s)
    
    static void Callback (CefRefPtr<CEF2APPHook> a_hook)
    {
        std::lock_guard<std::mutex> lock(a_hook->mutex_);
        a_hook->callback_();
    }

    static void FatalException (CefRefPtr<CEF2APPHook> a_hook, const std::exception a_std_exception)
    {
        std::lock_guard<std::mutex> lock(a_hook->mutex_);
        [a_hook->app_delegate_ showException:a_std_exception delayFor:2 andQuit:YES];
    }
    
}; // end of class 'CEF2APPHook'

/**
 * @brief Call this function from 'main' thread to start a CEF3 instance.
 *
 * param a_argc
 * param a_argv
 *
 * param a_settings
 * param a_started_callback
 * param a_finished_callback
 *
 * return
 */
static int StartCEF3 (int a_argc, char* a_argv[],
                      const casper::cef3::browser::Settings& a_settings,
                      const std::function<void(CefRefPtr<CEF2APPHook>, casper::app::mac::Monitor::QuitCallback)>& a_started_callback,
                      const std::function<void()> a_finished_callback)
{
    //... load the CEF framework library at runtime instead of linking directly ...
    // ... as required by the macOS sandbox implementation ...
    CefScopedLibraryLoader library_loader;
    if ( false == library_loader.LoadInMain() ) {
        CASPER_APP_LOG("status",
                       "%s", "Unable to load CEF framework library!"
        );
        return -1;
    }
    
    const CefMainArgs main_args(a_argc, a_argv);
    
    const BOOL firstRun = ( NO == [PreferencesWindowController configured] );
    
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
    
    // ... startup logs ...
    try {
        
        ::casper::app::Logger::GetInstance().Startup(a_settings.paths_.logs_path_, "casper", CASPER_INFO);
        
    } catch (const std::exception* a_std_exception) {
        [Alerts showCriticalMessage: @"An std::exception Occurred"
                    informativeText: [NSString stringWithCString: a_std_exception->what() encoding: NSUTF8StringEncoding]
                         andButtons: @[@"Ok"]
        ];
        return -1;
    } catch (const std::exception& a_std_exception) {
        [Alerts showCriticalMessage: @"An std::exception Occurred"
                    informativeText: [NSString stringWithCString: a_std_exception.what() encoding: NSUTF8StringEncoding]
                         andButtons: @[@"Ok"]
         ];
        return -1;
    }
    
    
    CASPER_APP_DEBUG_LOG("status", "%s", "CEF creating main context...");
    
    // ... reate the main context object ...
    scoped_ptr<casper::cef3::client::mac::MainContext> context(new casper::cef3::client::mac::MainContext(a_settings, command_line));
    
    CefSettings settings;
    
    // ... populate the settings based on command line arguments ...
    context->PopulateSettings(&settings);
    
    // ... create the main message loop object ...
    scoped_ptr<casper::cef3::browser::MainMessageLoop> message_loop;
    if ( NO == firstRun ) {
        message_loop.reset(new casper::cef3::browser::MainMessageLoopStd([]() {
            casper::app::mac::Monitor::GetInstance().Stop(/* a_soft*/ true);
        }));
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
    
    CefRefPtr<CEF2APPHook> hook = new CEF2APPHook();
    
    hook->Set(delegate);
    
    if ( NO == firstRun ) {
        a_started_callback(hook, [&message_loop, delegate] (const Json::Value& a_error) {
            // ... monitor process exited ...
            [delegate setRunningProcesses: Json::Value::null];
            if ( false == a_error.isNull() ) {
                [delegate showError: a_error andRelaunch: YES];
            }
            message_loop->Quit();
        });
    } else {
        [delegate showPreferences: nil];
    }

    CASPER_APP_DEBUG_LOG("status", "%s", "CEF run loop...");
    // ... run the message loop?
    // ... this will block until Quit() is called ...
    int result = message_loop->Run();

    // ... finally shut down CEF ...
    CASPER_APP_DEBUG_LOG("status", "%s", "CEF shutdown...");
    context->Shutdown();

    if ( NO == firstRun ) {
        a_finished_callback();
    }

    // ... reset delegate ..
    hook->Set(static_cast<AppDelegate*>(nullptr));
    
    const BOOL relaunch = [delegate shouldRelaunch];
    
    // ... release objects ( in reverse order of creation ) ...
    [delegate release];
    message_loop.reset();
    context.reset();
    [autopool release];
    
    CASPER_APP_DEBUG_LOG("status", "%s", "Stopped...");
    
    if ( YES == relaunch ) {
        CASPER_APP_DEBUG_LOG("status", "%s", "Relaunch...");
        [NSTask launchedTaskWithLaunchPath:[[NSBundle mainBundle]executablePath] arguments: @[]];
    }
    
    // ... done ...
    return result;
}

/**
 * @brief casper-app entry point
 *
 * @param a_argc
 * @param a_arvg
 */
int main (int a_argc, char* a_argv[])
{
#ifdef __APPLE__
    signal(SIGPIPE, SIG_IGN);
#endif
    
    // ... set locale must be called @ main(int argc, char** argv) ...
    const char* lc_all = setlocale (LC_ALL, NULL);
    
    setlocale (LC_NUMERIC, "C");
    
    const char* lc_numeric = setlocale (LC_NUMERIC, NULL);

    // ... install signal(s) handler ...
    ::ev::Signals::GetInstance().Startup(::casper::app::Logger::GetInstance().loggable_data());
    ::ev::Signals::GetInstance().Register(
                                          /* a_signals */
                                          { SIGUSR1, SIGTERM, SIGQUIT, SIGTTIN },
                                          /* a_callback */
                                          [](const int a_sig_no) {
                                              // ... is a 'shutdown' signal?
                                              switch(a_sig_no) {
                                                  case SIGQUIT:
                                                  case SIGTERM:
                                                  {
                                                      AppDelegate* delegate = (AppDelegate*)[NSApp delegate];
                                                      [delegate quit: nil];
                                                  }
                                                      return true;
                                                  default:
                                                      return false;
                                              }
                                          }
    );
    
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
    osal::debug::Trace::GetInstance().Log("status", "\t- %-12s: %s\n"     , "LC_ALL"    , lc_all);
    osal::debug::Trace::GetInstance().Log("status", "\t- %-12s: %s - " DOUBLE_FMT "\n", "LC_NUMERIC", lc_numeric, (double)123.456);

    osal::debug::Trace::GetInstance().Log("status", "* %s - %s process w/pid %u configured...\n",
                                          CASPER_INFO, "main", getpid()
    );
    
    // ...
    
    const NSBundle*      bundle            = [NSBundle mainBundle];
    const NSString*      bundleIdenfitifer = (NSString*)[bundle objectForInfoDictionaryKey:@"CFBundleIdentifier"];
    const NSString*      productVersion    = (NSString*)[bundle objectForInfoDictionaryKey:@"CFBundleShortVersionString"];
    const NSString*      productName       = (NSString*)[bundle objectForInfoDictionaryKey:@"CFBundleName"];
    const NSProcessInfo* processInfo       = [NSProcessInfo processInfo];
    const NSString*      userAgent         = [[NSString alloc]initWithFormat: @"%@ / %@ ( macOS %@",
                                              productName, productVersion, [processInfo operatingSystemVersionString]
                                             ];
    const NSString*      initialURL        = (NSString*)[bundle objectForInfoDictionaryKey:@"CASPERInitialURL"];
    const BOOL           appIsAgent        = [[bundle objectForInfoDictionaryKey:@"LSUIElement"] boolValue];
    
    const NSString*   userDirectory     = NSHomeDirectory();
    const std::string logs_dir          = NORMALIZE_PATH((
                                                          std::string([userDirectory cStringUsingEncoding:NSUTF8StringEncoding])
                                                          +
                                                          std::string("/Library/Logs/") + [productName cStringUsingEncoding:NSUTF8StringEncoding]
                                          ));

    const std::string exec_dir = NORMALIZE_PATH((std::string([[bundle executablePath] cStringUsingEncoding:NSUTF8StringEncoding]) + "/.." ));
    
    casper::cef3::browser::Settings browser_settings = {
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
            /* app_executable_directory */ exec_dir,
            /* app_working_directory_ */ "",
            /* logs_path_             */ logs_dir,
            /* downloads_path_        */ "",
            /* cache_path_            */ ""
        }
    };
    
    if ( false == EnsurePath(NSApplicationDirectory, {}, browser_settings.paths_.app_working_directory_) ) {
        return -1;
    }
    
    if ( false == EnsurePath(NSCachesDirectory, { [bundleIdenfitifer cStringUsingEncoding:NSUTF8StringEncoding] }, browser_settings.paths_.cache_path_) ) {
        return -1;
    }
    
    if ( false == EnsurePath(NSDownloadsDirectory, {}, browser_settings.paths_.downloads_path_) ) {
        return -1;
    }

    // TODO CW
    [[NSFileManager defaultManager]createDirectoryAtPath:[NSString stringWithCString:logs_dir.c_str() encoding:NSUTF8StringEncoding]
                             withIntermediateDirectories:YES attributes:nil error:nil];
    
//    if ( false == EnsurePath(NSCachesDirectory, { [bundleIdenfitifer cStringUsingEncoding:NSUTF8StringEncoding], "logs"}, browser_settings.paths_.logs_path_) ) {
//        return -1;
//    }

    
//    const NSString* runtimeDirectoryPrefixURL   = [PreferencesWindowController runtimeDirectoryPrefixURL];
    const NSString* configDirectoryPrefixURL    = [PreferencesWindowController configDirectoryPrefixURL];
    
    const NSString* postgresSQLDataDirectoryURL = [PreferencesWindowController postgreSQLDataDirectoryURL];
    const NSString* postgreSQLArguments         = [PreferencesWindowController postgreSQLArguments];
    
    const std::string working_dir    = NORMALIZE_PATH(browser_settings.paths_.cache_path_);
    
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
    

    const char* rt_app_dir_prefix;
    const char* rt_monitor_app_dir;

#ifdef DEBUG
    const char* const xcode_app_debug = getenv("XCODE_APP_DEBUG");
    if ( nullptr != xcode_app_debug && 0 == strcasecmp("true", xcode_app_debug) ) {
        rt_app_dir_prefix  = getenv("XCODE_RUNTIME_APP_PATH");
        rt_monitor_app_dir = getenv("XCODE_RUNTIME_MONITOR_APP_DIR");
    } else {
        rt_app_dir_prefix  = nullptr;
        rt_monitor_app_dir = nullptr;
    }
#else
    rt_app_dir_prefix  = nullptr;
    rt_monitor_app_dir = nullptr;
#endif
    
    const std::string app_dir_prefix    = NORMALIZE_PATH(nullptr != rt_app_dir_prefix
                                                          ?
                                                         rt_app_dir_prefix
                                                          :
                                                         exec_dir
                                          );
    
    const std::string monitor_app_dir   = NORMALIZE_PATH(nullptr != rt_monitor_app_dir
                                                          ?
                                                         rt_monitor_app_dir
                                                          :
                                                         exec_dir
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
    const std::string resources_dir   = NORMALIZE_PATH(std::string([[bundle resourcePath] cStringUsingEncoding:NSUTF8StringEncoding]));
    
    
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
    
    const MonitorSettings monitor_settings = {
        /* app_        */ {
            /* runtime_directory_    */ runtime_path,
            /* executable_directory_ */ exec_dir,
            /* config_directory_     */ resources_dir,
            /* working_directory_    */ working_dir,
            /* cache_directory_      */ browser_settings.paths_.cache_path_,
            /* logs_directory_       */ logs_dir
        },
        /* postgreSQL_ */ {
            /* data_dir_  */ postgresql_data_dir,
            /* arguments_ */ postgresql_arguments
        },
        /* config_dir_prefix_ */ config_dir_prefix,
        /* apps_dir_prefix_   */ app_dir_prefix,
        /* working_directory_ */ working_dir
    };

    Json::Value directories  = Json::Value(Json::ValueType::objectValue);
    
    directories["runtime"] = monitor_settings.app_.runtime_directory_;
    directories["working"] = monitor_settings.app_.working_directory_;
    directories["config"]  = monitor_settings.app_.config_directory_;
    directories["logs"]    = monitor_settings.app_.logs_directory_;
    
    Json::Value commonVariables                        = Json::Value(Json::ValueType::objectValue);
    commonVariables["@@APP_DIRECTORY_PREFIX@@"]        = monitor_settings.apps_dir_prefix_;
    commonVariables["@@APP_CONFIG_DIRECTORY_PREFIX@@"] = monitor_settings.config_dir_prefix_;
    commonVariables["@@APP_WORKING_DIRECTORY_PATH@@"]  = monitor_settings.working_directory_;
    
    Json::Value postgreSQLVariables                     = Json::Value(Json::ValueType::objectValue);
    postgreSQLVariables["@@APP_POSTGRESQL_DATA_DIR@@" ] = monitor_settings.postgreSQL_.data_dir_;
    postgreSQLVariables["@@APP_POSTGRESQL_ARGUMENTS@@"] = monitor_settings.postgreSQL_.arguments_;
    
    
    Json::Value config                = Json::Value(Json::ValueType::objectValue);
    config["directories"]             = directories;
    config["variables"]["common"]     = commonVariables;
    config["variables"]["postgresql"] = postgreSQLVariables;
    
    Json::FastWriter fast_writer;
    
    const std::string b64_config  = cc::base64_url_unpadded::encode(fast_writer.write(config));
    const std::string launch_path = monitor_app_dir + "monitor";    
    
    osal::File::Delete(directories["runtime"].asCString(), "*.socket", nullptr);
    
    return StartCEF3(a_argc, a_argv, browser_settings,
                     /* a_started_callback */
                     [&b64_config, &directories, &launch_path] (CefRefPtr<CEF2APPHook> hook, casper::app::mac::Monitor::QuitCallback quit_callback) {
                         
                         CefRefPtr<CEF2APPHook> lambda_hook = hook;
                         
                         
                         Json::Value config = Json::Value(Json::ValueType::objectValue);
                         
                         config["directories"]["runtime"] = directories["runtime"].asCString();
                         config["monitor"]["path"] = launch_path;
                         config["monitor"]["arguments"] = Json::Value(Json::ValueType::arrayValue);
                         config["monitor"]["arguments"].append("-c");
                         config["monitor"]["arguments"].append(b64_config);
                         
                         casper::app::mac::Monitor::GetInstance().Start(config,
                                                                        /* a_bind_callback */
                                                                        [lambda_hook] (std::function<void()> a_callback) -> AppDelegate* {
                                                                            
                                                                            return lambda_hook->Bind(a_callback);
                                                                            
                                                                        },
                                                                        /* a_dispatch_callback */
                                                                        [lambda_hook] () {
                                                                            
                                                                            CefPostTask(TID_UI, base::Bind(&CEF2APPHook::Callback, lambda_hook));
                                                                            
                                                                        },
                                                                        quit_callback
                        );
                         
                     },
                     [] {
                         casper::app::mac::Monitor::GetInstance().Stop(/* a_destroy */ true);
                         casper::app::mac::Monitor::Destroy();
                     }
    );
}

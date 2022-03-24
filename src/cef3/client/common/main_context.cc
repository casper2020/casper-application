// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef3/client/common/main_context.h"

#include "cef3/browser/root_window_manager.h"

#include "cef3/common/client/switches.h"

/**
 * @brief Default constructor.
 *
 * @param a_settings
 * @param a_command_line
 */
casper::cef3::client::common::MainContext::MainContext (const casper::cef3::browser::Settings& a_settings,
                                                        CefRefPtr<CefCommandLine> a_command_line)
    : casper::cef3::browser::MainContext::MainContext(a_settings),
      command_line_(a_command_line),
      status_({
          /* initialized_ */ false,
          /* shutdown_    */ false
      })
{
    /* empty */
}

/**
 * @brief Destructor
 */
casper::cef3::client::common::MainContext::~MainContext ()
{
    /* empty */
}

#ifdef __APPLE__
#pragma mark -
#endif

/**
 * @brief Set download URI for a suggested file name.
 *
 * @param a_suggested_file_name
 * @param o_uri
 */
bool casper::cef3::client::common::MainContext::GetDownloadURI (const std::string& a_suggested_file_name, std::string& o_uri)
{
    if ( false == GetDownloadsPath(o_uri) ) {
        return false;
    }
    o_uri += a_suggested_file_name;
    return true;
}

#ifdef __APPLE__
#pragma mark -
#endif

/**
 * @brief Initialize CEF and associated main context state.
 *
 * This method must be called on the same thread that created this object.
 */
bool casper::cef3::client::common::MainContext::Initialize (const CefMainArgs& args, const CefSettings& settings, CefRefPtr<CefApp> application,
                                                            void* windows_sandbox_info)
{
    
    DCHECK(thread_checker_.CalledOnValidThread());
    DCHECK(false == status_.initialized_);
    DCHECK(false == status_.shutdown_);
    
    if ( false == CefInitialize(args, settings, application, windows_sandbox_info) ) {
        return false;
    }
    
    if ( 0 == settings_.paths_.app_working_directory_.length() ) {
        (void)GetDownloadsPath(settings_.paths_.app_working_directory_);
    }
    if ( 0 == settings_.paths_.downloads_path_.length() ) {
        (void)GetDownloadsPath(settings_.paths_.downloads_path_);
    }
    if ( 0 == settings_.paths_.logs_path_.length() ) {
        (void)GetLogsPath(settings_.paths_.logs_path_);
    }
    
    root_window_manager_.reset(new casper::cef3::browser::RootWindowManager(settings_.application_.terminate_when_all_windows_closed_));
    
    status_.initialized_ = true;
    
    return true;
}

/**
 * @brief Shut down CEF and associated context state.
 *
 * This method must be called on the same thread that created this object.
 */
void casper::cef3::client::common::MainContext::Shutdown()
{
    DCHECK(thread_checker_.CalledOnValidThread());
    DCHECK(status_.initialized_);
    DCHECK(false == status_.shutdown_);
    
    root_window_manager_.reset();
    
    CefShutdown();
    
    status_.shutdown_ = true;
}


#ifdef __APPLE__
#pragma mark -
#endif

std::string casper::cef3::client::common::MainContext::GetConsoleLogPath ()
{
    return settings_.paths_.logs_path_ + "console.log";
}

std::string casper::cef3::client::common::MainContext::GetDownloadPath (const std::string& a_file_name)
{
    return settings_.paths_.downloads_path_ + a_file_name;
}

std::string casper::cef3::client::common::MainContext::GetAppWorkingDirectory ()
{
    return std::string();
}

std::string casper::cef3::client::common::MainContext::GetMainURL()
{
    return settings_.application_.main_url_;
}

cef_color_t casper::cef3::client::common::MainContext::GetBackgroundColor ()
{
    return settings_.application_.window_.background_color_;
}

bool casper::cef3::client::common::MainContext::UseViews ()
{
    return settings_.application_.window_.use_views_;
}

bool casper::cef3::client::common::MainContext::UseWindowlessRendering ()
{
    return settings_.application_.window_.use_windowless_rendering_;
}

void casper::cef3::client::common::MainContext::PopulateSettings (CefSettings* o_settings)
{
    
    // When generating projects with CMake the CEF_USE_SANDBOX value will be defined
    // automatically. Pass -DUSE_SANDBOX=OFF to the CMake command-line to disable
    // use of the sandbox.
#if !defined(CEF_USE_SANDBOX)
    o_settings->no_sandbox = true;
#endif

#if defined(OS_WIN) || defined(OS_LINUX)
    o_settings->multi_threaded_message_loop = a_command_line->HasSwitch(casper::cef3::common::client::switches::kMultiThreadedMessageLoop);
#endif
    
    if ( false == o_settings->multi_threaded_message_loop ) {
        o_settings->external_message_pump = command_line_->HasSwitch(casper::cef3::common::client::switches::kExternalMessagePump);
    }
    
    CefString(&o_settings->user_agent)         = settings_.application_.user_agent_;
    CefString(&o_settings->user_agent_product) = settings_.product_.version_;
    CefString(&o_settings->cache_path)         = command_line_->GetSwitchValue(casper::cef3::common::client::switches::kCachePath);
//    // TODO CW
//    if ( nullptr == o_settings->cache_path.str || 0 == o_settings->cache_path.length ) {
//        cef_string_ascii_to_utf16(settings_.paths_.cache_path_.c_str(), settings_.paths_.cache_path_.length(), &o_settings->cache_path);
//        o_settings->persist_session_cookies = 1;
//    }

    if ( true == settings_.application_.window_.use_windowless_rendering_ ) {
        o_settings->windowless_rendering_enabled = true;
    }    
    
    if ( 0 != settings_.application_.window_.background_color_ ) {
        o_settings->background_color = settings_.application_.window_.background_color_;
    }
}

void casper::cef3::client::common::MainContext::PopulateBrowserSettings (CefBrowserSettings* o_settings)
{
    if ( true == command_line_->HasSwitch(casper::cef3::common::client::switches::kOffScreenFrameRate) ) {
        o_settings->windowless_frame_rate = atoi(command_line_->GetSwitchValue(casper::cef3::common::client::switches::kOffScreenFrameRate).ToString().c_str());
    }
    if ( 0 != settings_.application_.window_.background_color_ ) {
        o_settings->background_color = settings_.application_.window_.background_color_;
    }
    
//    if ( settings_.paths_.cache_path_.length() > 0 ) {
//        o_settings->application_cache = STATE_ENABLED;
//    }
}
// TODO CW
//void casper::cef3::client::common::MainContext::PopulateOsrSettings (::client::OsrRenderer::Settings* o_settings)
//{
//    o_settings->show_update_rect = command_line_->HasSwitch(casper::cef3::common::client::switches::kShowUpdateRect);
//    if ( 0 != settings_.application_.window_.background_color_ ) {
//        o_settings->background_color = settings_.application_.window_.background_color_;
//    }
//}

casper::cef3::browser::RootWindowManager* casper::cef3::client::common::MainContext::GetRootWindowManager()
{
    DCHECK(status_.Valid());
    return root_window_manager_.get();
}


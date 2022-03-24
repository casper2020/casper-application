// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef3/client/common/display_handler.h"

#include "include/wrapper/cef_helpers.h" // CEF_REQUIRE_UI_THREAD

#include "cef3/browser/main_context.h"

#include "cef3/client/common/client_handler.h" // ClientHandlerDelegate

#include "cef3/browser/main_message_loop.h" // CURRENTLY_ON_MAIN_THREAD, MAIN_POST_CLOSURE

casper::cef3::client::common::DisplayHandler::DisplayHandler (CefRefPtr<casper::cef3::client::common::BaseHandler> a_base_handler)
    : base_handler_(a_base_handler),
      console_log_file_(casper::cef3::browser::MainContext::Get()->GetConsoleLogPath())
{
    DCHECK(!console_log_file_.empty());
}

casper::cef3::client::common::DisplayHandler::~DisplayHandler ()
{
    /* empty */
}

#if defined(OS_WIN)
    #define NEWLINE "\r\n"
#else
    #define NEWLINE "\n"
#endif

#ifdef __APPLE__
#pragma mark - CefDisplayHandler
#endif

bool casper::cef3::client::common::DisplayHandler::OnConsoleMessage (CefRefPtr<CefBrowser> a_browser,
                                                                     cef_log_severity_t a_level, const CefString& a_message, const CefString& a_source, int a_line)
{
    CEF_REQUIRE_UI_THREAD();
    
    std::stringstream ss;
    ss << "Level: ";
    switch (a_level) {
        case LOGSEVERITY_DEBUG:
            ss << "Debug" << NEWLINE;
            break;
        case LOGSEVERITY_INFO:
            ss << "Info" << NEWLINE;
            break;
        case LOGSEVERITY_WARNING:
            ss << "Warn" << NEWLINE;
            break;
        case LOGSEVERITY_ERROR:
            ss << "Error" << NEWLINE;
            break;
        default:
            NOTREACHED();
            break;
    }
    ss << "Message: " << a_message.ToString() << NEWLINE
    << "Source: " << a_source.ToString() << NEWLINE << "Line: " << a_line
    << NEWLINE << "-----------------------" << NEWLINE;
    
    return WriteToConsoleLog(ss.str().c_str());
}

#ifdef __APPLE__
#pragma mark -
#endif

/**
 * @brief Write a message to the log file.
 *
 * @param a_message
 */
bool casper::cef3::client::common::DisplayHandler::WriteToConsoleLog (const std::string& a_message)
{
#if 1
    fputs(a_message.c_str(), stderr);
    fflush(stderr);
#else
    FILE* file = fopen(console_log_file_.c_str(), "a");
    if ( nullptr == file ) {
        return false;
    }
    fputs(a_message.c_str(), file);
    fclose(file);
#endif
    return true;
}

#ifdef __APPLE__
#pragma mark -
#endif

void casper::cef3::client::common::DisplayHandler::NotifyAddress (const CefString& url)
{
    if (!CURRENTLY_ON_MAIN_THREAD()) {
        MAIN_POST_CLOSURE(base::BindOnce(&casper::cef3::client::common::DisplayHandler::NotifyAddress, this, url));
        return;
    }

    if ( base_handler_->delegate_ptr_)
        base_handler_->delegate_ptr_->OnSetAddress(url);
}

void casper::cef3::client::common::DisplayHandler::NotifyTitle (const CefString& title)
{
    if (!CURRENTLY_ON_MAIN_THREAD()) {
        MAIN_POST_CLOSURE(base::BindOnce(&casper::cef3::client::common::DisplayHandler::NotifyTitle, this, title));
        return;
    }

    if ( base_handler_->delegate_ptr_ )
        base_handler_->delegate_ptr_->OnSetTitle(title);
}

void casper::cef3::client::common::DisplayHandler::NotifyFavicon (CefRefPtr<CefImage> image)
{
    if (!CURRENTLY_ON_MAIN_THREAD()) {
        MAIN_POST_CLOSURE(base::BindOnce(&casper::cef3::client::common::DisplayHandler::NotifyFavicon, this, image));
        return;
    }

    if ( base_handler_->delegate_ptr_ )
        base_handler_->delegate_ptr_->OnSetFavicon(image);
}

void casper::cef3::client::common::DisplayHandler::NotifyFullscreen (bool fullscreen)
{
    if (!CURRENTLY_ON_MAIN_THREAD()) {
        MAIN_POST_CLOSURE(base::BindOnce(&casper::cef3::client::common::DisplayHandler::NotifyFullscreen, this, fullscreen));
        return;
    }

    if ( base_handler_->delegate_ptr_ )
        base_handler_->delegate_ptr_->OnSetFullscreen(fullscreen);
}

void casper::cef3::client::common::DisplayHandler::NotifyAutoResize (const CefSize& new_size)
{
    if (!CURRENTLY_ON_MAIN_THREAD()) {
        MAIN_POST_CLOSURE(base::BindOnce(&casper::cef3::client::common::DisplayHandler::NotifyAutoResize, this, new_size));
        return;
    }

    if ( base_handler_->delegate_ptr_ )
        base_handler_->delegate_ptr_->OnAutoResize(new_size);
}

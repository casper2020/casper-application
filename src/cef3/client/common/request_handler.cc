// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef3/client/common/request_handler.h"

#include "include/wrapper/cef_helpers.h" // CEF_REQUIRE_UI_THREAD

#include "include/cef_command_line.h"    // CefCommandLine

#include "cef3/browser/root_window_config.h"
#include "cef3/browser/root_window_manager.h"

#include "cef3/browser/main_context.h"

#include "cef3/common/client/switches.h"

/**
 * @brief Default constructor
 *
 * @param a_startup_url
 * @param a_is_osr
 * @param a_base_handler
 */
casper::cef3::client::common::RequestHandler::RequestHandler (const std::string& a_startup_url,
                                                              const bool&        a_is_osr,
                                                              CefRefPtr<casper::cef3::client::common::BaseHandler> a_base_handler)
    : startup_url_(a_startup_url), is_osr_(a_is_osr), base_handler_(a_base_handler)
{
    /* empty */
}

/**
 * @brief Destructor
 */
casper::cef3::client::common::RequestHandler::~RequestHandler ()
{
    /* empty */
}

#ifdef __APPLE__
#pragma mark -
#endif

bool casper::cef3::client::common::RequestHandler::OnBeforeBrowse (CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                                                   CefRefPtr<CefRequest> request,
                                                                   bool user_gesture, bool is_redirect)
{
    CEF_REQUIRE_UI_THREAD();
    
    base_handler_->message_router_->OnBeforeBrowse(browser, frame);
    return false;
}


bool casper::cef3::client::common::RequestHandler::OnOpenURLFromTab (CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                                                     const CefString& target_url,
                                                                     CefRequestHandler::WindowOpenDisposition target_disposition,
                                                                     bool user_gesture)
{
    if ( target_disposition == WOD_NEW_BACKGROUND_TAB || target_disposition == WOD_NEW_FOREGROUND_TAB ) {
        // Handle middle-click and ctrl + left-click by opening the URL in a new browser window.
        casper::cef3::browser::RootWindowConfig config;
        config.with_controls = true;
        config.with_osr = is_osr_;
        config.url = target_url;
        
        casper::cef3::browser::MainContext::Get()->GetRootWindowManager()->CreateRootWindow(config);
        return true;
    }
    
    // Open the URL in the current browser window.
    return false;
}


cef_return_value_t casper::cef3::client::common::RequestHandler::OnBeforeResourceLoad (CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                                                                       CefRefPtr<CefRequest> request,
                                                                                       CefRefPtr<CefRequestCallback> callback)
{
    CEF_REQUIRE_IO_THREAD();
    
    return base_handler_->resource_manager_->OnBeforeResourceLoad(browser, frame, request, callback);
}

CefRefPtr<CefResourceHandler> casper::cef3::client::common::RequestHandler::GetResourceHandler (CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                                                                                  CefRefPtr<CefRequest> request)
{
    CEF_REQUIRE_IO_THREAD();
    
    return base_handler_->resource_manager_->GetResourceHandler(browser, frame, request);
}

CefRefPtr<CefResponseFilter> casper::cef3::client::common::RequestHandler::GetResourceResponseFilter (CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                                                                                      CefRefPtr<CefRequest> request,
                                                                                                      CefRefPtr<CefResponse> response)
{
    CEF_REQUIRE_IO_THREAD();
    
    // MODIFIED BY CW
    return nullptr;
}


bool casper::cef3::client::common::RequestHandler::OnQuotaRequest (CefRefPtr<CefBrowser> browser,
                                                                   const CefString& origin_url,
                                                                   int64 new_size,
                                                                   CefRefPtr<CefRequestCallback> callback)
{
    CEF_REQUIRE_IO_THREAD();
    
    static const int64 max_size = 1024 * 1024 * 20;  // 20mb.
    
    // Grant the quota request if the size is reasonable.
    callback->Continue(new_size <= max_size);
    return true;
}


void casper::cef3::client::common::RequestHandler::OnProtocolExecution (CefRefPtr<CefBrowser> browser,
                                                                        const CefString& url,
                                                                        bool& allow_os_execution)
{
    CEF_REQUIRE_UI_THREAD();
    
    allow_os_execution = false;
}


bool casper::cef3::client::common::RequestHandler::OnCertificateError (CefRefPtr<CefBrowser> browser,
                                                                       CefLoadHandler::ErrorCode cert_error,
                                                                       const CefString& request_url,
                                                                       CefRefPtr<CefSSLInfo> ssl_info,
                                                                       CefRefPtr<CefRequestCallback> callback)
{
    CEF_REQUIRE_UI_THREAD();
    
    if (cert_error == ERR_CERT_AUTHORITY_INVALID &&
        request_url.ToString().find("https://www.magpcss.org/") == 0U) {
        // Allow the CEF Forum to load. It has a self-signed certificate.
        callback->Continue(true);
        return true;
    }
    
    CefRefPtr<CefX509Certificate> cert = ssl_info->GetX509Certificate();
    if ( cert.get() ) {
        // Load the error page.
        // TODO CW - display error
        // LoadErrorPage(browser->GetMainFrame(), request_url, cert_error, GetCertificateInformation(cert, ssl_info->GetCertStatus()));
    }
    
    return false;  // Cancel the request.
}


bool casper::cef3::client::common::RequestHandler::OnSelectClientCertificate (CefRefPtr<CefBrowser> browser,
                                                                              bool isProxy,
                                                                              const CefString& host,
                                                                              int port,
                                                                              const X509CertificateList& certificates,
                                                                              CefRefPtr<CefSelectClientCertificateCallback> callback)
{
    CEF_REQUIRE_UI_THREAD();
    
    const CefRefPtr<CefCommandLine> command_line = CefCommandLine::GetGlobalCommandLine();
    if ( false == command_line->HasSwitch(casper::cef3::common::client::switches::kSslClientCertificate) ) {
        return false;
    }
    
    const std::string& cert_name = command_line->GetSwitchValue(casper::cef3::common::client::switches::kSslClientCertificate);
    
    if (cert_name.empty()) {
        callback->Select(NULL);
        return true;
    }
    
    std::vector<CefRefPtr<CefX509Certificate>>::const_iterator it =
    certificates.begin();
    for (; it != certificates.end(); ++it) {
        CefString subject((*it)->GetSubject()->GetDisplayName());
        if (subject == cert_name) {
            callback->Select(*it);
            return true;
        }
    }
    
    return true;
}


void casper::cef3::client::common::RequestHandler::OnRenderProcessTerminated (CefRefPtr<CefBrowser> browser,
                                                                              TerminationStatus status)
{
    CEF_REQUIRE_UI_THREAD();
    
    base_handler_->message_router_->OnRenderProcessTerminated(browser);
    
    // Don't reload if there's no start URL, or if the crash URL was specified.
    if ( startup_url_.empty() || startup_url_ == "chrome://crash" ) {
        return;
    }
    
    const CefRefPtr<CefFrame> frame = browser->GetMainFrame();
    std::string url = frame->GetURL();
    
    // Don't reload if the termination occurred before any URL had successfully
    // loaded.
    if ( url.empty() ) {
        return;
    }
    
    std::string start_url = startup_url_;
    
    // Convert URLs to lowercase for easier comparison.
    std::transform(url.begin(), url.end(), url.begin(), tolower);
    std::transform(start_url.begin(), start_url.end(), start_url.begin(), tolower);
    
    // Don't reload the URL that just resulted in termination.
    if ( url.find(start_url) == 0 ) {
        return;
    }
    
    frame->LoadURL(startup_url_);
}

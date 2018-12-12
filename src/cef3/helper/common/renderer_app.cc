// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef3/helper/common/renderer_app.h"

/**
 * @brief Default constructor.
 */
casper::cef3::helper::common::RendererApp::RendererApp ()
{
    /* empty */
}

void casper::cef3::helper::common::RendererApp::OnRenderThreadCreated (CefRefPtr<CefListValue> extra_info)
{
    // TODO CW
    return;
//    DelegateSet::iterator it = delegates_.begin();
//    for (; it != delegates_.end(); ++it)
//        (*it)->OnRenderThreadCreated(this, extra_info);
}

void casper::cef3::helper::common::RendererApp::OnWebKitInitialized ()
{
    // TODO CW
    return;
//    DelegateSet::iterator it = delegates_.begin();
//    for (; it != delegates_.end(); ++it)
//        (*it)->OnWebKitInitialized(this);
}

void casper::cef3::helper::common::RendererApp::OnBrowserCreated (CefRefPtr<CefBrowser> browser)
{
    // TODO CW
    return;
//    DelegateSet::iterator it = delegates_.begin();
//    for (; it != delegates_.end(); ++it)
//        (*it)->OnBrowserCreated(this, browser);
}

void casper::cef3::helper::common::RendererApp::OnBrowserDestroyed (CefRefPtr<CefBrowser> browser)
{
    // TODO CW
    return;
//    DelegateSet::iterator it = delegates_.begin();
//    for (; it != delegates_.end(); ++it)
//        (*it)->OnBrowserDestroyed(this, browser);
}

CefRefPtr<CefLoadHandler> casper::cef3::helper::common::RendererApp::GetLoadHandler ()
{
    CefRefPtr<CefLoadHandler> load_handler;
    // TODO CW
//    DelegateSet::iterator it = delegates_.begin();
//    for (; it != delegates_.end() && !load_handler.get(); ++it)
//        load_handler = (*it)->GetLoadHandler(this);
    
    return load_handler;
}

void casper::cef3::helper::common::RendererApp::OnContextCreated(CefRefPtr<CefBrowser> browser,
                                                                 CefRefPtr<CefFrame> frame,
                                                                 CefRefPtr<CefV8Context> context)
{
    // TODO CW
//    DelegateSet::iterator it = delegates_.begin();
//    for (; it != delegates_.end(); ++it)
//        (*it)->OnContextCreated(this, browser, frame, context);
}

void casper::cef3::helper::common::RendererApp::OnContextReleased (CefRefPtr<CefBrowser> browser,
                                                                   CefRefPtr<CefFrame> frame,
                                                                   CefRefPtr<CefV8Context> context)
{
    // TODO CW
//    DelegateSet::iterator it = delegates_.begin();
//    for (; it != delegates_.end(); ++it)
//        (*it)->OnContextReleased(this, browser, frame, context);
}

void casper::cef3::helper::common::RendererApp::OnUncaughtException (CefRefPtr<CefBrowser> browser,
                                                                     CefRefPtr<CefFrame> frame,
                                                                     CefRefPtr<CefV8Context> context,
                                                                     CefRefPtr<CefV8Exception> exception,
                                                                     CefRefPtr<CefV8StackTrace> stackTrace)
{
    // TODO CW
//    DelegateSet::iterator it = delegates_.begin();
//    for (; it != delegates_.end(); ++it) {
//        (*it)->OnUncaughtException(this, browser, frame, context, exception,
//                                   stackTrace);
//    }
}

void casper::cef3::helper::common::RendererApp::OnFocusedNodeChanged (CefRefPtr<CefBrowser> browser,
                                                                      CefRefPtr<CefFrame> frame,
                                                                      CefRefPtr<CefDOMNode> node)
{
    // TODO CW
//    DelegateSet::iterator it = delegates_.begin();
//    for (; it != delegates_.end(); ++it)
//        (*it)->OnFocusedNodeChanged(this, browser, frame, node);
}

bool casper::cef3::helper::common::RendererApp::OnProcessMessageReceived (CefRefPtr<CefBrowser> browser,
                                                                          CefProcessId source_process,
                                                                          CefRefPtr<CefProcessMessage> message)
{
    DCHECK_EQ(source_process, PID_BROWSER);
    
    bool handled = false;
    // TODO CW
//    DelegateSet::iterator it = delegates_.begin();
//    for (; it != delegates_.end() && !handled; ++it) {
//        handled =
//        (*it)->OnProcessMessageReceived(this, browser, source_process, message);
//    }
    
    return handled;
}

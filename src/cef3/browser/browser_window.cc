// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef3/browser/browser_window.h"

#include "include/base/cef_bind.h"

#include "cef3/browser/main_message_loop.h"

casper::cef3::browser::BrowserWindow::BrowserWindow (casper::cef3::browser::BrowserWindow::Delegate* delegate)
    : delegate_(delegate), is_closing_(false)
{
  DCHECK(delegate_);
}

void casper::cef3::browser::BrowserWindow::SetDeviceScaleFactor (float device_scale_factor)
{
    
}

float casper::cef3::browser::BrowserWindow::GetDeviceScaleFactor() const
{
  return 1.0f;
}

CefRefPtr<CefBrowser> casper::cef3::browser::BrowserWindow::GetBrowser() const
{
  REQUIRE_MAIN_THREAD();
  return browser_;
}

bool casper::cef3::browser::BrowserWindow::IsClosing() const
{
  REQUIRE_MAIN_THREAD();
  return is_closing_;
}

void casper::cef3::browser::BrowserWindow::OnBrowserCreated(CefRefPtr<CefBrowser> browser)
{
  REQUIRE_MAIN_THREAD();
  DCHECK(!browser_);
  browser_ = browser;

  delegate_->OnBrowserCreated(browser);
}

void casper::cef3::browser::BrowserWindow::OnBrowserClosing(CefRefPtr<CefBrowser> browser)
{
  REQUIRE_MAIN_THREAD();
  DCHECK_EQ(browser->GetIdentifier(), browser_->GetIdentifier());
  is_closing_ = true;

  delegate_->OnBrowserWindowClosing();
}

void casper::cef3::browser::BrowserWindow::OnBrowserClosed (CefRefPtr<CefBrowser> browser)
{
  REQUIRE_MAIN_THREAD();
  if (browser_.get()) {
    DCHECK_EQ(browser->GetIdentifier(), browser_->GetIdentifier());
    browser_ = NULL;
  }

  client_handler_->DetachDelegate();
  client_handler_ = NULL;

  // |this| may be deleted.
  delegate_->OnBrowserWindowDestroyed();
}

void casper::cef3::browser::BrowserWindow::OnSetAddress(const std::string& url)
{
  REQUIRE_MAIN_THREAD();
  delegate_->OnSetAddress(url);
}

void casper::cef3::browser::BrowserWindow::OnSetTitle(const std::string& title)
{
  REQUIRE_MAIN_THREAD();
  delegate_->OnSetTitle(title);
}

void casper::cef3::browser::BrowserWindow::OnSetFullscreen(bool fullscreen)
{
  REQUIRE_MAIN_THREAD();
  delegate_->OnSetFullscreen(fullscreen);
}

void casper::cef3::browser::BrowserWindow::OnAutoResize(const CefSize& new_size)
{
  REQUIRE_MAIN_THREAD();
  delegate_->OnAutoResize(new_size);
}

void casper::cef3::browser::BrowserWindow::OnSetLoadingState (bool isLoading, bool canGoBack, bool canGoForward)
{
  REQUIRE_MAIN_THREAD();
  delegate_->OnSetLoadingState(isLoading, canGoBack, canGoForward);
}

void casper::cef3::browser::BrowserWindow::OnSetDraggableRegions ( const std::vector<CefDraggableRegion>& regions)
{
  REQUIRE_MAIN_THREAD();
  delegate_->OnSetDraggableRegions(regions);
}

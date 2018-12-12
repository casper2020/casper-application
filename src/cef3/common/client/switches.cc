// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef3/common/client/switches.h"

// CEF and Chromium support a wide range of command-line switches. This file
// only contains command-line switches specific to the cefclient application.
// View CEF/Chromium documentation or search for *_switches.cc files in the
// Chromium source code to identify other existing command-line switches.
// Below is a partial listing of relevant *_switches.cc files:
//   base/base_switches.cc
//   cef/libcef/common/cef_switches.cc
//   chrome/common/chrome_switches.cc (not all apply)
//   content/public/common/content_switches.cc

const char casper::cef3::common::client::switches::kMultiThreadedMessageLoop[] = "multi-threaded-message-loop";
const char casper::cef3::common::client::switches::kExternalMessagePump[] = "external-message-pump";
const char casper::cef3::common::client::switches::kCachePath[] = "cache-path";
const char casper::cef3::common::client::switches::kUrl[] = "url";
const char casper::cef3::common::client::switches::kOffScreenRenderingEnabled[] = "off-screen-rendering-enabled";
const char casper::cef3::common::client::switches::kOffScreenFrameRate[] = "off-screen-frame-rate";
const char casper::cef3::common::client::switches::kTransparentPaintingEnabled[] = "transparent-painting-enabled";
const char casper::cef3::common::client::switches::kShowUpdateRect[] = "show-update-rect";
const char casper::cef3::common::client::switches::kMouseCursorChangeDisabled[] = "mouse-cursor-change-disabled";
const char casper::cef3::common::client::switches::kRequestContextPerBrowser[] = "request-context-per-browser";
const char casper::cef3::common::client::switches::kRequestContextSharedCache[] = "request-context-shared-cache";
const char casper::cef3::common::client::switches::kRequestContextBlockCookies[] = "request-context-block-cookies";
const char casper::cef3::common::client::switches::kBackgroundColor[] = "background-color";
const char casper::cef3::common::client::switches::kEnableGPU[] = "enable-gpu";
const char casper::cef3::common::client::switches::kFilterURL[] = "filter-url";
const char casper::cef3::common::client::switches::kUseViews[] = "use-views";
const char casper::cef3::common::client::switches::kHideFrame[] = "hide-frame";
const char casper::cef3::common::client::switches::kHideControls[] = "hide-controls";
const char casper::cef3::common::client::switches::kAlwaysOnTop[] = "always-on-top";
const char casper::cef3::common::client::switches::kHideTopMenu[] = "hide-top-menu";
const char casper::cef3::common::client::switches::kWidevineCdmPath[] = "widevine-cdm-path";
const char casper::cef3::common::client::switches::kSslClientCertificate[] = "ssl-client-certificate";
const char casper::cef3::common::client::switches::kCRLSetsPath[] = "crl-sets-path";
const char casper::cef3::common::client::switches::kLoadExtension[] = "load-extension";

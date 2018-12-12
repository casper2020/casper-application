// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef3/common/main.h"

/**
 * @brief
 *
 * @param a_command_line
 */
casper::cef3::common::Main::ProcessType casper::cef3::common::Main::GetProcessType (const CefRefPtr<CefCommandLine> a_command_line)
{
    if ( false == a_command_line->HasSwitch("type") ) {
        return casper::cef3::common::Main::ProcessType::Browser;
    }
    const std::string& process_type = a_command_line->GetSwitchValue("type");
    if ( "renderer" == process_type ) {
        return casper::cef3::common::Main::ProcessType::Renderer;
    }
    return casper::cef3::common::Main::Other;
}


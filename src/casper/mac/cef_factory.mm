/**
 * @file cef_factory.mm
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

#include "casper/mac/client_handler.h"

#pragma mark - ClientHandler

CefRefPtr<casper::cef3::client::common::ClientHandler> casper::cef3::client::common::ClientHandler::Factory (const ::std::string& a_startup_url, const bool& a_is_osr,
                                                                                                             void* a_delegate)
{
    return new casper::cef3::client::mac::ClientHandler(a_startup_url, a_is_osr,
                                                        static_cast<casper::cef3::client::common::ClientHandlerDelegate*>(a_delegate)
    );
}

#pragma mark - ContextMenuHandler

#include "casper/app/cef3/common/context_menu_handler.h"

CefRefPtr<casper::cef3::client::common::ContextMenuHandler> casper::cef3::client::common::ClientHandler::ContextMenuHandlerFactory (CefRefPtr<casper::cef3::client::common::BaseHandler> a_base_handler)
{
    return new casper::app::cef3::common::ContextMenuHandler(a_base_handler);
}

#include "cef3/shared/browser/utils/resource_util.h"

void casper::cef3::client::common::ClientHandler::SetupResourceManager (CefRefPtr<CefResourceManager> resource_manager)

{
    // TODO CW / CASPER
}

#pragma mark - MainMessageLoopExternalPump

#include "cef3/browser/mac/main_message_loop_external_pump.h"

scoped_ptr<casper::cef3::browser::MainMessageLoopExternalPump> casper::cef3::browser::MainMessageLoopExternalPump::Factory ()
{
    return scoped_ptr<casper::cef3::browser::MainMessageLoopExternalPump>(new casper::cef3::browser::MainMessageLoopExternalPumpMAC());
}

#pragma mark - RootWindow

#include "cef3/browser/mac/root_window.h"

scoped_refptr<casper::cef3::browser::RootWindow> casper::cef3::browser::RootWindow::Factory (bool /* a_use_views */)
{
    return new casper::cef3::browser::RootWindowMAC();
}

/**
 * @file main_context.mm
 *
 * Copyright (c) 2011-2022 Cloudware S.A. All rights reserved.
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

#include "cef3/client/mac/main_context.h"

#import <Cocoa/Cocoa.h>

/**
 * @brief Default constructor.
 *
 * @param a_settings
 * @param a_command_line
 */
casper::cef3::client::mac::MainContext::MainContext (const casper::cef3::browser::Settings& a_settings, CefRefPtr<CefCommandLine> a_command_line)
    : casper::cef3::client::common::MainContext(a_settings, a_command_line)
{
    /* empty */
}

/**
 * @brief Destructor
 */
casper::cef3::client::mac::MainContext::~MainContext ()
{
    /* empty */
}

/**
 * @brief Retrieve the logs path.
 *
 * @param o_path
 *
 * @return
 */
bool casper::cef3::client::mac::MainContext::GetLogsPath (std::string& o_path)
{
    NSArray<NSURL*>* urls = [[NSFileManager defaultManager]URLsForDirectory:NSCachesDirectory inDomains:NSUserDomainMask];
    if ( nil == urls || 0 == [urls count] || nil == [urls firstObject] ) {
        return false;
    }
    const NSString* url = [[urls firstObject] absoluteString];
    if ( 0 == [url length] ) {
        return false;
    }
    o_path = std::string([url cStringUsingEncoding:NSUTF8StringEncoding]);
    if ( '/' != o_path[o_path.length()-1] ) {
        o_path += '/';
    }
    return true;
}

/**
 * @brief Retrieve the downloads path.
 *
 * @param o_path
 *
 * @return
 */
bool casper::cef3::client::mac::MainContext::GetDownloadsPath (std::string& o_path)
{
    NSArray<NSURL*>* urls = [[NSFileManager defaultManager]URLsForDirectory:NSDownloadsDirectory inDomains:NSUserDomainMask];
    if ( nil == urls || 0 == [urls count] || nil == [urls firstObject] ) {
        return false;
    }
    const NSString* url = [[urls firstObject] absoluteString];
    if ( 0 == [url length] ) {
        return false;
    }
    o_path = std::string([url cStringUsingEncoding:NSUTF8StringEncoding]);
    if ( '/' != o_path[o_path.length()-1] ) {
        o_path += '/';
    }
    return true;
}

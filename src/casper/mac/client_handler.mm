/**
 * @file client_handler.cc
 *
 * Copyright (c) 2010-2017 Neto Ranito & Seabra LDA. All rights reserved.
 *
 * This file is part of casper.
 *
 * casper is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * casper  is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with casper.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "casper/mac/client_handler.h"

#import <Cocoa/Cocoa.h>

#import <Quartz/Quartz.h> // PDFDocument

#include "include/wrapper/cef_helpers.h" // CEF_REQUIRE_UI_THREAD

#include "cef3/client/common/main_context.h"

#pragma mark -

/**
 * @brief Default constructor.
 *
 * @param a_startup_url
 * @param a_is_osr
 * @param a_delegate
 */
casper::cef3::client::mac::ClientHandler::ClientHandler (const ::std::string& a_startup_url, const bool& a_is_osr,
                                                         casper::cef3::client::common::ClientHandlerDelegate* a_delegate)
    : casper::cef3::client::common::ClientHandler(a_startup_url, a_is_osr, a_delegate)
{
    /* empty */
}

/**
 * @brief Destructor.
 */
casper::cef3::client::mac::ClientHandler::~ClientHandler ()
{
    /* empty */
}

#pragma mark -

/**
 * @brief Retrieve a list of available printers.
 *
 * @param o_printers
 *
 * @return
 */
bool casper::cef3::client::mac::ClientHandler::GetPrinters (std::vector<casper::cef3::client::mac::ClientHandler::Printer> &o_printers)
{
    CFArrayRef printerList;
    Boolean    isRemote = false;

    if ( noErr != PMServerCreatePrinterList(kPMServerLocal, &printerList) || nil == printerList ) {
        return false;
    }
    
    const CFIndex count = CFArrayGetCount(printerList);
    for ( int index = 0; index < count; index++ ) {
        const PMPrinter printer = (PMPrinter)CFArrayGetValueAtIndex(printerList, index);
        const NSString *name    = (__bridge NSString *)(PMPrinterGetName(printer));        
        PMPrinterIsRemote(printer, &isRemote);
        o_printers.push_back({
            /* name_      */ [name cStringUsingEncoding:NSUTF8StringEncoding],
            /* default_   */ ( YES == PMPrinterIsDefault(printer) ),
            /* remote_    */ ( YES == isRemote ),
        });
    }
    return true;
}

/**
 * @brief Print a PDF document
 *
 * @param a_uri
 * @param a_directly
 *
 * @return
 */
bool casper::cef3::client::mac::ClientHandler::PrintPDF (const std::string& a_uri, bool a_directly)
{
    NSURL*       url       = [[NSURL alloc]initWithString:[[NSString alloc]initWithUTF8String:a_uri.c_str()]];
    PDFDocument* document  = [[PDFDocument alloc]initWithURL:url];
    NSPrintInfo* printInfo = [[NSPrintInfo alloc]init];
    
    NSPrintOperation* printOperation = [document printOperationForPrintInfo:printInfo scalingMode:kPDFPrintPageScaleNone autoRotate:NO];
    
    [printOperation setShowsPrintPanel: ( true == a_directly )];
    
    return ( YES == [printOperation runOperation] );
}

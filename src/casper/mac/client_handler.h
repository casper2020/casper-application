/**
 * @file client_handler.h
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

#ifndef CASPER_CEF3_CLIENT_MACOS_CLIENT_HANDLER_H_
#define CASPER_CEF3_CLIENT_MACOS_CLIENT_HANDLER_H_

#pragma once

#include "cef3/client/common/client_handler.h"

namespace casper
{

    namespace cef3
    {

        namespace client
        {
            
            namespace mac
            {
                
                class ClientHandler final : public ::casper::cef3::client::common::ClientHandler
                {
                                   
                protected: // Data Type(s)
                    
                    typedef struct {
                        const std::string name_;
                        const bool        default_;
                        const bool        remote_;
                    } Printer;
                    
                protected: // Data
                    
                    std::vector<Printer>                                    printers_;

                public: // Constructor(s) / Destructor
                    
                    ClientHandler (const ::std::string& a_startup_url, const bool& a_is_osr,
                                   casper::cef3::client::common::ClientHandlerDelegate* a_delegate);
                    virtual ~ClientHandler();
                                       
                private: // Method(s) / Function(s)
                    
                   bool GetPrinters              (std::vector<Printer>& o_printers);
                   bool PrintPDF                 (const std::string& a_uri, bool a_directly);
                   
                private: // Include the default reference counting implementation.
                    
                    IMPLEMENT_REFCOUNTING(ClientHandler);
                    DISALLOW_COPY_AND_ASSIGN(ClientHandler);
                    
                }; // end of class 'ClientHandler'
                
            } // end of namespace 'mac'
            
        } // end of namespace 'client'
        
    } // end of namespace 'cef3'
    
} // end of namespace 'casper'

#endif // CASPER_CEF3_CLIENT_MACOS_CLIENT_HANDLER_H_

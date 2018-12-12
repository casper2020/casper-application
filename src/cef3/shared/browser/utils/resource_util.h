// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CASPER_CEF3_SHARED_BROWSER_UTILS_RESOURCE_UTILS_H_
#define CASPER_CEF3_SHARED_BROWSER_UTILS_RESOURCE_UTILS_H_
#pragma once

#include <string>
#include "include/cef_image.h"
#include "include/cef_stream.h"

namespace casper
{
    
    namespace cef3
    {
        
        namespace shared
        {
            
            namespace browser
            {
                
                namespace utils
                {
                    
                    namespace resource
                    {
                        // Returns the directory containing resource files.
                        bool GetResourceDir(std::string& dir);
                        
                        // Retrieve a resource as a string.
                        bool LoadBinaryResource(const char* resource_name, std::string& resource_data);
                        
                        // Retrieve a resource as a steam reader.
                        CefRefPtr<CefStreamReader> GetBinaryResourceReader(const char* resource_name);

                    } // end of namespace 'resource'
                    
                } // end of namespace 'utils'
                
            } // end of namespace 'browser'
            
        } // end of namespace 'shared'
        
    } // end of namespace 'cef3'
    
} // end of namespace 'casper'

#endif // CASPER_CEF3_SHARED_BROWSER_UTILS_RESOURCE_UTILS_H_

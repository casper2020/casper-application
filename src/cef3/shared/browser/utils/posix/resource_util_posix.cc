// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef3/shared/browser/utils/resource_util.h"

#include "cef3/shared/browser/utils/file_util.h"

#include <stdio.h>

// PRIVATE
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
                        
                        bool ReadFileToString (const char* path, std::string& data)
                        {
                            // Implementation adapted from base/file_util.cc
                            FILE* file = fopen(path, "rb");
                            if (!file)
                                return false;
                            
                            char buf[1 << 16];
                            size_t len;
                            while ((len = fread(buf, 1, sizeof(buf), file)) > 0)
                                data.append(buf, len);
                            fclose(file);
                            
                            return true;
                        }
                    }
                }
            }
        }
    }
}

bool casper::cef3::shared::browser::utils::resource::LoadBinaryResource(const char* resource_name, std::string& resource_data)
{
    std::string path;
    if (!GetResourceDir(path))
        return false;
    
    path.append("/");
    path.append(resource_name);
    
    return ReadFileToString(path.c_str(), resource_data);
}

CefRefPtr<CefStreamReader> casper::cef3::shared::browser::utils::resource::GetBinaryResourceReader(const char* resource_name)
{
    std::string path;
    if (!GetResourceDir(path))
        return nullptr;
    
    path.append("/");
    path.append(resource_name);
    
    if (! casper::cef3::shared::browser::utils::file::FileExists(path.c_str()) )
        return nullptr;
    
    return CefStreamReader::CreateForFile(path);
}


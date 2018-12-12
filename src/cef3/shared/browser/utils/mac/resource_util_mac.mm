// Copyright (c) 2013 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cef3/shared/browser/utils/resource_util.h"

#import <Foundation/Foundation.h>
#include <mach-o/dyld.h>
#include <stdio.h>

#include "include/base/cef_logging.h"

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
                        // Implementation adapted from Chromium's base/mac/foundation_util.mm
                        bool UncachedAmIBundled()
                        {
                            return [[[NSBundle mainBundle] bundlePath] hasSuffix:@".app"];
                        }
                        
                        bool AmIBundled()
                        {
                            static bool am_i_bundled = UncachedAmIBundled();
                            return am_i_bundled;
                        }
                    }
                }
            }
        }
    }
}

// Implementation adapted from Chromium's base/base_path_mac.mm
bool casper::cef3::shared::browser::utils::resource::GetResourceDir(std::string& dir)
{
    // Retrieve the executable directory.
    uint32_t pathSize = 0;
    _NSGetExecutablePath(NULL, &pathSize);
    if (pathSize > 0) {
        dir.resize(pathSize);
        _NSGetExecutablePath(const_cast<char*>(dir.c_str()), &pathSize);
    }
    
    if ( casper::cef3::shared::browser::utils::resource::AmIBundled() ) {
        // Trim executable name up to the last separator.
        std::string::size_type last_separator = dir.find_last_of("/");
        dir.resize(last_separator);
        dir.append("/../Resources");
        return true;
    }
    
    dir.append("/Resources");
    return true;
}


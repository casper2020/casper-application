// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CASPER_CEF3_COMMON_CLIENT_SWITCHES_H_
#define CASPER_CEF3_COMMON_CLIENT_SWITCHES_H_
#pragma once

namespace casper
{
    
    namespace cef3
    {
        
        namespace common
        {
            
            namespace client
            {
                
                namespace switches
                {
                    
                    extern const char kMultiThreadedMessageLoop[];
                    extern const char kExternalMessagePump[];
                    extern const char kCachePath[];
                    extern const char kUrl[];
                    extern const char kOffScreenRenderingEnabled[];
                    extern const char kOffScreenFrameRate[];
                    extern const char kTransparentPaintingEnabled[];
                    extern const char kShowUpdateRect[];
                    extern const char kMouseCursorChangeDisabled[];
                    extern const char kRequestContextPerBrowser[];
                    extern const char kRequestContextSharedCache[];
                    extern const char kRequestContextBlockCookies[];
                    extern const char kBackgroundColor[];
                    extern const char kEnableGPU[];
                    extern const char kFilterURL[];
                    extern const char kUseViews[];
                    extern const char kHideFrame[];
                    extern const char kHideControls[];
                    extern const char kAlwaysOnTop[];
                    extern const char kHideTopMenu[];
                    extern const char kWidevineCdmPath[];
                    extern const char kSslClientCertificate[];
                    extern const char kCRLSetsPath[];
                    extern const char kLoadExtension[];
                    
                } // end of namespace 'switches'
                
            } // end of namespace 'client'
            
        } // end of namespace 'common'
        
    } // end of namespace 'cef3'
    
} // end of namespace 'casper'

#endif // CASPER_CEF3_COMMON_CLIENT_SWITCHES_H_


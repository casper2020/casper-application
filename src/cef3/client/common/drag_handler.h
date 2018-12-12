// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CASPER_CEF3_CLIENT_DRAG_HANDLER_H_
#define CASPER_CEF3_CLIENT_DRAG_HANDLER_H_

#pragma once

#include "include/cef_drag_handler.h" // CefDragHandler

#include "include/base/cef_ref_counted.h" // CefRefPtr

#include "cef3/client/common/base_handler.h"

namespace casper
{
    
    namespace cef3
    {
        
        namespace client
        {
            
            namespace common
            {
                
                class DragHandler : public CefDragHandler
                {
                    
                    IMPLEMENT_REFCOUNTING(DragHandler);
                    
                protected: // Ptrs
                    
                    CefRefPtr<BaseHandler> base_handler_;
                    
                public: // Constructor(s) / Destructor
                    
                    DragHandler (CefRefPtr<BaseHandler> a_base_handler);
                    virtual ~DragHandler ();

                public: // CefDragHandler Method(s) / Function(s)
                    
                    bool OnDragEnter              (CefRefPtr<CefBrowser> browser,
                                                   CefRefPtr<CefDragData> dragData,
                                                   CefDragHandler::DragOperationsMask mask) OVERRIDE;
                    
                    void OnDraggableRegionsChanged (CefRefPtr<CefBrowser> browser,
                                                    const std::vector<CefDraggableRegion>& regions) OVERRIDE;

                protected:
                    
                    void NotifyDraggableRegions (const std::vector<CefDraggableRegion>& regions);
                    
                }; // end of class 'DragHandler'
                
            } // end of namespace 'common'
            
        } // end of namespace 'client'
        
    } // end of namespace 'cef3'
    
}  // end of namespace 'casper'

#endif // CASPER_CEF3_CLIENT_DRAG_HANDLER_H_

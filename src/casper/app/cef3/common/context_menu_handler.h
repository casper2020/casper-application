/**
 * @file context_menu_handler.h
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
#ifndef CASPER_APP_CEF3_COMMON_CONTEXT_MENU_HANDLER_H_
#define CASPER_APP_CEF3_COMMON_CONTEXT_MENU_HANDLER_H_

#pragma once

#include "cef3/client/common/context_menu_handler.h"

namespace casper
{
    
    namespace app
    {
        
        namespace cef3
        {
            
            namespace common
            {
                
                class ContextMenuHandler : public casper::cef3::client::common::ContextMenuHandler
                {
                    
                public: // Data Type(s)
                    
                    class Menu
                    {
                        
                    public: // Data Type(s)
                        
                        class Entry
                        {
                            
                        public: // Enum(s)
                            
                            enum Type {
                                Item,
                                Separator
                            };
                            
                        public: // Const Data
                            
                            const Type         type_;
                            const int          id_;
                            const std::string  label_;
                            
                        public: // Data
                            
                            std::vector<Entry*> children_;
                            
                        public: // Constructor(s) / Destructor
                            
                            Entry (const Type a_type, const int a_id, const std::string& a_label, const std::vector<Entry*>& a_children)
                            : type_(a_type), id_(a_id), label_(a_label)
                            {
                                for  ( const auto child : a_children ) {
                                    children_.push_back(new Entry(child->type_, child->id_, child->label_, child->children_));
                                }
                            }
                            
                            virtual ~Entry ()
                            {
                                for ( auto child : children_ ) {
                                    delete child;
                                }
                            }
                            
                            Entry(Entry const&)            = delete; // Copy construct
                            Entry(Entry&&)                 = delete; // Move construct
                            Entry& operator=(Entry const&) = delete; // Copy assign
                            Entry& operator=(Entry &&)     = delete; // Move assign
                            
                        };
                        
                    public:
                        
                        static void AddItem    (CefRefPtr<CefMenuModel> a_model, const Entry& a_entry);
                        static void AddItems   (CefRefPtr<CefMenuModel> a_model, const std::vector<Entry*>& a_entries);
                        
                    private:
                        
                        static void AddSubMenu (CefRefPtr<CefMenuModel> a_model, const Entry& a_definitions,
                                                CefRefPtr<CefMenuModel> a_sub_menu = nullptr);
                        
                    }; // end of class 'Menu'
                    
                public: // Enum(s)
                    
                    enum ContextMenuIds : int
                    {
                        Home               = 0,
                        Reload             = 100,
                        DeveloperMenu      = 200,
                        ShowDeveloperTools = 201,
                        HideDeveloperTools = 203,
                        InspectElement     = 202,
                        Info               = 300
                    };
                    
                private: // Data
                    
                    std::vector<Menu::Entry*> context_menu_entries_;
                    
                public: // Constructor(s) / Destructor
                    
                    ContextMenuHandler (CefRefPtr<casper::cef3::client::common::BaseHandler> a_base_handler);
                    virtual ~ContextMenuHandler();
                    
                public: // ContextMenuHandler Method(s) / Function(s)
                    
                    void OnBeforeContextMenu  (CefRefPtr<CefBrowser> a_browser, CefRefPtr<CefFrame> a_frame,
                                               CefRefPtr<CefContextMenuParams> a_params, CefRefPtr<CefMenuModel> a_model) override;
                    
                    bool OnContextMenuCommand (CefRefPtr<CefBrowser> a_browser, CefRefPtr<CefFrame> a_frame,
                                               CefRefPtr<CefContextMenuParams> a_params, int a_command_id, CefRunContextMenuCallback::EventFlags a_event_flags) override;
                    
                protected: // Static Method(s) / Function(s)
                    
                    static void OutputTree (const Menu::Entry* a_entry, int a_level);
                    
                }; // end of class 'RequestHandler'
                
            } // end of namespace 'common'
            
        } // end of namespace 'cef3'
        
    } // end of namespace 'app'
    
}  // end of namespace 'casper'

#endif // CASPER_APP_CEF3_COMMON_CONTEXT_MENU_HANDLER_H_

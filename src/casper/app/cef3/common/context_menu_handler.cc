/**
 * @file context_menu_handler.cc
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

#include "casper/app/cef3/common/context_menu_handler.h"

#include "include/wrapper/cef_helpers.h" // CEF_REQUIRE_UI_THREAD

#include "cef3/client/common/main_context.h"

#include "casper/app/cef3/common/helper.h"

#ifdef __APPLE__
#pragma mark - ContextMenuHandler::Menu
#endif

void casper::app::cef3::common::ContextMenuHandler::Menu::AddItem (CefRefPtr<CefMenuModel> a_model,
                                                                   const casper::app::cef3::common::ContextMenuHandler::Menu::Entry& a_entry)
{
    if ( 0 == a_entry.children_.size()  ) {
        if ( casper::app::cef3::common::ContextMenuHandler::Menu::Entry::Type::Separator == a_entry.type_ ) {
            a_model->AddSeparator();
        } else {
            a_model->AddItem(a_entry.id_, a_entry.label_);
        }
    } else {
        AddSubMenu(a_model, a_entry, nullptr);
    }
}

void casper::app::cef3::common::ContextMenuHandler::Menu::AddItems (CefRefPtr<CefMenuModel> a_model,
                                                                    const std::vector<casper::app::cef3::common::ContextMenuHandler::Menu::Entry*> &a_entries)
{
    for ( auto entry : a_entries ) {
        AddItem(a_model, *entry);
    }
}

#ifdef __APPLE__
#pragma mark -
#endif

void casper::app::cef3::common::ContextMenuHandler::Menu::AddSubMenu (CefRefPtr<CefMenuModel> a_model,
                                                                      const casper::app::cef3::common::ContextMenuHandler::Menu::Entry& a_definitions,
                                                                      CefRefPtr<CefMenuModel> a_sub_menu)
{
    CefRefPtr<CefMenuModel> submenu =  a_sub_menu ? a_sub_menu ->AddSubMenu(a_definitions.id_, a_definitions.label_) :  a_model->AddSubMenu(a_definitions.id_, a_definitions.label_);
    for ( auto child : a_definitions.children_ ) {
        switch(child->type_) {
            case casper::app::cef3::common::ContextMenuHandler::Menu::Entry::Type::Item:
                if ( child->children_.size() > 0 ) {
                    CefRefPtr<CefMenuModel> menu = submenu->AddSubMenu(child->id_, child->label_);
                    for ( size_t idx = 0 ; idx < child->children_.size() ; ++idx ) {
                        AddItem(menu, *(child->children_[idx]));
                    }
                } else {
                    AddItem(submenu, *child);
                }
                break;
            case casper::app::cef3::common::ContextMenuHandler::Menu::Entry::Type::Separator:
                submenu->AddSeparator();
                break;
            default:
                break;
        }
        
    }
}

#ifdef __APPLE__
#pragma mark - ContextMenuHandler
#endif

/**
 * @brief Default constructor.
 */
casper::app::cef3::common::ContextMenuHandler::ContextMenuHandler (CefRefPtr<casper::cef3::client::common::BaseHandler> a_base_handler)
    : casper::cef3::client::common::ContextMenuHandler(a_base_handler)
{
    context_menu_entries_ = {
#ifdef DEBUG
        new casper::app::cef3::common::ContextMenuHandler::Menu::Entry(
                /* type_     */ casper::app::cef3::common::ContextMenuHandler::Menu::Entry::Type::Item,
                /* id_       */ casper::app::cef3::common::ContextMenuHandler::ContextMenuIds::Home,
                /* label_    */ "Home",
                /* children_ */ {}
        ),
        new casper::app::cef3::common::ContextMenuHandler::Menu::Entry(
                /* type_     */ casper::app::cef3::common::ContextMenuHandler::Menu::Entry::Type::Separator,
                /* id_       */ -1,
                /* label_    */ "",
                /* children_ */ {}
        ),
#endif
        new casper::app::cef3::common::ContextMenuHandler::Menu::Entry(
                /* type_     */ casper::app::cef3::common::ContextMenuHandler::Menu::Entry::Type::Item,
                /* id_       */ casper::app::cef3::common::ContextMenuHandler::ContextMenuIds::Reload,
                /* label_    */ "Reload",
                /* children_ */ {}
        ),
        new casper::app::cef3::common::ContextMenuHandler::Menu::Entry(
                /* type_     */ casper::app::cef3::common::ContextMenuHandler::Menu::Entry::Type::Separator,
                /* id_       */ -1,
                /* label_    */ "",
                /* children_ */ {}
        ),
        new casper::app::cef3::common::ContextMenuHandler::Menu::Entry(
                /* type_     */ casper::app::cef3::common::ContextMenuHandler::Menu::Entry::Type::Item,
                /* id_       */ casper::app::cef3::common::ContextMenuHandler::ContextMenuIds::DeveloperMenu,
                /* label_    */ "Developer",
                /* children_ */ {
                    new casper::app::cef3::common::ContextMenuHandler::Menu::Entry(
                            /* type_     */ casper::app::cef3::common::ContextMenuHandler::Menu::Entry::Type::Item,
                            /* id_       */ casper::app::cef3::common::ContextMenuHandler::ContextMenuIds::ShowDeveloperTools,
                            /* label_    */ "&Show Developer Tools",
                            /* children_ */ {}
                    ),
                    new casper::app::cef3::common::ContextMenuHandler::Menu::Entry(
                           /* type_     */ casper::app::cef3::common::ContextMenuHandler::Menu::Entry::Type::Item,
                           /* id_       */ casper::app::cef3::common::ContextMenuHandler::ContextMenuIds::HideDeveloperTools,
                           /* label_    */ "&Hide Developer Tools",
                           /* children_ */ {}
                    ),
                    new casper::app::cef3::common::ContextMenuHandler::Menu::Entry(
                            /* type_     */ casper::app::cef3::common::ContextMenuHandler::Menu::Entry::Type::Separator,
                            /* id_       */ -2,
                            /* label_    */ "",
                            /* children_ */ {}
                    ),
                    new casper::app::cef3::common::ContextMenuHandler::Menu::Entry(
                            /* type_     */ casper::app::cef3::common::ContextMenuHandler::Menu::Entry::Type::Item,
                            /* id_       */ casper::app::cef3::common::ContextMenuHandler::ContextMenuIds::InspectElement,
                            /* label_    */ "&Inspect Element",
                            /* children_ */ {}
                    ),
                    new casper::app::cef3::common::ContextMenuHandler::Menu::Entry(
                            /* type_     */ casper::app::cef3::common::ContextMenuHandler::Menu::Entry::Type::Separator,
                            /* id_       */ -3,
                            /* label_    */ "",
                            /* children_ */ {}
                    ),
                    new casper::app::cef3::common::ContextMenuHandler::Menu::Entry(
                            /* type_     */ casper::app::cef3::common::ContextMenuHandler::Menu::Entry::Type::Item,
                            /* id_       */ casper::app::cef3::common::ContextMenuHandler::ContextMenuIds::Info,
                            /* label_    */ "Info",
                            /* children_ */ {}
                    )
            }
        )
    };
}

/**
 * @brief Destructor.
 */
casper::app::cef3::common::ContextMenuHandler::~ContextMenuHandler ()
{
    for ( auto entry : context_menu_entries_ ) {
        delete entry;
    }
    context_menu_entries_.clear();
}

/**
 * @brief
 *
 * @param a_browser
 * @param a_frame
 * @param a_params
 * @param a_model
 */
void casper::app::cef3::common::ContextMenuHandler::OnBeforeContextMenu (CefRefPtr<CefBrowser> a_browser, CefRefPtr<CefFrame> /* a_frame */,
                                                                         CefRefPtr<CefContextMenuParams> a_params, CefRefPtr<CefMenuModel> a_model)
{
    CEF_REQUIRE_UI_THREAD();
    
    if ( ( a_params->GetTypeFlags() & (CM_TYPEFLAG_PAGE | CM_TYPEFLAG_FRAME) ) != 0 ) {
        
        while ( a_model->GetCount() ) {
            a_model->RemoveAt(0);
        }
        
        for ( auto entry : context_menu_entries_ ) {
            OutputTree(entry, 0);
        }
        
        
        if ( context_menu_entries_.size() > 0 ) {
            casper::app::cef3::common::ContextMenuHandler::Menu::AddItems(a_model, context_menu_entries_);
        }
        
    }
    
    if ( nullptr != base_handler_->delegate_ptr_ ) {
        // TODO CW base_handler_->delegate_ptr_->OnBeforeContextMenu(a_model);
    }
}

/**
 * @brief
 *
 * @param a_browser
 * @param a_frame
 * @param a_params
 * @param a_command_id
 * @param a_event_flags
 */
bool casper::app::cef3::common::ContextMenuHandler::OnContextMenuCommand (CefRefPtr<CefBrowser> a_browser, CefRefPtr<CefFrame> /* a_frame */,
                                                                          CefRefPtr<CefContextMenuParams> a_params, int a_command_id,
                                                                          CefRunContextMenuCallback::EventFlags /* a_event_flags */)
{
    CEF_REQUIRE_UI_THREAD();
    
    auto context = casper::cef3::client::common::MainContext::MainContext::Get();

    switch (static_cast<casper::app::cef3::common::ContextMenuHandler::ContextMenuIds>(a_command_id)) {
        case casper::app::cef3::common::ContextMenuHandler::ContextMenuIds::Home:
            a_browser->GetMainFrame()->LoadURL(context->settings().application_.main_url_);
            return true;
        case casper::app::cef3::common::ContextMenuHandler::ContextMenuIds::Reload:
            a_browser->Reload();
            return true;
        case casper::app::cef3::common::ContextMenuHandler::ContextMenuIds::ShowDeveloperTools:
            (void)casper::app::cef3::common::Helper::ShowDeveloperTools(a_browser);
            return true;
        case casper::app::cef3::common::ContextMenuHandler::ContextMenuIds::HideDeveloperTools:
            (void)casper::app::cef3::common::Helper::HideDeveloperTools(a_browser);
            return true;
        case casper::app::cef3::common::ContextMenuHandler::ContextMenuIds::InspectElement:
            (void)casper::app::cef3::common::Helper::ShowDeveloperTools(a_browser, CefPoint(a_params->GetXCoord(), a_params->GetYCoord()));
            return true;
            // TODO CW
//        case casper::app::cef3::common::ContextMenuHandler::ContextMenuIds::Info:
//            (void)ShowDeveloperInformation(a_browser);
//            return true;
        default:
            return false;
    }
}

#ifdef __APPLE__
#pragma mark -
#endif

/**
 * @brief Output a menu tree.
 *
 * @param a_entry
 * @param a_level
 */
void casper::app::cef3::common::ContextMenuHandler::OutputTree (const casper::app::cef3::common::ContextMenuHandler::Menu::Entry* a_entry, int a_level)
{
    fprintf(stderr, "%*c", a_level, 'x');
    if ( casper::app::cef3::common::ContextMenuHandler::Menu::Entry::Type::Separator == a_entry->type_ ) {
        fprintf(stderr, "%c\n", '-');
    } else {
        fprintf(stderr, "%*c", a_level, 'x');
        fprintf(stderr, "%3d: %s\n", a_entry->id_, a_entry->label_.c_str());
        if ( a_entry->children_.size() > 0 ) {
            for ( auto child : a_entry->children_ ) {
                OutputTree(child, a_level + 2);
            }
        }
    }
}
//
//void ShowDeveloperInformation (CefRefPtr<CefBrowser> a_browser);
//
///**
// * @brief Show developer information.
// *
// * @param a_browser
// */
//void casper::cef3::client::mac::ClientHandler::ShowDeveloperInformation (CefRefPtr<CefBrowser> a_browser)
//{
//    auto context = casper::cef3::client::common::MainContext::MainContext::Get();
//    
//    std::stringstream ss;
//    
//    ss << "<html><head><title>Developer Information</title></head>"
//    "<body bgcolor=\"white\">"
//    "<h3>User Agent: </h3>" << context->settings().application_.user_agent_ << "</br>"
//    "<h3>Printers</h3>";
//    ss << "<table border=1><tr><th>Name</th><th>Default</th><th>Remote</th></tr>";
//    
//    printers_.clear();
//    if ( true == GetPrinters(printers_) ) {
//        for ( auto printer : printers_ ) {
//            ss << "<tr>"
//            "<td>" << printer.name_ << "</td>"
//            "<td>" << ( printer.default_ ? "Yes" : "No" ) << "</td>"
//            "<td>" << ( printer.remote_ ? "Yes" : "No" ) << "</td>"
//            "</tr>";
//        }
//    }
//    ss << "</table>";
//    
//    ss << "</body></html>";
//    
//    casper::app::cef3::common::Helper::ShowInfo("text/html", ss.str(), /* TODO is_osr() */ false);
//}

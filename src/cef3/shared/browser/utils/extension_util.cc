// Copyright (c) 2017 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef3/shared/browser/utils/extension_util.h"

#include "include/base/cef_bind.h"
#include "include/cef_parser.h"
#include "include/cef_path_util.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/base/cef_cxx17_backports.h"

#include "cef3/shared/browser/utils/file_util.h"
#include "cef3/shared/browser/utils/resource_util.h"

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
                    
                    namespace extension
                    {
                        
                        std::string GetResourcesPath() {
                            CefString resources_dir;
                            if (CefGetPath(PK_DIR_RESOURCES, resources_dir) && !resources_dir.empty()) {
                                return resources_dir.ToString() + casper::cef3::shared::browser::utils::file::kPathSep;
                            }
                            return std::string();
                        }
                        
                        // Internal extension paths may be prefixed with PK_DIR_RESOURCES and always
                        // use forward slash as path separator.
                        std::string GetInternalPath(const std::string& extension_path) {
                            const std::string& resources_path = GetResourcesPath();
                            std::string internal_path;
                            if (!resources_path.empty() && extension_path.find(resources_path) == 0U) {
                                internal_path = extension_path.substr(resources_path.size());
                            } else {
                                internal_path = extension_path;
                            }
                            
#if defined(OS_WIN)
                            // Normalize path separators.
                            std::replace(internal_path.begin(), internal_path.end(), '\\', '/');
#endif
                            
                            return internal_path;
                        }
                        
                        using ManifestCallback = base::OnceCallback<void(CefRefPtr<CefDictionaryValue> /*manifest*/)>;
                        void RunManifestCallback (ManifestCallback callback, CefRefPtr<CefDictionaryValue> manifest)
                        {
                            if (!CefCurrentlyOn(TID_UI)) {
                                // Execute on the browser UI thread.
                                CefPostTask(TID_UI, base::BindOnce(RunManifestCallback, std::move(callback), manifest));
                                return;
                            }
                            std::move(callback).Run(manifest);
                        }
                        
                        // Asynchronously reads the manifest and executes |callback| on the UI thread.
                        void GetInternalManifest (const std::string& extension_path, ManifestCallback callback)
                        {
                            if ( !CefCurrentlyOn(TID_FILE_USER_BLOCKING) ) {
                                // Execute on the browser FILE thread.
                                CefPostTask(TID_FILE_USER_BLOCKING,
                                            base::BindOnce(GetInternalManifest, extension_path, std::move(callback)));
                                return;
                            }
                            
                            const std::string& manifest_path = GetInternalExtensionResourcePath(casper::cef3::shared::browser::utils::file::JoinPath(extension_path, "manifest.json"));
                            std::string manifest_contents;
                            if ( ! casper::cef3::shared::browser::utils::resource::LoadBinaryResource(manifest_path.c_str(), manifest_contents) || manifest_contents.empty() ) {
                                LOG(ERROR) << "Failed to load manifest from " << manifest_path;
                                RunManifestCallback(std::move(callback), nullptr);
                                return;
                            }
                            
                            CefString error_msg;
                            CefRefPtr<CefValue> value = CefParseJSONAndReturnError(manifest_contents, JSON_PARSER_RFC, error_msg);
                            if (!value || value->GetType() != VTYPE_DICTIONARY) {
                                if (error_msg.empty())
                                    error_msg = "Incorrectly formatted dictionary contents.";
                                LOG(ERROR) << "Failed to parse manifest from " << manifest_path << "; "
                                << error_msg.ToString();
                                RunManifestCallback(std::move(callback), nullptr);
                                return;
                            }
                            
                            RunManifestCallback(std::move(callback), value->GetDictionary());
                        }
                        
                        void LoadExtensionWithManifest(CefRefPtr<CefRequestContext> request_context,
                                                       const std::string& extension_path,
                                                       CefRefPtr<CefExtensionHandler> handler,
                                                       CefRefPtr<CefDictionaryValue> manifest) {
                            CEF_REQUIRE_UI_THREAD();
                            
                            // Load the extension internally. Resource requests will be handled via
                            // AddInternalExtensionToResourceManager.
                            request_context->LoadExtension(extension_path, manifest, handler);
                        }
                        
                    }
                }
            }
        }
    }
}

#ifdef __APPLE__
#pragma mark -
#endif

bool casper::cef3::shared::browser::utils::extension::IsInternalExtension (const std::string& a_extension_path)
{
    // List of internally handled extensions.
    static const char* extensions[] = {"set_page_color"};
    const std::string& internal_path = GetInternalPath(a_extension_path);
    for ( size_t i = 0; i < base::size(extensions); ++i ) {
        // Exact match or first directory component.
        const std::string& extension = extensions[i];
        if ( internal_path == extension || internal_path.find(extension + '/') == 0 ) {
            return true;
        }
    }
    return false;
}

std::string casper::cef3::shared::browser::utils::extension::GetInternalExtensionResourcePath (const std::string& a_extension_path)
{
    return "extensions/" + GetInternalPath(a_extension_path);
}

std::string casper::cef3::shared::browser::utils::extension::GetExtensionResourcePath (const std::string& a_extension_path, bool* o_internal) {
    const bool is_internal = IsInternalExtension(a_extension_path);
    if (o_internal)
        *o_internal = is_internal;
    if (is_internal)
        return GetInternalExtensionResourcePath(a_extension_path);
    return a_extension_path;
}

bool casper::cef3::shared::browser::utils::extension::GetExtensionResourceContents (const std::string& a_extension_path, std::string& o_contents)
{
        CEF_REQUIRE_FILE_USER_BLOCKING_THREAD();
    
    if (IsInternalExtension(a_extension_path)) {
        const std::string& contents_path =
        GetInternalExtensionResourcePath(a_extension_path);
        return casper::cef3::shared::browser::utils::resource::LoadBinaryResource(contents_path.c_str(), o_contents);
    }
    
    return casper::cef3::shared::browser::utils::file::ReadFileToString(a_extension_path, &o_contents);
}

void casper::cef3::shared::browser::utils::extension::LoadExtension (CefRefPtr<CefRequestContext> request_context, const std::string& extension_path,
                                                                     CefRefPtr<CefExtensionHandler> handler)
{
    if ( !CefCurrentlyOn(TID_UI) ) {
        // Execute on the browser UI thread.
        CefPostTask(TID_UI, base::BindOnce(LoadExtension, request_context,
                                           extension_path, handler));
        return;
    }
    
    if ( IsInternalExtension(extension_path) ) {
        // Read the extension manifest and load asynchronously.
        GetInternalManifest(extension_path,
                            base::BindOnce(LoadExtensionWithManifest, request_context,
                                           extension_path, handler));
    } else {
        // Load the extension from disk.
        request_context->LoadExtension(extension_path, nullptr, handler);
    }
}

void casper::cef3::shared::browser::utils::extension::AddInternalExtensionToResourceManager (CefRefPtr<CefExtension> a_extension, CefRefPtr<CefResourceManager> a_resource_manager)
{
    DCHECK(IsInternalExtension(a_extension->GetPath()));
    
    if (!CefCurrentlyOn(TID_IO)) {
        // Execute on the browser IO thread.
        CefPostTask(TID_IO, base::BindOnce(AddInternalExtensionToResourceManager, a_extension, a_resource_manager));
        return;
    }
    
    const std::string& origin        = GetExtensionOrigin(a_extension->GetIdentifier());
    const std::string& resource_path = GetInternalExtensionResourcePath(a_extension->GetPath());
    
    // Add provider for bundled resource files.
#if defined(OS_WIN)
    // Read resources from the binary.
    resource_manager->AddProvider(CreateBinaryResourceProvider(origin, resource_path), 50, std::string());
#elif defined(OS_POSIX)
    // Read resources from a directory on disk.
    std::string resource_dir;
    if ( casper::cef3::shared::browser::utils::resource::GetResourceDir(resource_dir) ) {
        resource_dir += "/" + resource_path;
        a_resource_manager->AddDirectoryProvider(origin, resource_dir, 50,
                                               std::string());
    }
#endif
}

std::string casper::cef3::shared::browser::utils::extension::GetExtensionOrigin (const std::string& a_extension_id)
{
    return "chrome-extension://" + a_extension_id + "/";
}

std::string casper::cef3::shared::browser::utils::extension::GetExtensionURL (CefRefPtr<CefExtension> a_extension)
{
    CefRefPtr<CefDictionaryValue> browser_action =
    a_extension->GetManifest()->GetDictionary("browser_action");
    if (browser_action) {
        const std::string& default_popup =
        browser_action->GetString("default_popup");
        if (!default_popup.empty())
            return GetExtensionOrigin(a_extension->GetIdentifier()) + default_popup;
    }
    
    return std::string();
}

std::string casper::cef3::shared::browser::utils::extension::GetExtensionIconPath (CefRefPtr<CefExtension> a_extension, bool* o_internal)
{
    CefRefPtr<CefDictionaryValue> browser_action =
    a_extension->GetManifest()->GetDictionary("browser_action");
    if (browser_action) {
        const std::string& default_icon = browser_action->GetString("default_icon");
        if (!default_icon.empty()) {
            return GetExtensionResourcePath(
                                            casper::cef3::shared::browser::utils::file::JoinPath(a_extension->GetPath(), default_icon), o_internal);
        }
    }
    
    return std::string();
}

// Copyright (c) 2012 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "include/cef_app.h"
#include "include/wrapper/cef_library_loader.h"

#include "cef3/common/main.h"

#include "cef3/helper/common/other_app.h"
#include "cef3/helper/common/renderer_app.h"

// When generating projects with CMake the CEF_USE_SANDBOX value will be defined
// automatically. Pass -DUSE_SANDBOX=OFF to the CMake command-line to disable
// use of the sandbox.
#if defined(CEF_USE_SANDBOX)
    #include "include/cef_sandbox_mac.h"
#endif

// Entry point function for sub-processes.
int main(int argc, char* argv[])
{
    
#if defined(CEF_USE_SANDBOX)
    // Initialize the macOS sandbox for this helper process.
    CefScopedSandboxContext sandbox_context;
    if ( false == sandbox_context.Initialize(argc, argv) ) {
        return 1;
    }
#endif
    
    // Load the CEF framework library at runtime instead of linking directly
    // as required by the macOS sandbox implementation.
    CefScopedLibraryLoader library_loader;
    if ( false == library_loader.LoadInHelper() ) {
        return 1;
    }
    
    CefMainArgs main_args(argc, argv);
    
    // Parse command-line arguments.
    CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
    command_line->InitFromArgv(argc, argv);
    
    // Create a ClientApp of the correct type.
    CefRefPtr<CefApp> app;
    
    const ::casper::cef3::common::Main::ProcessType process_type = ::casper::cef3::common::Main::GetProcessType(command_line);
    if ( ::casper::cef3::common::Main::ProcessType::Renderer == process_type ) {
        app = new ::casper::cef3::helper::common::RendererApp();
    } else if ( ::casper::cef3::common::Main::ProcessType::Other == process_type ) {
        app = new ::casper::cef3::helper::common::OtherApp();
    }
    
    return CefExecuteProcess(main_args, app, NULL);
}

// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef3/browser/main_message_loop_std.h"

#include "include/cef_app.h" // CefRunMessageLoop, CefQuitMessageLoop

/**
 * @brief Default constructor.
 */
casper::cef3::browser::MainMessageLoopStd::MainMessageLoopStd ()
{
    /* empty */
}

/**
 * @brief
 */
int casper::cef3::browser::MainMessageLoopStd::MainMessageLoopStd::Run ()
{
    CefRunMessageLoop();
    return 0;
}

/**
 * @brief
 */
void casper::cef3::browser::MainMessageLoopStd::MainMessageLoopStd::Quit ()
{
    CefQuitMessageLoop();
}

/**
 * @brief
 *
 * @param task
 */
void casper::cef3::browser::MainMessageLoopStd::MainMessageLoopStd::PostTask(CefRefPtr<CefTask> a_task)
{
    CefPostTask(TID_UI, a_task);
}

/**
 * @brief
 */
bool casper::cef3::browser::MainMessageLoopStd::MainMessageLoopStd::RunsTasksOnCurrentThread() const
{
    return CefCurrentlyOn(TID_UI);
}

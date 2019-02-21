// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef3/browser/main_message_loop_std.h"

#include "include/cef_app.h" // CefRunMessageLoop, CefQuitMessageLoop

/**
 * @brief Default constructor.
 *
 * @parm a_one_shot_quit_callback
 */
casper::cef3::browser::MainMessageLoopStd::MainMessageLoopStd (std::function<void()> a_one_shot_quit_callback)
{
    one_shot_quit_callback_ = a_one_shot_quit_callback;
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
    if ( nullptr != one_shot_quit_callback_ ) {
        one_shot_quit_callback_();
        one_shot_quit_callback_ = nullptr;
    } else {
        CefQuitMessageLoop();
    }
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

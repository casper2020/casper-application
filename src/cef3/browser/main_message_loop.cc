// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef3/browser/main_message_loop.h"

#include "include/wrapper/cef_closure_task.h" // CefCreateClosureTask

casper::cef3::browser::MainMessageLoop* s_main_message_loop_ = NULL;

/**
 * @brief Default constructor.
 */
casper::cef3::browser::MainMessageLoop::MainMessageLoop ()
{
    DCHECK(!s_main_message_loop_);
    s_main_message_loop_ = this;
}
    
/**
 * @brief Destructor.
 */
casper::cef3::browser::MainMessageLoop::~MainMessageLoop ()
{
    s_main_message_loop_ = NULL;
}

/**
 * @return MainMessageLoop instance.
 */
casper::cef3::browser::MainMessageLoop* casper::cef3::browser::MainMessageLoop::Get ()
{
    DCHECK(s_main_message_loop_);
    return s_main_message_loop_;
}

/**
 * @brief
 *
 * @param a_closure
 */
void casper::cef3::browser::MainMessageLoop::PostClosure (const base::Closure& a_closure)
{
    PostTask(CefCreateClosureTask(a_closure));
}

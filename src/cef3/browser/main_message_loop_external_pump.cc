// Copyright (c) 2016 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef3/browser/main_message_loop_external_pump.h"

#include <climits>

#include "include/cef_app.h"
#include "include/wrapper/cef_helpers.h"

#include "cef3/browser/main_message_loop.h"

// Special timer delay placeholder value. Intentionally 32-bit for Windows and
// OS X platform API compatibility.
const int32 kTimerDelayPlaceholder = INT_MAX;

// The maximum number of milliseconds we're willing to wait between calls to
// DoWork().
const int64 kMaxTimerDelay = 1000 / 30;  // 30fps

casper::cef3::browser::MainMessageLoopExternalPump* g_external_message_pump = NULL;

casper::cef3::browser::MainMessageLoopExternalPump::MainMessageLoopExternalPump ()
: is_active_(false), reentrancy_detected_(false) {
    DCHECK(!g_external_message_pump);
    g_external_message_pump = this;
}

casper::cef3::browser::MainMessageLoopExternalPump::~MainMessageLoopExternalPump ()
{
    g_external_message_pump = NULL;
}

casper::cef3::browser::MainMessageLoopExternalPump* casper::cef3::browser::MainMessageLoopExternalPump::Get ()
{
    return g_external_message_pump;
}

void casper::cef3::browser::MainMessageLoopExternalPump::OnScheduleWork(int64 delay_ms)
{
    REQUIRE_MAIN_THREAD();
    
    if (delay_ms == kTimerDelayPlaceholder && IsTimerPending()) {
        // Don't set the maximum timer requested from DoWork() if a timer event is
        // currently pending.
        return;
    }
    
    KillTimer();
    
    if (delay_ms <= 0) {
        // Execute the work immediately.
        DoWork();
    } else {
        // Never wait longer than the maximum allowed time.
        if (delay_ms > kMaxTimerDelay)
            delay_ms = kMaxTimerDelay;
        
        // Results in call to OnTimerTimeout() after the specified delay.
        SetTimer(delay_ms);
    }
}

void casper::cef3::browser::MainMessageLoopExternalPump::OnTimerTimeout ()
{
    REQUIRE_MAIN_THREAD();
    
    KillTimer();
    DoWork();
}

void casper::cef3::browser::MainMessageLoopExternalPump::DoWork ()
{
    const bool was_reentrant = PerformMessageLoopWork();
    if (was_reentrant) {
        // Execute the remaining work as soon as possible.
        OnScheduleMessagePumpWork(0);
    } else if (!IsTimerPending()) {
        // Schedule a timer event at the maximum allowed time. This may be dropped
        // in OnScheduleWork() if another timer event is already in-flight.
        OnScheduleMessagePumpWork(kTimerDelayPlaceholder);
    }
}

bool casper::cef3::browser::MainMessageLoopExternalPump::PerformMessageLoopWork ()
{
    if (is_active_) {
        // When CefDoMessageLoopWork() is called there may be various callbacks
        // (such as paint and IPC messages) that result in additional calls to this
        // method. If re-entrancy is detected we must repost a request again to the
        // owner thread to ensure that the discarded call is executed in the future.
        reentrancy_detected_ = true;
        return false;
    }
    
    reentrancy_detected_ = false;
    
    is_active_ = true;
    CefDoMessageLoopWork();
    is_active_ = false;
    
    // |reentrancy_detected_| may have changed due to re-entrant calls to this
    // method.
    return reentrancy_detected_;
}

// Copyright (c) 2016 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

#include "cef3/browser/mac/main_message_loop_external_pump.h"

#pragma mark - MainMessageLoopExternalPumpMAC

casper::cef3::browser::MainMessageLoopExternalPumpMAC::MainMessageLoopExternalPumpMAC ()
    : owner_thread_([[NSThread currentThread] retain]), timer_(nil)
{
    event_handler_ = [[[EventHandler alloc] initWithPump:this] retain];
}
    
casper::cef3::browser::MainMessageLoopExternalPumpMAC::~MainMessageLoopExternalPumpMAC ()
{
    KillTimer();
    [owner_thread_ release];
    [event_handler_ release];
}
    
void casper::cef3::browser::MainMessageLoopExternalPumpMAC::Quit ()
{
    [NSApp stop:nil];
}
    
int casper::cef3::browser::MainMessageLoopExternalPumpMAC::Run ()
{
    // Run the message loop.
    [NSApp run];
    
    KillTimer();
    
    // We need to run the message pump until it is idle. However we don't have
    // that information here so we run the message loop "for a while".
    for (int i = 0; i < 10; ++i) {
        // Let default runloop observers run.
        CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.001, 1);
        
        // Do some work.
        CefDoMessageLoopWork();
        
        // Sleep to allow the CEF proc to do work.
        [NSThread sleepForTimeInterval:0.05];
    }
    
    return 0;
}
    
void casper::cef3::browser::MainMessageLoopExternalPumpMAC::OnScheduleMessagePumpWork (int64 delay_ms)
{
    // This method may be called on any thread.
    NSNumber* number = [NSNumber numberWithInt:static_cast<int>(delay_ms)];
    [event_handler_ performSelector:@selector(scheduleWork:)
                           onThread:owner_thread_
                         withObject:number
                      waitUntilDone:NO];
}

void casper::cef3::browser::MainMessageLoopExternalPumpMAC::HandleScheduleWork (int64 delay_ms)
{
    OnScheduleWork(delay_ms);
}

void casper::cef3::browser::MainMessageLoopExternalPumpMAC::HandleTimerTimeout ()
{
    OnTimerTimeout();
}

void casper::cef3::browser::MainMessageLoopExternalPumpMAC::SetTimer (int64 delay_ms)
{
    DCHECK_GT(delay_ms, 0);
    DCHECK(!timer_);
    
    const double delay_s = static_cast<double>(delay_ms) / 1000.0;
    timer_ = [[NSTimer timerWithTimeInterval:delay_s
                                      target:event_handler_
                                    selector:@selector(timerTimeout:)
                                    userInfo:nil
                                     repeats:NO] retain];
    
    // Add the timer to default and tracking runloop modes.
    NSRunLoop* owner_runloop = [NSRunLoop currentRunLoop];
    [owner_runloop addTimer:timer_ forMode:NSRunLoopCommonModes];
    [owner_runloop addTimer:timer_ forMode:NSEventTrackingRunLoopMode];
}

void casper::cef3::browser::MainMessageLoopExternalPumpMAC::KillTimer ()
{
    if (timer_ != nil) {
        [timer_ invalidate];
        [timer_ release];
        timer_ = nil;
    }
}

#pragma mark - EventHandler

@implementation EventHandler

- (id)initWithPump:(casper::cef3::browser::MainMessageLoopExternalPumpMAC*)pump {
    if (self = [super init]) {
        pump_ = pump;
    }
    return self;
}

- (void)scheduleWork:(NSNumber*)delay_ms {
    pump_->HandleScheduleWork([delay_ms integerValue]);
}

- (void)timerTimeout:(id)obj {
    pump_->HandleTimerTimeout();
}

@end

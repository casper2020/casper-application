// Copyright (c) 2016 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

#include "include/cef_app.h"
#include "cef3/browser/main_message_loop_external_pump.h"

@class EventHandler;

#pragma mark - MainMessageLoopExternalPumpMAC

namespace casper
{
    
    namespace cef3
    {
        
        namespace browser
        {
            
            class MainMessageLoopExternalPumpMAC : public casper::cef3::browser::MainMessageLoopExternalPump
            {
                
            public:
                
                MainMessageLoopExternalPumpMAC();
                ~MainMessageLoopExternalPumpMAC();
                
                // MainMessageLoopStd methods:
                void Quit () override;
                int  Run  () override;
                
                // MainMessageLoopExternalPump methods:
                void OnScheduleMessagePumpWork(int64 delay_ms) override;
                
                // Internal methods used for processing the event callbacks. They are public
                // for simplicity but should not be used directly.
                void HandleScheduleWork(int64 delay_ms);
                void HandleTimerTimeout();
                
            protected:
                
                // MainMessageLoopExternalPump methods:
                void SetTimer(int64 delay_ms) override;
                void KillTimer() override;
                bool IsTimerPending() override { return timer_ != nil; }
                
            private:
                // Owner thread that will run events.
                NSThread* owner_thread_;
                
                // Pending work timer.
                NSTimer* timer_;
                
                // Used to handle event callbacks on the owner thread.
                EventHandler* event_handler_;
                
            }; // end of class 'MainMessageLoopExternalPumpMAC'
            
        } // end of namespace 'browser'
        
    } // end of namespace 'cef3'
    
} // end of namespace 'casper'

#pragma mark - EventHandler

// Object that handles event callbacks on the owner thread.
@interface EventHandler : NSObject {
@private
    casper::cef3::browser::MainMessageLoopExternalPumpMAC* pump_;
}

- (id)initWithPump:(casper::cef3::browser::MainMessageLoopExternalPumpMAC*)pump;
- (void)scheduleWork:(NSNumber*)delay_ms;
- (void)timerTimeout:(id)obj;
@end




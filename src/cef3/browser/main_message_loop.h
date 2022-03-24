// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CASPER_CEF3_BROWSER_MAIN_MESSAGE_LOOP_H_
#define CASPER_CEF3_BROWSER_MAIN_MESSAGE_LOOP_H_

#pragma once

#include "include/base/cef_bind.h"
#include "include/base/cef_scoped_refptr.h"
#include "include/base/cef_callback_forward.h"
#include "include/cef_task.h"

namespace casper
{
    
    namespace cef3
    {
        
        namespace browser
        {
            
            // Represents the message loop running on the main application thread in the
            // browser process. This will be the same as the CEF UI thread on Linux, OS X
            // and Windows when not using multi-threaded message loop mode. The methods of
            // this class are thread-safe unless otherwise indicated.
            class MainMessageLoop
            {
                
            public:
                
                // Returns the singleton instance of this object.
                static MainMessageLoop* Get();
                
            public:
                
                // Run the message loop. The thread that this method is called on will be
                // considered the main thread. This blocks until Quit() is called.
                virtual int Run() = 0;
                
                // Quit the message loop.
                virtual void Quit() = 0;
                
                // Post a task for execution on the main message loop.
                virtual void PostTask(CefRefPtr<CefTask> task) = 0;
                
                // Returns true if this message loop runs tasks on the current thread.
                virtual bool RunsTasksOnCurrentThread() const = 0;
                
            public:
                
                // Post a closure for execution on the main message loop.
                void PostClosure(base::OnceClosure closure);
                // Post a closure for execution on the main message loop.
                void PostClosure(const base::RepeatingClosure& closure);

            protected:
                
                // Only allow deletion via scoped_ptr.
                friend struct std::default_delete<MainMessageLoop>;
                
                MainMessageLoop();
                virtual ~MainMessageLoop();
                
            private:
                
                DISALLOW_COPY_AND_ASSIGN(MainMessageLoop);
                
            }; // end of class 'MainMessageLoop'
            
#define CURRENTLY_ON_MAIN_THREAD() \
    casper::cef3::browser::MainMessageLoop::Get()->RunsTasksOnCurrentThread()
            
#define REQUIRE_MAIN_THREAD() DCHECK(CURRENTLY_ON_MAIN_THREAD())
            
#define MAIN_POST_TASK(task) \
    casper::cef3::browser::MainMessageLoop::Get()->PostTask(task)
            
#define MAIN_POST_CLOSURE(closure) \
    casper::cef3::browser::MainMessageLoop::Get()->PostClosure(closure)
            
            // Use this struct in conjuction with RefCountedThreadSafe to ensure that an
            // object is deleted on the main thread. For example:
            //
            // class Foo : public base::RefCountedThreadSafe<Foo, DeleteOnMainThread> {
            //  public:
            //   Foo();
            //   void DoSomething();
            //
            //  private:
            //   // Allow deletion via scoped_refptr only.
            //   friend struct DeleteOnMainThread;
            //   friend class base::RefCountedThreadSafe<Foo, DeleteOnMainThread>;
            //
            //   virtual ~Foo() {}
            // };
            //
            // base::scoped_refptr<Foo> foo = new Foo();
            // foo->DoSomething();
            // foo = NULL;  // Deletion of |foo| will occur on the main thread.
            //
            
            struct DeleteOnMainThread {
                template <typename T>
                static void Destruct(const T* x) {
                    if ( CURRENTLY_ON_MAIN_THREAD() ) {
                        delete x;
                    } else {
                        casper::cef3::browser::MainMessageLoop::Get()->PostClosure(base::BindOnce(&DeleteOnMainThread::Destruct<T>, base::Unretained(x)));
                    }
                }
            }; // end of struct 'DeleteOnMainThread'
            
        } // end of namespace 'browser'
        
    } // end of namespace 'cef3'
    
} // end of namespace 'casper'

#endif  // CEF_TESTS_SHARED_BROWSER_MAIN_MESSAGE_LOOP_H_

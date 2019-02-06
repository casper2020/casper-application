/**
 * @file singleton.h - NOT Thread safe singleton
 *
 * Based on code originally developed for NDrive S.A.
 *
 * Copyright (c) 2011-2018 Cloudware S.A. All rights reserved.
 *
 * This file is part of casper-osal.
 *
 * casper-osal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * casper-osal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with osal.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CASPER_SINGLETON_H_
#define CASPER_SINGLETON_H_
#pragma once

#include <type_traits>

namespace cc
{
    
    template <typename C> class Initializer
    {
        
    protected: // Refs
        
        C& instance_;
        
    public: // Constructor(s) / Destructor
        
        Initializer (C& a_instance);
        virtual ~Initializer ();
        
    };
    
    template <typename C> Initializer<C>::Initializer (C& a_instance)
        : instance_(a_instance)
    {
        
    }
    
    template <typename C> Initializer<C>::~Initializer ()
    {
        /* empty */
    }
    
    template <typename T, typename I, typename = std::enable_if<std::is_base_of<Initializer<T>, I>::value>> class Singleton
    {
        
    private: // Static Data
        
        static T* instance_;
        static I* initializer_;
        
    protected: // constructor
        
        Singleton ()
        {
            /* empty */
        }
                
        ~Singleton()
        {
            /* empty */
        }
                
    private: // operators
        
        T& operator = (const T& a_singleton)
        {
            if ( &a_singleton != this ) {
                instance_ = a_singleton.instance_;
            }
            return this;
        }
        
    public: // inline method(s) / function(s) declaration
        
        static T& GetInstance () __attribute__ ((visibility ("default")))
        {
            if ( nullptr == Singleton<T,I>::instance_ ) {
                Singleton<T,I>::instance_    = new T();
                Singleton<T,I>::initializer_ = new I(*Singleton<T,I>::instance_);
            }
            return *Singleton<T,I>::instance_;
        }
        
        static void Destroy ()
        {
            if ( nullptr != Singleton<T,I>::initializer_ ) {
                delete initializer_;
                initializer_ = nullptr;
            }
            if ( nullptr != Singleton<T,I>::instance_ ) {
                delete instance_;
                instance_ = nullptr;
            }
        }
        
    public: // Operators Overload
        
        // delete copy and move constructors and assign operators
        Singleton(Singleton const&)            = delete;        // Copy construct
        Singleton(Singleton&&)                 = delete;        // Move construct
        Singleton& operator=(Singleton const&) = delete;        // Copy assign
        Singleton& operator=(Singleton &&)     = delete;        // Move assign
        
    }; // end of class Singleton
    
    template <typename T, typename I, typename S> T* Singleton<T,I,S>::instance_    = nullptr;
    template <typename T, typename I, typename S> I* Singleton<T,I,S>::initializer_ = nullptr;

} // end of namespace casper

#endif // CASPER_SINGLETON_H_
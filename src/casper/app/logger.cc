/**
 * @file logger.h
 *
 * Based on code originally developed for NDrive S.A.
 *
 * Copyright (c) 2011-2018 Cloudware S.A. All rights reserved.
 *
 * This file is part of casper.
 *
 * casper is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * casper is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with osal.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "casper/app/logger.h"

/**
 * @brief This method will be called when it's time to initialize this singleton.
 */
casper::app::Initializer::Initializer (casper::app::Logger& a_instance)
    : ::cc::Initializer<casper::app::Logger>(a_instance)
{
    instance_.loggable_data_ = new ::ev::Loggable::Data(
      /* a_owner_ptr */ this, /* a_ip_addr */ "127.0.0.1", /* a_module */ "casper-application", /* a_tag */ ""
    );
    instance_.client_ = new ::ev::LoggerV2::Client(*instance_.loggable_data_);
}

/**
 * @brief Destructor.
 */
casper::app::Initializer::~Initializer ()
{
    instance_.InnerShutdown(/* a_complete */ true);
}

#ifdef __APPLE__
#pragma mark -
#endif

/**
 * @brief Startup.
 *
 * @param a_path
 * @param a_module
 * @param a_tag
 */
void casper::app::Logger::Startup (const std::string& a_path, const std::string& a_module, const std::string& a_tag)
{
    InnerStartup(a_path, a_module, a_tag, __FUNCTION__);
}

/**
 * @brief Restart.
 *
 * @param a_module
 * @param a_tag
 */
void casper::app::Logger::Restart (const std::string& a_module, const std::string& a_tag)
{
    InnerShutdown(/* a_complete */ false);
    InnerStartup(path_, a_module, a_tag, __PRETTY_FUNCTION__);
}

/**
 * @brief Startup.
 *
 * @param a_path
 * @param a_module
 * @param a_tag
 * @param a_caller_func
 */
void casper::app::Logger::InnerStartup (const std::string& a_path,
                                        const std::string& a_module, const std::string& a_tag,
                                        const char* const a_caller_func)
{
    path_ = a_path;

    loggable_data_->SetModule(a_module);
    loggable_data_->SetTag(a_tag);
    
    // TODO CW
    ::ev::LoggerV2::GetInstance().Startup();
    ::ev::LoggerV2::GetInstance().Register(client_, { "status", "error", /* , "stderr", "stdout" */ });
    ::ev::LoggerV2::GetInstance().Register("status", path_ + std::string("status.log"));
    ::ev::LoggerV2::GetInstance().Register("error", path_ + std::string("error.log"));
//    ::ev::LoggerV2::GetInstance().Register("stderr", path_ + std::string("stderr.log"));
//    ::ev::LoggerV2::GetInstance().Register("stdout", path_ + std::string("stdout.log"));
    ::ev::LoggerV2::GetInstance().Log(client(), "status",
                                      ":::: %s ::::", "::::"
    );
    ::ev::LoggerV2::GetInstance().Log(client(), "status",
                                      ":::: %s (%p) by %s ::::", a_caller_func, this, a_tag.c_str()
    );
    ::ev::LoggerV2::GetInstance().Log(client(), "status",
                                      ":::: %s ::::", "::::"
    );
}

void casper::app::Logger::InnerShutdown (const bool a_complete)
{
    // ... logger ...
    ::ev::LoggerV2::GetInstance().Unregister(client_);
    ::ev::LoggerV2::GetInstance().Shutdown();
    ::ev::LoggerV2::Destroy();
    
    if ( false == a_complete ) {
        return;
    }
    
    // ... all previously allocated pointers ....
    if ( nullptr != loggable_data_ ) {
        delete loggable_data_;
        loggable_data_ = nullptr;
    }
    
    if ( nullptr == client_ ) {
        delete client_;
        client_ = nullptr;
    }
}

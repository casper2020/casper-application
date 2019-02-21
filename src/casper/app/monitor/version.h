/**
 * @file version.h
 *
 * Copyright (c) 2010-2016 Neto Ranito & Seabra LDA. All rights reserved.
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
 * along with casper. If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once
#ifndef CASPER_APP_MONITOR_VERSION_H_
#define CASPER_VERSION_H_

#ifndef CASPER_MONITOR_NAME
    #define CASPER_MONITOR_NAME "casper"
#endif

#ifndef CASPER_MONITOR_VERSION
    #define CASPER_MONITOR_VERSION "x.x.x"
#endif

#ifndef CASPER_MONITOR_INFO
    #define CASPER_MONITOR_INFO CASPER_MONITOR_NAME " v" CASPER_MONITOR_VERSION
#endif

#endif // CASPER_APP_MONITOR_VERSION_H_

/**
 * @file client_handler.h
 *
 * Copyright (c) 2010-2017 Neto Ranito & Seabra LDA. All rights reserved.
 *
 * This file is part of casper.
 *
 * casper is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * casper  is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with casper.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CASPER_MAC_ALERTS_H_
#define CASPER_MAC_ALERTS_H_

#pragma once

#import <Cocoa/Cocoa.h>

@interface Alerts : NSObject

+(NSModalResponse)showInformationalMessage:(NSString*)message informativeText:(NSString*)text andButtons:(NSArray*)buttons;
+(NSModalResponse)showWarningMessage:(NSString*)message informativeText:(NSString*)text andButtons:(NSArray*)buttons;
+(NSModalResponse)showCriticalMessage:(NSString*)message informativeText:(NSString*)text andButtons:(NSArray*)buttons;

@end

#endif // CASPER_MAC_ALERTS_H_

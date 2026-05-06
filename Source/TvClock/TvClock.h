/*******************************************************************************
* TvClock.h
*
* Useful includes and global declarations for the TV Clock.
*
* History:
*   16-JAN-2026 JMC
*      Start.
*
* Copyright (C) 2026 Joseph M. Corbett
*
* This program is free software: you can redistribute it and/or modify it under
* the terms of the GNU General Public License as published by the Free Software
* Foundation, either version 3 of the License, or (at your option) any later
* version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
* details.
*
* You should have received a copy of the GNU General Public License along with
* this program. If not, see <http://www.gnu.org/licenses>.
*
*******************************************************************************/
#if !defined TVLCOCK_H
#define TVLCOCK_H

#include <Adafruit_GFX.h>       // Core graphics library
#include "TouchScreen.h"        // For touchscreen interface.
#include "TsKeypad.h"           // For touchscreen keypad.
#include "ClockFonts.h"         // For fonts.
#include "TimeDisplay.h"        // For the actual time display page.
#include "ClockRtc.h"           // For real time clock (RTC) intfrace.
#include "ClockTz.h"            // For timezone management.
#include "GuiDriver.h"          // For GUI screen management.
#include "Backlight.h"          // For display backlight control.

extern Adafruit_ILI9341 gTft;   // The TFT display.
extern GFXcanvas16 gCanvas;     // The common display canvas.
extern TsKeypad &gTs;           // The touchscreen keypad object.
extern GuiDriver gGui;          // The GUI driver object.
extern TimeDisplay gTd;         // The time display object.
extern Backlight gBacklight;    // Display backlight object.


#endif // TVLCOCK_H.
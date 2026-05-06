/******************************************************************************
* GuiScreens.h
*
* Contains includes and function declarations supporting the screen interface.
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

#if !defined GUISCREENS_H
#define GUISCREENS_H

#include "TvClock.h"            // For many clock related objects.

// Globally accessable functions.
scr_vec_t *GetHomeScreen();     // Returns a pointer to the main screen.
bool IsHomeScreen();            // Returns an indication of whether the current
                                // screen is the home screen.
void InitScreens();             // Initialize the screen system by displaying
                                // the main screen.

#endif // GUISCREENS_H.
/*******************************************************************************
* ClockWelcome.h
*
* Global declarations for the TV Clock welcome screen.
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

#if !defined CLOCKWELCOME_H
#define CLOCKWELCOME_H

/*******************************************************************************
* ClockWelcome namespace
*
* This namespace contains functions to display a power-up welcome screen.
*******************************************************************************/
namespace ClockWelcome
{

    /***************************************************************************
    * ShowWelcome()
    *
    * Shows a bitmapped background with welcome text.
    ***************************************************************************/
    void ShowWelcome();


}; // End ClockWelcome namespace.

#endif // CLOCKWELCOME_H.
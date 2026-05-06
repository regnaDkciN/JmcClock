/*******************************************************************************
* ClockHelper.h
*
* Declares the ClockHelper namespace.  This is basically a collection of useful
* functions and typedefs that don't fit anywhere else.
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

#if !defined CLOCKHELPER_H
#define CLOCKHELPER_H

#include <cstdint>          // For uint16_t.
#include <cmath>            // For fabs().
#include <Adafruit_GFX.h>   // Core graphics library.
#include "RGBConverter.h"   // For conversion between color spaces (HSV and RGB16).


// Type for specifying 16-bit colors.
typedef uint16_t rgb16_t;
// Type for specifying color parameters (hue, saturation, value).
typedef double   hsv_t;


/*******************************************************************************
* ClockHelper namespace
*
* This namespace contains some random miscellaneous things that don't seem
* to fit in any class.  It doesn't make sense to create separate classes for
* them, so a namespace is used to minimize naming conflicts.
*******************************************************************************/
namespace ClockHelper
{
    /***************************************************************************
    * Hsv2Rgb16()
    *
    * Converts HSV color coordinates into the corresponding 16-bit color value
    * used by the display.
    *
    * Arguments:
    *   hue - The hue value of the color being generated (0.0 - 1.0).
    *   sat - The color saturation value of the color being generated (0.0 - 1.0).
    *   val - The brightness value of the color being generated (0.0 - 1.0).
    *
    * Returns:
    *   Returns the 16-bit (rgb16_t) value corresponding to the 3 input arguments.
    ***************************************************************************/
    rgb16_t Hsv2Rgb16(hsv_t hue, hsv_t sat, hsv_t val);


    /***************************************************************************
    * Rgb162Hsv()
    *
    * Converts RGB16 color value used by the display into the corresponding HSV color
    * value.
    *
    * Arguments:
    *   rgb - The 16-bit RGB value to be converted.
    *   rH  - Reference to the generated hue value (0.0 - 1.0).
    *   rS  - Reference to the generated saturation value (0.0 - 1.0).
    *   rV  - Reference to the generated brightness value (0.0 - 1.0).
    *
    * Returns:
    *   Returns the HSV values in the h, s, and v arguments.
    ***************************************************************************/
    void Rgb162Hsv(rgb16_t rgb, hsv_t &rH, hsv_t &rS, hsv_t &rV);


    /***************************************************************************
    * Rgb162Rgb24()
    *
    * Converts an rgb16_t color value to the corresponding 24-bit color value.
    *
    * Arguments:
    *   rgb16 - The rgb16_tt RGB value to be converted.
    *
    * Returns:
    *   Returns the 24-bit value corresponding to rgb16.
    ***************************************************************************/
    uint32_t Rgb162Rgb24(rgb16_t rgb16);


    /***************************************************************************
    * Rgb242Rgb16()
    *
    * Converts 24 bit RGB color value to the corresponding rgb_16 (rgb565) value.
    *
    * Arguments:
    *   rgb24 - The 24-bit RGB value to be converted.
    *
    * Returns:
    *   Returns the 16-bit (rgb565) value corresponding to rgb24..
    ***************************************************************************/
    rgb16_t Rgb242Rgb16(uint32_t rgb24);


    /***************************************************************************
    * PrintVertical()
    *
    * Prints a given string vertically on the given canvas.
    *
    * Arguments:
    *   rCanvas - A reference to the canvas to be printed on.
    *   pStr    - A pointer to the string to display.
    *   x       - The X-coordinate (in pixels) of the first character.
    *   y       - The Y-coordinate (in pixels) of the first character.
    *   spacing - The vertical spacing (in pixels) to add between vertical characters.
    ***************************************************************************/
    void PrintVertical(GFXcanvas16 &rCanvas, const char *pStr,
                       int16_t x, int16_t y, int16_t spacing);

}; // End ClockHelper namespace.


#endif // CLOCKHELPER_H.
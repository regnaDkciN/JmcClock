/*******************************************************************************
* ClockHelper.cpp
*
* This is a collection of useful functions and typedefs that don't fit anywhere
* else.
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
#include "ClockHelper.h"    // Defines the namespace we live in and other stuff.


/*******************************************************************************
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
*******************************************************************************/
rgb16_t ClockHelper::Hsv2Rgb16(hsv_t hue, hsv_t sat, hsv_t val)
{
    double r = 0.0;
    double g = 0.0;
    double b = 0.0;

    RGBConverter::hsvToRgb(hue, sat, val, &r, &g, &b);
    return (((rgb16_t)floor(r * 31.999) & 0x1f) << 11) |
           (((rgb16_t)floor(g * 63.999) & 0x3f) << 5)  |
            ((rgb16_t)floor(b * 31.999) & 0x1f);

} // End Hsv2Rgb16().


/*******************************************************************************
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
*******************************************************************************/
void ClockHelper::Rgb162Hsv(rgb16_t rgb, hsv_t &rH, hsv_t &rS, hsv_t &rV)
{
    double r = (double)((rgb >> 11) & 0x1f) / 31.0;
    double g = (double)((rgb >> 5)  & 0x3f) / 63.0;
    double b = (double)(rgb  & 0x1f) / 31.0;
    RGBConverter::rgbToHsv(r, g, b, &rH, &rS, &rV);
} // End Rgb162Hsv().


/*******************************************************************************
* Rgb242Rgb16()
*
* Converts 24 bit RGB color value to the corresponding rgb_16 (rgb565) value.
*
* Arguments:
*   rgb24 - The 24-bit RGB value to be converted.
*
* Returns:
*   Returns the 16-bit (rgb565) value corresponding to rgb24.
*******************************************************************************/
rgb16_t ClockHelper::Rgb242Rgb16(uint32_t rgb24)
{
    return ((((rgb24 >> 16) >> 3) & 0x1f) << 11) |
           ((((rgb24 >> 8)  >> 2) & 0x3f) << 5)  |
           ((rgb24 >> 3) & 0x1f);
} // End Rgb242Rgb16();


/*******************************************************************************
* Rgb162Rgb24()
*
* Converts an rgb16_t color value to the corresponding 24-bit color value.
*
* Arguments:
*   rgb16 - The rgb16_tt RGB value to be converted.
*
* Returns:
*   Returns the 24-bit value corresponding to rgb16.
*******************************************************************************/
uint32_t ClockHelper::Rgb162Rgb24(rgb16_t rgb16)
{
    return (((rgb16 >> 11) & 0x1f) << 19) |
           (((rgb16 >> 5)  & 0x3f) << 10) |
           ((rgb16 & 0x1f) << 3);
} // End Rgb162Rgb24();


/*******************************************************************************
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
*******************************************************************************/
void ClockHelper::PrintVertical(GFXcanvas16 &rCanvas, const char *pStr,
                  int16_t x, int16_t y, int16_t spacing)
{
    do
    {
        char ch = *pStr++;
        rCanvas.setCursor(x, y);
        rCanvas.print(ch);
        y += spacing;
    } while (*pStr != '\0');
} // End PrintVertical().


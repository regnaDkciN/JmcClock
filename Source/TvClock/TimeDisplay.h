/*******************************************************************************
* TimeDisplay.h
*
* Declares the TimeDisplay class.  This class displays the main clock page
* with the current hour and minute.  It can also optionally display:
*   - Day of Week
*   - Date
*   - Running Seconds
*   - AM/PM indication
*   - Timezone information for example EDT/EST.
*   - An indication of when the clock is offline.
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

#if !defined TIMEDISPLAY_H
#define TIMEDISPLAY_H

#include <Adafruit_GFX.h>       // For canvas and other graphics.
#include <Adafruit_ILI9341.h>   // The 2.8" TFT dispolay driver.
#include <WiFi.h>               // For WiFi handling.
#include "ClockFonts.h"         // For fonts.
#include "TsKeypad.h"           // For touch screen interface.


// Instances of the clock fonts used for the time display.
extern ClockFonts TimeMainFonts;
extern ClockFonts TimeSecondaryFonts;
extern ClockFonts TimeMinorFonts;


/*******************************************************************************
* FontCycleTime enum
*
* Enum for selecting how often, if ever, the HH:MM font will cycle.
*******************************************************************************/
enum FontCycleTime
{
    FC_NEVER = 0,   // Never change the font.
    FC_SECOND,      // Change the font when the second changes (each second).
    FC_MINUTE,      // Change the font when the minute changes (each minute).
    FC_HOUR,        // Change the font when the hour changes (each hour).
    FC_DAY,         // Change the font when the day changes (each day).
    FC_WEEK,        // Change the font when the week changes (each Sunday).
    FC_MONTH,       // Change the font when the month changes (each month).
    FC_YEAR         // Change the font when the year changes (each year).
}; // End FontCycleTime.


/*******************************************************************************
* TimeDisplay class
*
* This class is used to display the current time page.
*******************************************************************************/
class TimeDisplay
{
public:

    /***************************************************************************
    * Constructor
    *
    * Initializes a new TimeDisplay instance.
    *
    * Arguments:
    *   disp   - Reference to the display used to show the time.
    *   canvas - Reference to the canvas onto which the time will be printed.
    *   touch  - Reference to the touch screen being used.
    *   paint  - 'true' to paint screen after canvas is complete.
    ***************************************************************************/
    TimeDisplay(Adafruit_ILI9341 &disp, GFXcanvas16 &canvas, TsKeypad &touch,
                bool paint = true) :
            m_Disp(disp), m_Canvas(canvas), m_Touch(touch), m_Paint(paint),
            m_Hue(0.0), m_Sat(1.0), m_Val(1.0),
            m_OfflineDotRadius(3), m_OfflineColor(ILI9341_RED),
            m_12Hour(true), m_ShowAmPm(true), m_ShowTz(true), m_ShowWkDay(true),
            m_ShowSeconds(true), m_ShowDate(true), m_ShowTemp(true), m_DegreesF(true),
            m_ShowOffline(true), m_CycleColors(true),
            m_FontCycle(FC_MINUTE), m_ColorCyclePeriod(360.0),
            m_PrimaryColor(ILI9341_WHITE), m_SecondaryColor(ILI9341_BLUE),
            m_BgColor(ILI9341_BLACK), m_NvsEnd(0)
    { }


    /***************************************************************************
    * DisplayTime()
    *
    * Displays the current time with optional additional data.
    ***************************************************************************/
    void DisplayTime();


    // Setter for selecting time data to display.
    void Show12Hour(bool a)     { m_12Hour = a; }
    void ShowAmPm(bool a)       { m_ShowAmPm = a; }
    void ShowTz(bool a)         { m_ShowTz = a; }
    void ShowWkDay(bool a)      { m_ShowWkDay = a; }
    void ShowSeconds(bool a)    { m_ShowSeconds = a; }
    void ShowDate(bool a)       { m_ShowDate = a; }
    void ShowTemp(bool a)       { m_ShowTemp = a; }
    void ShowDegreesF(bool a)   { m_DegreesF = a; }
    void SetFontCycle(FontCycleTime t) { m_FontCycle = t; }
    void CycleColors(bool a)    { m_CycleColors = a; }
    void SetColorCyclePeriod(float_t p) { m_ColorCyclePeriod = p; }
    void SetColors(rgb16_t p, rgb16_t s, rgb16_t b)
    {
        m_PrimaryColor = p;
        m_SecondaryColor = s;
        m_BgColor = b;
    } // End SetColors().


    // Getters.
    bool IsShowing12Hour()   const { return m_12Hour; }
    bool IsShowingAmPm()     const { return m_ShowAmPm; }
    bool IsShowingTz()       const { return m_ShowTz; }
    bool IsShowingWkDay()    const { return m_ShowWkDay; }
    bool IsShowingSeconds()  const { return m_ShowSeconds; }
    bool IsShowingDate()     const { return m_ShowDate; }
    bool IsShowingTemp()     const { return m_ShowTemp; }
    bool IsShowingDegreesF() const { return m_DegreesF; }
    FontCycleTime FontCycle() const { return m_FontCycle; }
    bool IsColorCycling()    const { return m_CycleColors; }
    float_t GetColorCyclePeriod() const { return m_ColorCyclePeriod; }
    void GetColors (uint16_t &p, uint16_t &s, uint16_t &b) const
    {
        p = m_PrimaryColor;
        s = m_SecondaryColor;
        b = m_BgColor;
    } // End GetColors().


    #define TD_NVS_START ((uint8_t *)&m_12Hour)
    size_t GetNvsSize()    const    { return &m_NvsEnd - TD_NVS_START; }
    void   SaveNvs(uint8_t *buf)    { memcpy(buf, TD_NVS_START, GetNvsSize()); }
    void   RestoreNvs(uint8_t *buf) { memcpy(TD_NVS_START, buf, GetNvsSize()); }

private:
    // Maximum size of a time string.
    static const size_t MAX_TIME_STRING = 30;

    Adafruit_ILI9341 &m_Disp;   // Reference to the display in use.
    GFXcanvas16      &m_Canvas; // Reference to the canvas in use.
    TsKeypad         &m_Touch;  // Reference to the touch screen in use.

    bool     m_Paint;           // 'true' to paint the screen after canvas setup.
    hsv_t    m_Hue;             // Current hue of the primary color.
    hsv_t    m_Sat;             // Current chroma of the primary color.
    hsv_t    m_Val;             // Current luminance of the primary color.
    uint16_t m_OfflineDotRadius;// Radius, in pixels, of offline indicator.
    rgb16_t  m_OfflineColor;    // Color of offline indicator.

    // Start of options stored in NVS.
    bool m_12Hour;              // 'true' for 12 hour format, 'false' for 24.
    bool m_ShowAmPm;            // 'true' to show AM/PM indicator.
    bool m_ShowTz;              // 'true' to show timezone.
    bool m_ShowWkDay;           // 'true' to show weekday.
    bool m_ShowSeconds;         // 'true' to show seconds.
    bool m_ShowDate;            // 'true' to show date.
    bool m_ShowTemp;            // 'true' to show temperature.
    bool m_DegreesF;            // 'true' for degrees F, else degrees C.
    bool m_ShowOffline;         // 'true' to show when offline.
    bool m_CycleColors;         // 'true' to cycle colors.
    FontCycleTime m_FontCycle;  // When to cycle HH:MM font.
    float_t m_ColorCyclePeriod; // Period of color cycling.
    rgb16_t m_PrimaryColor;     // Primary color (if not cycling).
    rgb16_t m_SecondaryColor;   // Secondary color (if not cycling).
    rgb16_t m_BgColor;          // Background color (if not cycling).
    uint8_t m_NvsEnd;           // Marker for end of saved options.
    // End of options stored in NVS.

}; // End TimeDisplay class.


#endif // TIMEDISPLAY_H.

/*******************************************************************************
* TimeDisplay.cpp
*
* Implements the TimeDisplay class.  This class displays the main clock page
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

#include "TimeDisplay.h"    // Our class declaration.
#include "ClockHelper.h"    // For color manipulation and vertical printing.
#include "ClockRtc.h"       // For RTC interface.

// We're going to use all the functions in the clock helper.
using namespace ClockHelper;

/*******************************************************************************
* Generate the ClockFonts instances for the above fonts.  Thesae are exposed
* for general use.
*******************************************************************************/
ClockFonts TimeMainFonts(PrimaryFonts);
ClockFonts TimeSecondaryFonts(SecondaryFonts);
ClockFonts TimeMinorFonts(TertiaryFonts);


/*******************************************************************************
* DisplayTime()
*
* Displays the current time with optional additional data.
*******************************************************************************/
void TimeDisplay::DisplayTime()
{
    // Get the current unix time.
    time_t now;
    time(&now);

    // Convert unix time to usable local time data.
    tm timeinfo;
    localtime_r(&now, &timeinfo);

#if defined DEBUGGING
    // For debugging it may be useful to force a specific time.  The following
    // code allows for an external agent to set a specific time to be displayed.
    extern uint16_t month;
    extern uint16_t day;
    extern uint16_t year;
    extern uint16_t hour;
    extern uint16_t minute;
    extern uint16_t second;

    timeinfo.tm_sec = second;
    timeinfo.tm_min = minute;
    timeinfo.tm_hour = hour;
    timeinfo.tm_mday = day;
    timeinfo.tm_mon = month;
    timeinfo.tm_year = year;
    timeinfo.tm_isdst = true;

    now = mktime(&timeinfo);
#endif

    // Detect when time changes (second, minute, hour, ...).
    static uint16_t lastSecond = 60;
    static uint16_t lastMinute = 60;
    static uint16_t lastHour   = 25;
    static uint16_t lastDay    = 32;
    static uint16_t lastMonth  = 13;
    static uint16_t lastYear   = 1000;
    bool secondsChanged = timeinfo.tm_sec  != lastSecond;
    bool minutesChanged = timeinfo.tm_min  != lastMinute;
    bool hoursChanged   = timeinfo.tm_hour != lastHour;
    bool dayChanged     = timeinfo.tm_mday != lastDay;
    bool wkChanged      = dayChanged && (timeinfo.tm_wday == 0);
    bool monthChanged   = timeinfo.tm_mon  != lastMonth;
    bool yearChanged    = timeinfo.tm_year != lastYear;

    // Check for touches that indicate font change.
    if (secondsChanged)
    {
        // Get a reference to the real time clock for later use.
        ClockRtc &rtc = ClockRtc::Instance();

        // See if it's time to bump the font.
        if ((m_FontCycle == FC_SECOND)                    ||
           ((m_FontCycle == FC_MINUTE) && minutesChanged) ||
           ((m_FontCycle == FC_HOUR)   && hoursChanged)   ||
           ((m_FontCycle == FC_DAY)    && dayChanged)     ||
           ((m_FontCycle == FC_WEEK)   && wkChanged)      ||
           ((m_FontCycle == FC_MONTH)  && monthChanged)   ||
           ((m_FontCycle == FC_YEAR)   && yearChanged))
        {
            TimeMainFonts.NextActive();;
        }

        // If we're cycling colors, then do it here.
        if (m_CycleColors)
        {
            // Cycle colors in constant brightness.  Here we select the main color.
            // Cycle at rate given by m_ColorCyclePeriod.
            m_Hue += 1.0 / m_ColorCyclePeriod;
            if (m_Hue >= 1.0)
            {
                m_Hue -= 1.0;
            }

            // Select the alternate color as the complement of the main color.
            float_t   altHue = m_Hue + 0.5;
            if (altHue >= 1.0)
            {
                altHue -= 1.0;
            }
            // Save the new colors.
            m_PrimaryColor    = Hsv2Rgb16(m_Hue, m_Sat, m_Val);
            m_SecondaryColor  = Hsv2Rgb16(altHue, m_Sat, m_Val);
        }

        // Note: strftime() doesn't include a way to display the hour
        // value without either a leading 0 or space, so we need to
        // adjust our time data here.
        // Adjust based on 12 or 24 hour format.
        uint16_t currentHour = timeinfo.tm_hour;
        if (m_12Hour && (currentHour > 12))
        {
            currentHour -= 12;
            if (currentHour == 0)
            {
                currentHour = 12;
            }
        }

        // Initialize the canvas by clearing it.
        m_Canvas.fillScreen(m_BgColor);

        // Allocate a buffer to hold our time strings.
        char timeString[MAX_TIME_STRING] = { 0 };

        // Coordinate and size variables needed by getTextBounds().
        int16_t x1;
        int16_t y1;
        uint16_t w;
        uint16_t h;
        uint16_t line;
        uint16_t col;

        // Display the current time HH:MM.  Can't use strftime() due to
        // its inability to display hours in single digit format.
        sprintf(timeString, "%u:%02u", currentHour, timeinfo.tm_min);
        m_Canvas.setFont(TimeMainFonts.Font());
        m_Canvas.setTextSize(1);
        m_Canvas.getTextBounds(timeString, 0, 0, &x1, &y1, &w, &h);
        line = ((m_Canvas.height() - h) / 2) - y1;
        col  = ((m_Canvas.width()  - w) / 2) - x1;
        m_Canvas.setCursor(col, line);
        m_Canvas.setTextColor(m_PrimaryColor);
        m_Canvas.print(timeString);

        // Is the weekday supposed to be shown?
        if (m_ShowWkDay)
        {
            // Display the day of the week.
            strftime(timeString, sizeof(timeString), "%A", &timeinfo);
            m_Canvas.setFont(TimeSecondaryFonts.Font());
            m_Canvas.setTextSize(1);
            m_Canvas.getTextBounds(timeString, 0, 0, &x1, &y1, &w, &h);
            line = h + 10;
            col  = ((m_Canvas.width() - w) / 2) - x1;
            m_Canvas.setCursor(col, line);
            m_Canvas.setTextColor(m_SecondaryColor);
            m_Canvas.print(timeString);
        }

        // Are we displaying seconds?
        if (m_ShowSeconds)
        {
            // Display the current seconds.
            sprintf(timeString, "%02u", timeinfo.tm_sec);
            m_Canvas.setFont(TimeSecondaryFonts.Font());
            m_Canvas.setTextSize(1);
            m_Canvas.getTextBounds(timeString, 0, 0, &x1, &y1, &w, &h);
            line = 3 * m_Canvas.height() / 4 + 21;
            col  = m_Canvas.width() - w - 6;
            m_Canvas.setCursor(col, line);
            m_Canvas.setTextColor(m_PrimaryColor);
            m_Canvas.print(timeString);
        }

        // Are we displaying temperature?
        if (m_ShowTemp)
        {
            // Display the temperature in correct units.
            float_t degrees = m_DegreesF ? rtc.GetTempF() : rtc.GetTempC();
            sprintf(timeString, "%.0f", degrees);
            m_Canvas.setFont(TimeSecondaryFonts.Font());
            m_Canvas.setTextSize(1);
            m_Canvas.getTextBounds(timeString, 0, 0, &x1, &y1, &w, &h);
            line = 3 * m_Canvas.height() / 4 + 21;
            col  = 0;
            m_Canvas.setCursor(col, line);
            m_Canvas.setTextColor(m_PrimaryColor);
            m_Canvas.print(timeString);

            // Show the degrees symbol.  Since the font we're using doesn't have
            // a degree symbol, we use one from a diffetent font set and shift
            // it into place.
            const char DEGREES = '\xf7';
            sprintf(timeString, "%c", DEGREES);
            m_Canvas.setFont();
            m_Canvas.setTextSize(2);
            line = 3 * m_Canvas.height() / 4 - 2;
            col  = w + 4;
            m_Canvas.getTextBounds(timeString, 0, 0, &x1, &y1, &w, &h);
            m_Canvas.setCursor(col, line);
            m_Canvas.print(timeString);

            //  Show the units (F or C).
            m_Canvas.setFont(TimeSecondaryFonts.Font());
            m_Canvas.setTextSize(1);
            col += w - 4;
            sprintf(timeString, "%c", m_DegreesF ? 'F' : 'C');
            line = 3 * m_Canvas.height() / 4 + 21;
            m_Canvas.setCursor(col, line);
            m_Canvas.print(timeString);
        }

        // Are we displaying AM/PM?
        if (m_ShowAmPm && m_12Hour)
        {
            // Display AM/PM.
            strftime(timeString, sizeof(timeString), "%p", &timeinfo);
            m_Canvas.setFont(TimeMinorFonts.Font());
            m_Canvas.setTextSize(1);
            m_Canvas.setTextColor(m_SecondaryColor);
            PrintVertical(m_Canvas, timeString, 0, 25, 15);
        }

        // Are we displaying timezone?
        if (m_ShowTz)
        {
            // Display dst or not (i.e. EST or EDT).
            strftime(timeString, sizeof(timeString), "%Z", &timeinfo);
            m_Canvas.setFont(TimeMinorFonts.Font());
            m_Canvas.setTextSize(1);
            m_Canvas.setTextColor(m_SecondaryColor);
            PrintVertical(m_Canvas, timeString, m_Canvas.width() - 20, 20, 15);
        }

        // Are we showing the date?
        if (m_ShowDate)
        {
            // Display the date.
            size_t s = strftime(timeString, sizeof(timeString), "%B", &timeinfo);
            snprintf(timeString + s, sizeof(timeString) - s, " %d, %d", timeinfo.tm_mday, timeinfo.tm_year + 1900);
            m_Canvas.setFont(TimeSecondaryFonts.Font());
            m_Canvas.setTextSize(1);
            m_Canvas.getTextBounds(timeString, 0, 0, &x1, &y1, &w, &h);
            line = m_Canvas.height() - 10;
            col  =  ((m_Canvas.width()  - w) / 2) - x1;
            m_Canvas.setCursor(col, line);
            m_Canvas.setTextColor(m_SecondaryColor);
            m_Canvas.print(timeString);
        }

        // Draw a dot if not online.
        if (m_ShowOffline && !rtc.UsingNetworkTime())
        {
            m_Canvas.fillCircle(m_Canvas.width() - m_OfflineDotRadius,
                                m_Canvas.height() - m_OfflineDotRadius,
                                m_OfflineDotRadius, m_OfflineColor);
        }

        // Finally, display the page if allowed.
        if (m_Paint)
        {
            m_Disp.drawRGBBitmap(0, 0, m_Canvas.getBuffer(), m_Canvas.width(), m_Canvas.height());
        }
    }

    // Update our time change variables.
    lastSecond = timeinfo.tm_sec;
    lastMinute = timeinfo.tm_min;
    lastHour   = timeinfo.tm_hour;
    lastDay    = timeinfo.tm_mday;
    lastMonth  = timeinfo.tm_mon;
    lastYear   = timeinfo.tm_year;

} // End DisplayTime().

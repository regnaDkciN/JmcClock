/*******************************************************************************
* ClockRtc.h
*
* Declares the ClockRtc class.  This class handles all interactions with the
* real time clock.  Assumes use of DS3231 RTC.
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

#if !defined CLOCKRTC_H
#define CLOCKRTC_H

#include <RTClib.h>     // For RTC_DS3231 class.


/*******************************************************************************
* ClockRtc class
*
* This class derives from the DS3231 class, and supports use of the
* DS3231 real time clock.
*
* Note: This class only works with the DS3231 real time clock.
*******************************************************************************/
class ClockRtc : RTC_DS3231
{
public:
    // We're implemented as a singleton.  Return a reference to the only instance.
    static ClockRtc &Instance()
    {
        static ClockRtc rtc;
        return rtc;
    } // End Instance().


    // Simple getter to return indication of whether or not we're using NTP time.
    bool UsingNetworkTime() const { return m_UsingNetworkTime; }


    /***************************************************************************
    * GetTempX()
    *
    * Returns the float value of the current temperature in degrees C or F.
    ***************************************************************************/
    float GetTempC() { return getTemperature(); }
    float GetTempF() { return GetTempC() * (9.0 / 5.0) + 32.0; }


    /***************************************************************************
    * SetClock()
    *
    * Sets system time.  Tries to get time from NTP server(s).  If successful,
    * updates the RTC time and system time with NTP time.  If unsuccessful,
    * reads time from the (battery backed up) RTC and sets system time based
    * on it.
    *
    ***************************************************************************/
    void SetClock();

private:
    ClockRtc() : RTC_DS3231(), m_UsingNetworkTime(false) { begin(); }
    ~ClockRtc() { }

    // Unimplemented methods.
    ClockRtc(ClockRtc &r);
    ClockRtc &operator=(ClockRtc &r);

    bool m_UsingNetworkTime;     // 'true' if we're using NTP time.
}; // End ClockRtc class.

#endif // CLOCKRTC_H.
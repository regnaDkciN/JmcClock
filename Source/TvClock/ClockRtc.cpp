/*******************************************************************************
* ClockRtc.cpp
*
* Implements the ClockRtc class.  This class handles all interactions with the
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

#include "ClockWiFi.h"  // For WiFi connection.
#include "ClockRtc.h"   // DS3231 interface class that we derive from.


/*******************************************************************************
* SetClock()
*
* Sets system time.  Tries to get time from NTP server(s).  If successful,
* updates the RTC time and system time with NTP time.  If unsuccessful,
* reads time from the (battery backed up) RTC and sets system time based
* on it.
*
*******************************************************************************/
void ClockRtc::SetClock()
{
    // If we got NTP time use it, othrewise, use RTC clock time.
    if (ClockWiFi::NtpConnected())
    {
        // We got a valid NTP time.  Assume RTC is OK and set the real time
        // clock based on the new NTP time.
        Serial.printf("Got NTP time!\n");
        time_t now;
        time(&now);
        DateTime dt(now);
        this->adjust(dt);
        m_UsingNetworkTime = true;
    }
    else
    {
        // NTP time fetch failed.  Use time from the real time clock.
        // Assume RTC is OK.  Set system time based on the real time clock.
        Serial.printf("NTP init failed, using RTC time!\n");
        DateTime dt = this->now();
        timeval tv;
        tv.tv_sec = dt.unixtime();
        tv.tv_usec = 500000;  // Add half second of micros.
        settimeofday(&tv, NULL);
        m_UsingNetworkTime = false;
    }

    // Actually set the system clock.
    time_t now = time(nullptr);
    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);
    Serial.print("Current time: ");
    Serial.print(asctime(&timeinfo));
} // End SetClock().
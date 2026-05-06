/*******************************************************************************
* ClockWiFi.h
*
* Declares the ClockWiFi class.  This class handles all interactions with the
* WiFi network.
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

#if !defined CLOCKWIFI_H
#define CLOCKWIFI_H

#include <WiFi.h>       // For the WiFi standard library.


/*******************************************************************************
* ClockWiFi class
*
* Provides the interface to WiFi including SSID, Password, and up to two NTP
* server URLs.  All methods and data are static instead of making this a singleton.
*******************************************************************************/
class ClockWiFi
{
public:
    // Methods whose use should be pretty self evident.
    static void Init();
    static char *GetSsid()       { return m_Ssid; }
    static char *GetPwd()        { return m_Pwd; }
    static char *GetNtpServer1() { return m_NtpServer1; }
    static char *GetNtpServer2() { return m_NtpServer2; }
    static void SetSsid(const char *pBuf) { strlcpy(m_Ssid, pBuf, MAX_NAME_LEN); }
    static void SetPwd(const char *pBuf) { strlcpy(m_Pwd, pBuf, MAX_NAME_LEN); }
    static void SetNtpServer1(const char *pBuf) { strlcpy(m_NtpServer1, pBuf, MAX_NAME_LEN); }
    static void SetNtpServer2(const char *pBuf) { strlcpy(m_NtpServer2, pBuf, MAX_NAME_LEN); }
    static bool WiFiConnected()  { return WiFi.status() == WL_CONNECTED; }
    static bool NtpConnected()   { return m_NtpConnected; }

    /****************************************************************************
    * Scan()
    *
    * Scans for local WiFi networks.  Saves the SSID string of all unique local
    * networks found for later querry via GetNet().
    *
    * Returns:
    *   Returns the number of unique local WiFi networks found.
    ****************************************************************************/
    static uint32_t Scan();

    /****************************************************************************
    * GetNumNets()
    *
    * Returns the number of unique local networks found via Scan().
    *
    * Returns:
    *   Returns the number of unique local WiFi networks found via Scan().
    ****************************************************************************/
    static uint32_t GetNumNets() { return m_NumNets; }

    /****************************************************************************
    * GetNet()
    *
    * Returns a pointer tho the SSID string of a local WiFi network found via Scan().
    *
    * Arguments:
    *   i - The index of the requested WiFi network that was found via Scan().
    *       Valid values are 0 through m_NumNets.
    *
    * Returns:
    *   Returns a pointer to the requested SSID string.
    ****************************************************************************/
    static char *GetNet(size_t i) { return i < m_NumNets ? m_UniqueNets[i] : NULL; }

    /****************************************************************************
    * Connect()
    *
    * Attempts to connect to the WiFi network currently specified by m_Ssid using
    * the password currently specified by m_Pwd.  If successful, attempts to
    * connect to one of the NTP servers specified by m_NtpServer1 and m_NtpServer2.
    *
    * Returns:
    *   Returns 'true' if a network connections was successfully established and
    *   a connection to an NTP server was successfully completed.
    *   Returns 'false' otherwise.
    ****************************************************************************/
    static bool  Connect();

    /****************************************************************************
    * NVS related methods.
    ****************************************************************************/
    /****************************************************************************
    * GetNvsSize()
    *
    * Returns the size, in bytes, of the data stored in nonvolatile storage (NVS).
    ****************************************************************************/
    static size_t GetNvsSize() { return MAX_NAME_LEN * 4; }

    /****************************************************************************
    * SaveNvs()
    *
    * Saves our SSID, password, and NVS server data to NVS at the specified
    * address.
    *
    * Arguments:
    *   pBuf - A pointer to the address to which our settings will be saved.
    ****************************************************************************/
    static void   SaveNvs(uint8_t *pBuf);

    /****************************************************************************
    * RestoreNvs()
    *
    * Restores our SSID, password, and NVS server data from the specified NVS
    * address.
    *
    * Arguments:
    *   pBuf - A pointer to the address from which our settings will be restored.
    ****************************************************************************/
    static void   RestoreNvs(uint8_t *buf);

    static const size_t MAX_NAME_LEN = 33;  // Max length for SSID, pwd, and servers.
    static const size_t MAX_NUM_NETS = 16;  // Max number local nets to save.

private:
    static bool     m_NtpConnected;         // 'true' if connected to NTP server.
    static uint32_t m_NumNets;              // Number local WiFi nets found via Scan().
    static char     m_UniqueNets[MAX_NUM_NETS][MAX_NAME_LEN]; // Holds local net SSIDs.

    // Start of options stored in NVS.
    static char     m_Ssid[MAX_NAME_LEN];       // SSID used to connect to WiFi.
    static char     m_Pwd[MAX_NAME_LEN];        // Password used to connect to WiFi.
    static char     m_NtpServer1[MAX_NAME_LEN]; // NTP server 1 URL.
    static char     m_NtpServer2[MAX_NAME_LEN]; // NTP server 2 URL.
    // End of options stored in NVS.
}; // End ClockWiFi class.


#endif // CLOCKWIFI_H.


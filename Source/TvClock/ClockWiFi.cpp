/*******************************************************************************
* ClockWiFi.cpp
*
* Implements the ClockWiFi class.  This class handles all interactions with the
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
#include "ClockWiFi.h"      // Our class declarations.


// Class static data.
bool     ClockWiFi::m_NtpConnected = false;
uint32_t ClockWiFi::m_NumNets = 0;
char     ClockWiFi::m_Ssid[MAX_NAME_LEN] = {0};
char     ClockWiFi::m_Pwd[MAX_NAME_LEN] = {0};
char     ClockWiFi::m_NtpServer1[MAX_NAME_LEN] = {0};
char     ClockWiFi::m_NtpServer2[MAX_NAME_LEN] = {0};
char     ClockWiFi::m_UniqueNets[MAX_NUM_NETS][MAX_NAME_LEN] = {0};


/*******************************************************************************
* Init()
*
* Initializes the ClockWiFi class to factory values.  These values may later be
* overwritten via RestoreNvs().
*******************************************************************************/
void ClockWiFi::Init()
{
    // Not connected yet.
    m_NtpConnected = false;
    m_NumNets = false;

    // Clear all string buffers to 0.
    memset(m_Ssid, 0, sizeof(m_Ssid));
    memset(m_Pwd, 0, sizeof(m_Pwd));
    memset(m_NtpServer1, 0, sizeof(m_NtpServer1));
    memset(m_NtpServer2, 0, sizeof(m_NtpServer2));
    memset(m_UniqueNets, 0, sizeof(m_UniqueNets));

    // Start with prompt values for SSID and password.
    SetSsid("SSID");
    SetPwd("PWD");

    // Start with popular US NTP servers.
    SetNtpServer1("pool.ntp.org");
    SetNtpServer2("time.nist.gov");
} // End Init().


/*******************************************************************************
* Scan()
*
* Scans for local WiFi networks.  Saves the SSID string of all unique local
* networks found for later querry via GetNet().
*
* Returns:
*   Returns the number of unique local WiFi networks found.
*******************************************************************************/
uint32_t ClockWiFi::Scan()
{
    // Scan for nearby networks.
    Serial.println("** Scan Networks **");

    // Start with index 0.
    uint32_t numRawNets = 0;

    // Scan for local networks.  If none found, simply return 0.
    if ((numRawNets = WiFi.scanNetworks()) == (uint32_t)-1)
    {
        return 0;
    }

    // Print the list of networks seen (for debug).
    Serial.printf("Number of available networks: %d\n", numRawNets);

    // Save unique SSIDs in our local buffer.
    m_NumNets = 0;
    for (size_t thisNet = 0;
        (thisNet < numRawNets) && (m_NumNets < MAX_NUM_NETS - 1); thisNet++)
    {
        // Print the network number and name for each network found.
        Serial.printf("%d) %s\tSignal: %d\n", thisNet, WiFi.SSID(thisNet),
                      WiFi.RSSI(thisNet));
        if (*WiFi.SSID(thisNet))
        {
            // Save the SSID locally only if we havn't already seen it.
            size_t i = 0;
            for (; i < m_NumNets; i++)
            {
                if (!strcmp(WiFi.SSID(thisNet), m_UniqueNets[i]))
                {
                    break;
                }
            }
            if (i >= m_NumNets)
            {
                memset(m_UniqueNets[m_NumNets], 0, MAX_NAME_LEN);
                strncpy(m_UniqueNets[m_NumNets++], WiFi.SSID(thisNet), MAX_NAME_LEN - 1);
            }
        }
    }
    Serial.println();

    // More debug info.
    Serial.printf("Number of unique networks: %d\n", m_NumNets);
    for (size_t i = 0; i < m_NumNets; i++)
    {
        Serial.printf("array[%d]   %s\n", i, m_UniqueNets[i]);
    }

    // Return the number of unique nets found.
    return m_NumNets;
} // End Scan().


/*******************************************************************************
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
*******************************************************************************/
bool  ClockWiFi::Connect()
{
    // Try to connect.
    WiFi.begin(m_Ssid, m_Pwd);
    if (WiFiConnected())
    {
        // Connected successfully.
        // Start NTP connection.  Try to get time from the NTP hosts.
        NTP.begin(m_NtpServer1, m_NtpServer2, 5000);
        m_NtpConnected = NTP.waitSet(15 * 1000);
    }

    // Return an indication of whether or not NTP is connected.
    return m_NtpConnected;
} // End Connect().


/*******************************************************************************
* SaveNvs()
*
* Saves our SSID, password, and NVS server data to NVS at the specified
* address.
*
* Arguments:
*   pBuf - A pointer to the address to which our settings will be saved.
*******************************************************************************/
void ClockWiFi::SaveNvs(uint8_t *buf)
{
    // We simply copy our data to the specified NVS address.
    memcpy(buf, m_Ssid, sizeof(m_Ssid));
    buf += sizeof(m_Ssid);
    memcpy(buf, m_Pwd, sizeof(m_Pwd));
    buf += sizeof(m_Pwd);
    memcpy(buf, m_NtpServer1, sizeof(m_NtpServer1));
    buf += sizeof(m_NtpServer1);
    memcpy(buf, m_NtpServer2, sizeof(m_NtpServer2));
} // End SaveNvs().


/*******************************************************************************
* RestoreNvs()
*
* Restores our SSID, password, and NVS server data from the specified NVS
* address.
*
* Arguments:
*   pBuf - A pointer to the address from which our settings will be restored.
*******************************************************************************/
void ClockWiFi::RestoreNvs(uint8_t *buf)
{
    // Simply copy our data from NVS to local buffers.
    memcpy(m_Ssid, buf, sizeof(m_Ssid));
    buf += sizeof(m_Ssid);
    memcpy(m_Pwd, buf, sizeof(m_Pwd));
    buf += sizeof(m_Pwd);
    memcpy(m_NtpServer1, buf, sizeof(m_NtpServer1));
    buf += sizeof(m_NtpServer1);
    memcpy(m_NtpServer2, buf, sizeof(m_NtpServer2));
    Connect();
} // End RestoreNvs().

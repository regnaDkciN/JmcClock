/*******************************************************************************
* ClockNvs.cpp
*
* Implements the ClockNvs class.  This class supports NVS storage and retrieval
* of clock related settings.
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

#include <EEPROM.h>         // For RP2350 EEPROM simulation code.
#include "ClockNvs.h"       // Our class.
#include "ClockWiFi.h"      // For ClockWiFi class.


/*******************************************************************************
* Instance()
*
* Creates and return a reference to our singleton instance of our class.
*
* Returns:
*   Always returns a reference to the singleton instance of our class.
*******************************************************************************/
ClockNvs &ClockNvs::Instance()
{
    static ClockNvs nvs;
    return nvs;
} // End Instance().


/*******************************************************************************
* Init()
*
* Populates the NVS cache header then initializes the EEPROM system if it has
* not already been initialized.  Sets m_Initialized based on the result of the
* EEPROM initialization.
*******************************************************************************/
void ClockNvs::Init()
{
    // Fill the NVS cache header, then start the EEPROM subsystem.
    // This must be done before EEPROM is initialized because it sets up
    // the value of m_UserSize (the NVS data size).
    FillHeader();

    // Make sure we do this only once.
    if (!m_Initialized)
    {
        if (m_UserSize <= NVS_PARTITION_SIZE)
        {
            EEPROM.begin(NVS_PARTITION_SIZE);
            m_Initialized = true;
        }
        else
        {
            Serial.printf("ERROR: NVS m_UserSize (%d) larger than partition size (%d).\n",
                           m_UserSize, NVS_PARTITION_SIZE);
        }
    }
} // End Init().


/*******************************************************************************
* FillHeader()
*
* Fills in the NvsImage header of the NVS cache.  Does not update the
* payload or the payload checksum.
*******************************************************************************/
void ClockNvs::FillHeader()
{
    // Initially clear the entire NVS cache.
    memset(m_Cache, 0, sizeof(m_Cache));

    // Now fill in the cache header fields.
    NvsImage *pImage = (NvsImage *)m_Cache;
    pImage->m_Id = NVS_ID;
    pImage->m_IdComp = ~NVS_ID;

    pImage->m_Version = NVS_VERSION;
    pImage->m_VersionComp = ~NVS_VERSION;

    m_UserSize = sizeof(NvsImage);

    size_t len = gTd.GetNvsSize();
    pImage->m_TimeDisplayOfst = m_UserSize;
    pImage->m_TimeDisplayOfstComp = ~pImage->m_TimeDisplayOfst;
    m_UserSize += len;

    len = gBacklight.GetNvsSize();
    pImage->m_BacklightOfst = m_UserSize;
    pImage->m_BacklightOfstComp = ~pImage->m_BacklightOfst;
    m_UserSize += len;

    len = ClockTz::GetNvsSize();
    pImage->m_ClockTzOfst = m_UserSize;
    pImage->m_ClockTzOfstComp = ~pImage->m_ClockTzOfst;
    m_UserSize += len;

    len = TimeMainFonts.GetNvsSize();
    pImage->m_TimeMainFontsOfst = m_UserSize;
    pImage->m_TimeMainFontsOfstComp = ~pImage->m_TimeMainFontsOfst;
    m_UserSize += len;

    len = TimeSecondaryFonts.GetNvsSize();
    pImage->m_TimeSecondaryFontsOfst = m_UserSize;
    pImage->m_TimeSecondaryFontsOfstComp = ~pImage->m_TimeSecondaryFontsOfst;
    m_UserSize += len;

    len = TimeMinorFonts.GetNvsSize();
    pImage->m_TimeMinorFontsOfst = m_UserSize;
    pImage->m_TimeMinorFontsOfstComp = ~pImage->m_TimeMinorFontsOfst;
    m_UserSize += len;

    len = ClockWiFi::GetNvsSize();
    pImage->m_ClockWiFiOfst = m_UserSize;
    pImage->m_ClockWiFiOfstComp = ~pImage->m_ClockWiFiOfst;
    m_UserSize += len;

    pImage->m_Size = m_UserSize;
    pImage->m_SizeComp = ~m_UserSize;
} // End FillHeader().


/*******************************************************************************
* PrintCache()
*
* Prints the contents of the cache, which consists of the latest image of
* the NVS data including the NvsImage header and all payload data.
* This is normally only used for debugging.
*******************************************************************************/
void ClockNvs::PrintCache()
{
    NvsImage *pC = (NvsImage *)m_Cache;
    Serial.printf("m_Id: %x  %x\n", pC->m_Id, pC->m_IdComp);
    Serial.printf("m_Version: %x  %x\n", pC->m_Version, pC->m_VersionComp);
    Serial.printf("m_TimeDisplayOfst: %x  %x\n", pC->m_TimeDisplayOfst, pC->m_TimeDisplayOfstComp);
    Serial.printf("m_BacklightOfst: %x  %x\n", pC->m_BacklightOfst, pC->m_BacklightOfstComp);
    Serial.printf("m_ClockTzOfst: %x  %x\n", pC->m_ClockTzOfst, pC->m_ClockTzOfstComp);
    Serial.printf("m_TimeMainFontsOfst: %x  %x\n", pC->m_TimeMainFontsOfst, pC->m_TimeMainFontsOfstComp);
    Serial.printf("m_TimeSecondaryFontsOfst: %x  %x\n", pC->m_TimeSecondaryFontsOfst, pC->m_TimeSecondaryFontsOfstComp);
    Serial.printf("m_TimeMinorFontsOfst: %x  %x\n", pC->m_TimeMinorFontsOfst, pC->m_TimeMinorFontsOfstComp);
    Serial.printf("m_ClockWiFiOfst: %x  %x\n", pC->m_ClockWiFiOfst, pC->m_ClockWiFiOfstComp);
    Serial.printf("m_Size: %x  %x\n", pC->m_Size, pC->m_SizeComp);
    Serial.printf("m_Checksum: %x  %x\n", pC->m_Checksum, pC->m_ChecksumComp);

    uint8_t *p8 = (uint8_t *)m_Cache + sizeof(NvsImage);
    for (size_t i = sizeof(NvsImage); i < pC->m_Size; i++)
    {
        if (i == pC->m_TimeDisplayOfst)
        {
            Serial.printf("TimeDisplay\n");
        }
        if (i == pC->m_BacklightOfst)
        {
            Serial.printf("Backlight\n");
        }
        else if (i == pC->m_ClockTzOfst)
        {
            Serial.printf("ClockTz\n");
        }
        else if (i == pC->m_TimeMainFontsOfst)
        {
            Serial.printf("TimeMainFonts\n");
        }
        else if (i == pC->m_TimeSecondaryFontsOfst)
        {
            Serial.printf("TimeSecondaryFonts\n");
        }
        else if (i == pC->m_TimeMinorFontsOfst)
        {
            Serial.printf("TimeMinorFonts\n");
        }
        else if (i == pC->m_ClockWiFiOfst)
        {
            Serial.printf("WiFi\n");
        }
        if (i >= pC->m_ClockWiFiOfst)
        {
            Serial.printf("%3d - %c\n", i, *(char *)p8++);
        }
        else
        {
            Serial.printf("%3d - %2x\n", i, *p8++);
        }
    }
} // End PrintCache().


/*******************************************************************************
* Save()
*
* Saves all clock setting data to NVS.
*
* Returns:
*   Returns 'true' if successful, and 'false' otherwise.
*******************************************************************************/
bool ClockNvs::Save()
{
    // If Init() failed, then we can't go any further.
    if (!m_Initialized)
    {
        return false;
    }

    // Update the NVS cache header.
    FillHeader();

    // Copy each subsystem's NVS data to the NVS cache.
    NvsImage *pImage = (NvsImage *)m_Cache;
    uint8_t *pArray = (uint8_t *)m_Cache;
    gTd.SaveNvs(pArray + pImage->m_TimeDisplayOfst);
    gBacklight.SaveNvs(pArray + pImage->m_BacklightOfst);
    ClockTz::SaveNvs(pArray + pImage->m_ClockTzOfst);
    TimeMainFonts.SaveNvs(pArray + pImage->m_TimeMainFontsOfst);
    TimeSecondaryFonts.SaveNvs(pArray + pImage->m_TimeSecondaryFontsOfst);
    TimeMinorFonts.SaveNvs(pArray + pImage->m_TimeMinorFontsOfst);
    ClockWiFi::SaveNvs(pArray + pImage->m_ClockWiFiOfst);

    // Update the checksum.
    pImage->m_Checksum = CalcChecksum();
    pImage->m_ChecksumComp = ~pImage->m_Checksum;

    // Save our data to EEPROM.
    return SaveCache();
} // End Save().


/*******************************************************************************
* Restore()
*
* Restores all clock setting data from NVS.
*
* Returns:
*   Returns 'true' if successful, and 'false' otherwise.
*******************************************************************************/
bool ClockNvs::Restore()
{
    // If Init() failed, then we can't go any further.
    if (!m_Initialized)
    {
        return false;
    }

    // Read the saved EEPROM data into our local cache.
    RestoreCache();

    // Validate the cache before trying to use the restored data.
    if (!CacheValid())
    {
        Serial.printf("Restore failed due to invalid cache.\n");
        return false;
    }

    // Everything looks good.  Copy the (possibly) new data to each subsystem.
    NvsImage *pImage = (NvsImage *)m_Cache;
    uint8_t *pArray = (uint8_t *)m_Cache;
    gTd.RestoreNvs(pArray + pImage->m_TimeDisplayOfst);
    gBacklight.RestoreNvs(pArray + pImage->m_BacklightOfst);
    ClockTz::RestoreNvs(pArray + pImage->m_ClockTzOfst);
    TimeMainFonts.RestoreNvs(pArray + pImage->m_TimeMainFontsOfst);
    TimeSecondaryFonts.RestoreNvs(pArray + pImage->m_TimeSecondaryFontsOfst);
    TimeMinorFonts.RestoreNvs(pArray + pImage->m_TimeMinorFontsOfst);
    ClockWiFi::RestoreNvs(pArray + pImage->m_ClockWiFiOfst);

    // All subsystems have been updated, now try to connect to WiFi.
    Serial.printf("Attempting to connect...");
    ClockWiFi::Connect();
    Serial.println();

    // Set the clock's time.
    ClockRtc &rtc = ClockRtc::Instance();
    rtc.SetClock();
    return true;
} // End Restore().


/*******************************************************************************
* FactoryReset()
*
* Resets all clock settings to the factory defaults.  Be aware that all
* current settings will be lost.  As part of the operation, a software
* reset is performed.
*
* Returns:
*   Returns 'true' if successful, and 'false' otherwise.
*******************************************************************************/
bool ClockNvs::FactoryReset()
{
    // If Init() failed, then we can't go any further.
    if (!m_Initialized)
    {
        return false;
    }

    // First we need to invalidate the NVS cache.  This is easily done by
    // clearing it to all zeros.
    memset(m_Cache, 0, sizeof(m_Cache));

    // Save the cleared cache to EEPROM.
    if (SaveCache())
    {
        // Let the user know we're resetting.
        Serial.printf("FactoryReset.\n");
        delay(500);

        // Software reset the system.  !! Won't return from this. !!
        #define AIRCR_Register (*((volatile uint32_t*)(PPB_BASE + 0x0ED0C)))
        AIRCR_Register = 0x5FA0004;

        // The following line should never execute.
        return true;
    }
    return false;
} // End FactoryReset().


/*******************************************************************************
* SaveCache()
*
* Saves the current NVS cache to EEPROM and returns an indication of whether
* or not the EEPROM save was successful.
*
* Returns:
*   Returns 'true' if all NVS data was successfully stored to EEPROM.
*   Returns 'false' otherwise.
*******************************************************************************/
bool ClockNvs::SaveCache()
{
    EEPROM.put(0, m_Cache);
    return EEPROM.commit();
} // End SaveCache().


/*******************************************************************************
* RestoreCache()
*
* Simply copies the EEPROM data to the local NVS cache.  Does not verify
* the contents of the returned header or payload.
*******************************************************************************/
void ClockNvs::RestoreCache()
{
    EEPROM.get(0, m_Cache);
} // End RestoreCache().


/*******************************************************************************
* CalcChecksum()
*
* Calculates the checksum of the NVS payload and returns its value.
*
* Returns:
*   Always returns the 32-bit value of the checksum of the NVS payload data.
*******************************************************************************/
uint32_t ClockNvs::CalcChecksum()
{
    NvsImage *pImage = (NvsImage *)m_Cache;
    uint8_t *pArray = (uint8_t *)m_Cache;
    uint32_t checksum = 0;
    for (size_t i = sizeof(NvsImage); i < pImage->m_Size; i++)
    {
        checksum += pArray[i];
    }

    return checksum;
} // End CalcChecksum().


/*******************************************************************************
* CacheValid()
*
* Returns an indication of whether the NVS cache data is valid or not.
*
* Returns:
*   Returns 'true' if all header and checksum data are valid.
*   Returns 'false' otherwise.
*******************************************************************************/
bool ClockNvs::CacheValid()
{
    // Validate the NVS header information.
    NvsImage *pImage = (NvsImage *)m_Cache;
    if ((pImage->m_Id != NVS_ID) || (pImage->m_IdComp != ~pImage->m_Id)        ||
        (pImage->m_Version != NVS_VERSION)                                     ||
        (pImage->m_VersionComp != ~NVS_VERSION)                                ||
        (pImage->m_TimeDisplayOfst != sizeof(NvsImage))                        ||
        (pImage->m_TimeDisplayOfstComp != ~pImage->m_TimeDisplayOfst)          ||
        (pImage->m_BacklightOfst != pImage->m_TimeDisplayOfst +
                                                       gTd.GetNvsSize())       ||
        (pImage->m_BacklightOfstComp != ~pImage->m_BacklightOfst)              ||
        (pImage->m_ClockTzOfst != pImage->m_BacklightOfst +
                                                     gBacklight.GetNvsSize())  ||
        (pImage->m_ClockTzOfstComp != ~pImage->m_ClockTzOfst)                  ||
        (pImage->m_TimeMainFontsOfst != pImage->m_ClockTzOfst +
                                        ClockTz::GetNvsSize())                 ||
        (pImage->m_TimeMainFontsOfstComp != ~pImage->m_TimeMainFontsOfst)      ||
        (pImage->m_TimeSecondaryFontsOfst != pImage->m_TimeMainFontsOfst +
                                             TimeMainFonts.GetNvsSize())       ||
        (pImage->m_TimeSecondaryFontsOfstComp !=
                                            ~pImage->m_TimeSecondaryFontsOfst) ||
        (pImage->m_TimeMinorFontsOfst != pImage->m_TimeSecondaryFontsOfst +
                                            TimeMinorFonts.GetNvsSize())       ||
        (pImage->m_TimeMinorFontsOfstComp != ~pImage->m_TimeMinorFontsOfst)    ||
        (pImage->m_ClockWiFiOfst != pImage->m_TimeMinorFontsOfst +
                                              TimeMinorFonts.GetNvsSize())     ||
        (pImage->m_ClockWiFiOfstComp != ~pImage->m_ClockWiFiOfst)              ||
        (pImage->m_Size != m_UserSize)                                         ||
        (pImage->m_SizeComp != ~m_UserSize)                                    ||
        (pImage->m_ChecksumComp != ~pImage->m_Checksum))
    {
        Serial.printf("Cache header invalid.\n");
        return false;
    }

    // If we get here, we're OK if the checksum is valid.
    return CalcChecksum() == pImage->m_Checksum;
} // End CacheValid().

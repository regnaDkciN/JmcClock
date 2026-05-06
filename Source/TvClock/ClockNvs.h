/*******************************************************************************
* ClockNvs.h
*
* Declares the ClockNvs class.  This class supports NVS storage and retrieval
* of clock related settings.
*
* Note: 'ClockNvs::NVS_VERSION' MUST BE CHANGED (INCREMENTED) EACH TIME ANY
*       CHANGE IS MADE TO THE HEADER STRUCTURE, OR THE SIZE OF THE PAYLOAD.
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

#if !defined CLOCKNVS_H
#define CLOCKNVS_H

#include "TvClock.h"        // For miscellaneous clock related objects.


/*******************************************************************************
* NvsImage
*
* This structure declares the header information that is stored in NVS at the
* head of the NVS setting data.
*******************************************************************************/
struct NvsImage
{
    int16_t  m_Id;                          // Identifies the struct as NVS data.
    int16_t  m_IdComp;                      //    Bitwise complement.
    int16_t  m_Version;                     // Structure version.
    int16_t  m_VersionComp;                 //    Bitwise complement.
    size_t   m_TimeDisplayOfst;             // Offset at which TimeDisplay data starts.
    size_t   m_TimeDisplayOfstComp;         //    Bitwise complement.
    size_t   m_BacklightOfst;               // Offset at which Backlight data starts.
    size_t   m_BacklightOfstComp;           //    Bitwise complement.
    size_t   m_ClockTzOfst;                 // Offset at which timezond data starts.
    size_t   m_ClockTzOfstComp;             //    Bitwise complement.
    size_t   m_TimeMainFontsOfst;           // Offset at which main font data starts.
    size_t   m_TimeMainFontsOfstComp;       //    Bitwise complement.
    size_t   m_TimeSecondaryFontsOfst;      // Offset at which secondary font data starts.
    size_t   m_TimeSecondaryFontsOfstComp;  //    Bitwise complement.
    size_t   m_TimeMinorFontsOfst;          // Offset at which minor font data starts.
    size_t   m_TimeMinorFontsOfstComp;      //    Bitwise complement.
    size_t   m_ClockWiFiOfst;               // Offset at which WiFi data starts.
    size_t   m_ClockWiFiOfstComp;           //    Bitwise complement.
    size_t   m_Size;                        // Size in bytes of data, including this header.
    size_t   m_SizeComp;                    //    Bitwise complement.
    uint32_t m_Checksum;                    // Checksum of the payload data.
    uint32_t m_ChecksumComp;                //    Bitwise complement.
}; // End NvsImage.

// Size of the EEPROM partition holding the NVS data.
// Must be a power of 2 greater than or equal to 256.
const size_t NVS_PARTITION_SIZE = 512;

// Tyedef to make working with the NVS cache easier.
typedef uint8_t nvs_cache_t[NVS_PARTITION_SIZE];


/*******************************************************************************
* ClockNvs Class
*
* This singleton class manages saving to NVS and restoring from NVS of the clock
* settings.
*******************************************************************************/
class ClockNvs
{
public:
    /***************************************************************************
    * Instance()
    *
    * Creates and returns a reference to the singleton instance of our class.
    *
    * Returns:
    *   Always returns a reference to the singleton instance of our class.
    ***************************************************************************/
    static ClockNvs &Instance();

    /***************************************************************************
    * Save()
    *
    * Saves all clock setting data to NVS.
    *
    * Returns:
    *   Returns 'true' if successful, and 'false' otherwise.
    ***************************************************************************/
    bool Save();

    /***************************************************************************
    * Restore()
    *
    * Restores all clock setting data from NVS.
    *
    * Returns:
    *   Returns 'true' if successful, and 'false' otherwise.
    ***************************************************************************/
    bool Restore();

    /***************************************************************************
    * FactoryReset()
    *
    * Resets all clock settings to the factory defaults.  Be aware that all
    * current settings will be lost.  As part of the operation, a software
    * reset is performed.
    *
    * Returns:
    *   Returns 'true' if successful, and 'false' otherwise.
    ***************************************************************************/
    bool FactoryReset();

    /***************************************************************************
    * PrintCache()
    *
    * Prints the contents of the cache, which consists of the latest image of
    * the NVS data including the NvsImage header and all payload data.
    * This is normally only used for debugging.
    ***************************************************************************/
    void PrintCache();

private:
    // NVS_ID identifies this as NVS settings data.  Do not change this.
    static const int16_t NVS_ID = 0x2468;

    /**************************************************************************
    * THIS CONSTANT MUST BE CHANGED (INCREMENTED) EACH TIME ANY CHANGE IS MADE
    * TO THE HEADER STRUCTURE, OR THE SIZE OF THE PAYLOAD.
    **************************************************************************/
    static const int16_t NVS_VERSION = 6;

    // Constructor.
    ClockNvs() : m_UserSize(0), m_Initialized(false) { Init(); }

    /***************************************************************************
    * CalcChecksum()
    *
    * Calculates the checksum of the NVS payload and returns its value.
    *
    * Returns:
    *   Always returns the 32-bit value of the checksum of the NVS payload data.
    ***************************************************************************/
    uint32_t CalcChecksum();

    /***************************************************************************
    * CacheValid()
    *
    * Returns an indication of whether the NVS cache data is valid or not.
    *
    * Returns:
    *   Returns 'true' if all header and checksum data are valid.
    *   Returns 'false' otherwise.
    ***************************************************************************/
    bool CacheValid();

    /***************************************************************************
    * Init()
    *
    * Populates the NVS cache header then initializes the EEPROM system if it has
    * not already been initialized.  Sets m_Initialized based on the result of
    * the EEPROM initialization.
    ***************************************************************************/
    void Init();

    /***************************************************************************
    * SaveCache()
    *
    * Saves the current NVS cache to EEPROM and returns an indication of whether
    * or not the EEPROM save was successful.
    *
    * Returns:
    *   Returns 'true' if all NVS data was successfully stored to EEPROM.
    *   Returns 'false' otherwise.
    ***************************************************************************/
    bool SaveCache();

    /***************************************************************************
    * RestoreCache()
    *
    * Simply copies the EEPROM data to the local NVS cache.  Does not verify
    * the contents of the returned header or payload.
    ***************************************************************************/
    void RestoreCache();

    /***************************************************************************
    * FillHeader()
    *
    * Fills in the NvsImage header of the NVS cache.  Does not update the
    * payload or the payload checksum.
    ***************************************************************************/
    void FillHeader();


    size_t       m_UserSize;        // Size of the NVS data, including header.
    bool         m_Initialized;     // 'true' if NVS has been initialized.
    nvs_cache_t  m_Cache;           // The local NVS cache.
}; // End ClockNvs().


#endif // CLOCKNVS_H.

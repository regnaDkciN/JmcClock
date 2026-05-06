/*******************************************************************************
* ClockTz.h
*
* Declares the ClockTz namespace and the TzData class.  These support timezone
* selection.
*
* History:
*   16-JAN-2026 JMC
*      Start.
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

#if !defined CLOCKTZ_H
#define CLOCKTZ_H

#include <math.h>       // For float_t.
#include <cstdint>      // For uint16_t.
#include <vector>       // For vector template.

/*******************************************************************************
* ClockTz namespace.
*******************************************************************************/
namespace ClockTz
{
    /***************************************************************************
    * TzData class
    *
    * Contains info pertaining to timezones throughout the world, including the
    * posix string used for each timezone.
    ***************************************************************************/
    class TzData
    {
    public:
        /***********************************************************************
        * Constructor.
        *
        * Timezone entry constructor.
        *
        * Arguments:
        *   i   The entry identification
        *   l   The string representing the major location of the time zone.
        *   o   The offset from UTC for this entry.
        *   p   The posix string representing the offset and DST handling of
        *       the time zone.
        ***********************************************************************/
        TzData(uint16_t i, const char *l, float o, const char *p) :
               m_Id(i), m_Location(l), m_Offset(o), m_PosixString(p) {}

        // Getters.  There  are no setters since this data is static.
        uint16_t       Id()       const { return m_Id; }
        const char    *Location() const { return m_Location; }
        float_t        Offset()   const { return m_Offset; }
        const char    *PosixStr() const { return m_PosixString; }

    private:
        uint16_t    m_Id;           // Entry identification.
        const char *m_Location;     // Major location of time zone.
        float_t     m_Offset;       // Time zone offset from UTC.
        const char *m_PosixString;  // Posix time zone string.

    }; // End TzData class.


    // Typedefs to make usage easier.
    typedef std::vector<TzData> tz_vec_t;
    typedef tz_vec_t::const_iterator tz_iter_t;

    // Type of sort last done on Tz table.
    enum TzSortType
    {
        SORT_ALPHA = 0,
        SORT_NUMERIC
    }; // End TzSortType.

    // Externs.
    extern tz_vec_t TzTable;              // Table (vector) of timezone information.
    extern void TzSortByLocation();       // Sort TzTable by location.
    extern void TzSortByOffset();         // Sort TzTable by offset from utc.
    extern const tz_iter_t FindTzById(uint16_t id); // Find tz given Id.
    extern size_t GetNvsSize();           // Return size of NVS options.
    extern void SaveNvs(uint8_t *pBuf);   // Save NVS options.
    extern void RestoreNvs(uint8_t *pBuf);// Restore NVS options.
    extern void Init();                   // Initialize timezone data.
    extern void SetTz(uint16_t id);       // Set Tz by Id.

    // Start of options stored in NVS.
    extern TzSortType CurTzSortType;      // Active Tz table sort type.
    extern uint16_t ActiveTzId;           // Active timezone id.
    // End of options stored in NVS.

}; // End namespace ClockTz.

#endif // CLOCKTZ_H.
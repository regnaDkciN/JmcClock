/*******************************************************************************
* ClockTz.cpp
*
* Creates and manages the timezone table.
*
* Reference the following links:
*   https://support.cyberdata.net/portal/en/kb/articles/010d63c0cfce3676151e1f2d5442e311
*   https://www.netburner.com/learn/time-zones-and-daylight-savings-with-olson-and-posix/
*   https://di-mgt.com.au/wclock/help/wclo_tzexplain.html
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

#include "ClockTz.h"    // For our stuff.
#include <algorithm>    // For std::sort
#include <string.h>     // For strcmp().


// ClockTz static variables.
uint16_t            ClockTz::ActiveTzId = 0;                // Active timezone id.
ClockTz::TzSortType ClockTz::CurTzSortType = SORT_ALPHA;    // Active sort type.
static uint16_t DEFAULT_TZ = 27;                            // New York as default time zone.


/*******************************************************************************
* Init()
*
* Initializes the timezone subsystem by setting the sort type and current
* timezone to default values.
*******************************************************************************/
void ClockTz::Init()
{
    TzSortByLocation();
    SetTz(DEFAULT_TZ);
} // End Init().


/*******************************************************************************
* SetTz()
*
* Sets the active timezone.
*
* Arguments:
*   id   This is the of the selected timezone.  See the TzTable vector.
*******************************************************************************/
void ClockTz::SetTz(uint16_t id)
{
    ActiveTzId = id;
    setenv("TZ", FindTzById(id)->PosixStr(), 1);
} // End SetTz().


/*******************************************************************************
* GetNvsSize()
*
* Returns the size in bytes of the timezone settings saved to NVS.
*******************************************************************************/
size_t ClockTz::GetNvsSize()
{
    return sizeof(CurTzSortType) + sizeof(ActiveTzId);
} // End GetNvsSize().


/*******************************************************************************
* SaveNvs()
*
* Saves our non-volatile settings to NVS.
*
* Agruments:
*   pBuf   This is a pointer to the NVS buffer where our NVS data will be stored.
*******************************************************************************/
void ClockTz::SaveNvs(uint8_t *pBuf)
{
    memcpy(pBuf, &CurTzSortType, sizeof(CurTzSortType));
    memcpy(pBuf + sizeof(CurTzSortType), &ActiveTzId, sizeof(ActiveTzId));
} // End SaveNvs().


/*******************************************************************************
* RestoreNvs()
*
* Retrieves previously stored NVS timezone settings.
*
* Agruments:
*   pBuf   This is a pointer to the buffer holding the restored NVS data.
*******************************************************************************/
void ClockTz::RestoreNvs(uint8_t *pBuf)
{
    memcpy(&CurTzSortType, pBuf, sizeof(CurTzSortType));
    memcpy(&ActiveTzId, pBuf + sizeof(CurTzSortType), sizeof(ActiveTzId));
} // End RestoreNvs().


/*******************************************************************************
* TzSortByLocation()
*
* Sorts our timezone table by location name.
*******************************************************************************/
void ClockTz::TzSortByLocation()
{
    CurTzSortType = SORT_ALPHA;
    std::sort(TzTable.begin(), TzTable.end(), [](const TzData &a, const TzData &b)
    { return strcmp(a.Location(), b.Location()) < 0; } );
} // End TzSortByLocation().


/*******************************************************************************
* TzSortByOffset()
*
* Sorts our timezone table by offset from UTC.
*******************************************************************************/
void ClockTz::TzSortByOffset()
{
    CurTzSortType = SORT_NUMERIC;
    std::sort(TzTable.begin(), TzTable.end(), [](const TzData &a, const TzData &b)
    { return a.Offset() < b.Offset(); } );
} // End TzSortByOffset().


/*******************************************************************************
* FindTzById()
*
* Find a timezone given an Id.
*
* Arguments:
*   id - Id of timezone to find.
*
* Returns:
*   Returns an iterator to the specified timezone if found.  Returns NULL otherwise.
*******************************************************************************/
const ClockTz::tz_iter_t ClockTz::FindTzById(uint16_t id)
{
    tz_iter_t entry = TzTable.end();
    // Loop through all entries till we find the one with the specified id.
    for(tz_iter_t i = TzTable.begin(); i != TzTable.end(); ++i)
    {
        if (i->Id() == id)
        {
            // Found it.
            entry = i;
            break;
        }
    }
    return entry;
} // End FindTzById().


/*******************************************************************************
* TzTable
*
* Vector table of worldwide timezones and their associated offset from UTC and
* posix string.  Taken from:
* https://support.cyberdata.net/portal/en/kb/articles/010d63c0cfce3676151e1f2d5442e311
*******************************************************************************/
ClockTz::tz_vec_t ClockTz::TzTable =
{
    {  0, "Alaska",            -9,     "ASKT9AKDT,M3.2.0/2:00:00,M11.1.0/2:00:00" },
    {  1, "America Samoa",     -11,    "NUT11" },
    {  2, "Argentina",         -3,     "ART3" },
    {  3, "Azerbaijan",         4,     "GST-4" },
    {  4, "Bangladesh",         6,     "BST-6" },
    {  5, "Cabo Verde",        -1,     "CVT1" },
    {  6, "Central Australia",  9.5,   "ACST-8:30ACDT,M3.2.0/2:00:00,M11.1.0/2:00:00" },
    {  7, "Chatham Islands",    12.75, "CHAST-11:45CHADT,M3.2.0/2:00:00,M11.1.0/2:00:00" },
    {  8, "Chicago",           -6,     "CST6CDT,M3.2.0/2:00:00,M11.1.0/2:00:00" },
    {  9, "China",              8,     "CST-8" },
    { 10, "Christmas Island",   14,    "LINT-14" },
    { 11, "Denver",            -7,     "MST7MDT,M3.2.0/2:00:00,M11.1.0/2:00:00" },
    { 12, "Eastern Australia",  10,    "AEST-9AEDT,M3.2.0/2:00:00,M11.1.0/2:00:00" },
    { 13, "French Polynesia",  -9.5,   "MART9:30,M3.2.0/2:00:00,M11.1.0/2:00:00" },
    { 14, "Germany",            2,     "CEST-1CET,M3.2.0/2:00:00,M11.1.0/2:00:00" },
    { 15, "Greece",             3,     "MSK-3" },
    { 16, "Greenland",         -2,     "WGST3WGT,M3.2.0/2:00:00,M11.1.0/2:00:00" },
    { 17, "Hawaii",            -10,    "HST11HDT,M3.2.0/2:00:00,M11.1.0/2:00:00" },
    { 18, "Iceland",            0,     "GMT" },
    { 19, "India",              5.5,   "IST-5:30" },
    { 20, "Indonesia",          7,     "WIB-7" },
    { 21, "Iran",               4.5,   "IRDT-3:30IRST,M3.2.0/2:00:00,M11.1.0/2:00:00" },
    { 22, "Japan",              9,     "JST-9" },
    { 23, "Lord Howe Island",   10.5,  "LHST-9:30LHDT,M3.2.0/2:00:00,M11.1.0/2:00:00" },
    { 24, "Los Angeles",       -8,     "PST8PDT,M3.2.0/2:00:00,M11.1.0/2:00:00" },
    { 25, "Myanmar",            6.5,   "MMT-6:30" },
    { 26, "Nepal",              5.75,  "NPT-5:45" },
    { 27, "New York",          -5,     "EST5EDT,M3.2.0/2:00:00,M11.1.0/2:00:00" },
    { 28, "New Zealand",        12,    "ANAT-12" },
    { 29, "Newfoundland",      -2.5,   "NDT3:30NST,M3.2.0/2:00:00,M11.1.0/2:00:00" },
    { 30, "Pakistan",           5,     "UZT-5" },
    { 31, "Phoenix",           -7,     "MST7" },
    { 32, "Solomon Islands",    11,    "SBT-11" },
    { 33, "Tonga",              13,    "TOT-12TOST,M3.2.0/2:00:00,M11.1.0/2:00:00" },
    { 24, "UK",                 1,     "BST0GMT,M3.2.0/2:00:00,M11.1.0/2:00:00" },
    { 35, "US Baker Island",   -12,    "AoE12" },
    { 36, "Western Australia",  8.75,  "ACWST-8:45" }
}; // End TzTable[].

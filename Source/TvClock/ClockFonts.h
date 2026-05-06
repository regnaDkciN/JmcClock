/*******************************************************************************
* ClockFonts.h
*
* Declares the ClockFonts class.  This class handles all the special fonts used
* by the clock.  Fonts are grouped  by size - Large, Small, and Tiny - into
* containers that allow for identification and selection of fonts.
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

#if !defined CLOCKFONTS_H
#define CLOCKFONTS_H

#include <Adafruit_GFX.h>       // Core graphics library.
#include <vector>               // For vector template.


/****************************************************************************
* ClockFontData struct
*
* This structure holds information related to a specific font.
****************************************************************************/
struct ClockFontData
{
    const GFXfont *m_pFont;  // Pointer to the font.
    const char    *m_pName;  // String containing the name of the font.
    bool           m_Active; // 'true' if this font may be selected when iterating.
    const char    *m_Icon;   // X64 encoded font icon.
}; // End ClockFontData struct.

// Forward declarations.
class ClockFonts;


// Typedefs for easier handling of the timezone vector.
typedef std::vector<ClockFontData> font_vec_t;
typedef font_vec_t::iterator font_iter_t;

// The fonts used by the system.
extern font_vec_t PrimaryFonts;         // Main time display font.
extern font_vec_t SecondaryFonts;       // Secondary time display font.
extern font_vec_t TertiaryFonts;        // Minor time display font.


/*******************************************************************************
* ClockFonts class
*
* This class is a container for ClockFontData instances.
*******************************************************************************/
class ClockFonts
{
public:
    /***************************************************************************
    * Constructor
    *
    * Initializes a new ClockFonts instance.
    *
    * Arguments:
    *   pFonts - A pointer to an array of ClockFontData structures that contains
    *            information regarding supported fonts.
    *
    * Note that all font entries are initialized to being 'active'.
    ***************************************************************************/
    ClockFonts(font_vec_t &fonts);

    /***************************************************************************
    // Getters and setters.
    // Note that all Next/Prev methods wrap when reaching end/begin of list.
    ***************************************************************************/
    const char *Name()        const { return m_CurrentFont->m_pName; }
    const GFXfont *Font()     const { return m_CurrentFont->m_pFont; }
    const GFXfont *NextFont()       { return Next()->m_pFont; }
    const GFXfont *PrevFont()       { return Prev()->m_pFont; }
    const GFXfont *NextActiveFont() { return NextActive()->m_pFont; }
    const GFXfont *PrevActiveFont() { return PrevActive()->m_pFont; }
    size_t NumFonts()         const { return m_Fonts.size(); }
    size_t NumActiveFonts()   const { return m_NumActive; }
    int16_t Index()           const { return m_CurrentFont - m_Fonts.begin(); }
    font_iter_t &Current()          { return m_CurrentFont; }
    font_iter_t &Begin()            { m_CurrentFont = m_Fonts.begin(); return m_CurrentFont; }
    font_iter_t &End()              { m_CurrentFont = (m_Fonts.end() - 1); return m_CurrentFont; }
    bool IsFirst()            const { return m_CurrentFont == m_Fonts.begin(); }
    bool IsLast()             const { return m_CurrentFont == (m_Fonts.end() - 1); }
    bool IsEnd()              const { return m_CurrentFont == m_Fonts.end(); }
    bool IsActive()           const { return m_CurrentFont->m_Active; }
    void SetIndex(size_t i)         { m_CurrentFont = m_Fonts.begin() + i; }
    size_t GetNvsSize()       const { return sizeof(int16_t) + (sizeof(bool) * m_Fonts.size()); }
    void SaveNvs(uint8_t *pBuf);
    void RestoreNvs(uint8_t *pBuf);
    void Print();

    // Miscellaneous support methods.

    // Pre-increment operator.  Does not wrap.
    font_iter_t &operator++() { return ++m_CurrentFont; }

    // Pre-decrement.  Does not wrap.
    font_iter_t &operator--() { return --m_CurrentFont; }

    // Subscript operator.
    ClockFonts &operator[](unsigned i)
    {
        if (i < m_Fonts.size())
        {
            m_CurrentFont = m_Fonts.begin() + i;
        }
        return *this;
    } // End operator[].

    /***************************************************************************
    * Next()
    *
    * Increments the current index and wraps if needed.
    *
    * Returns:
    *   Returns the index of the new current font.
    ***************************************************************************/
    font_iter_t &Next();

    /***************************************************************************
    * Prev()
    *
    * Deccrements the current index and wraps if needed.
    *
    * Returns:
    *   Returns the index of the new current font.
    ***************************************************************************/
    font_iter_t &Prev();

    /***************************************************************************
    * NextActive()
    *
    * Bump to next font that is marked as active, wrap if necessary.
    *
    * Returns:
    *   Returns the index of the new current font.
    ***************************************************************************/
    font_iter_t &NextActive();

    /***************************************************************************
    * PrevActive()
    *
    * Bump to next previous font that is marked as active, wrap if necessary.
    *
    * Returns:
    *   Returns the index of the new current font.
    ***************************************************************************/
    font_iter_t &PrevActive();

    /***************************************************************************
    * FirstActive()
    *
    * Find the first font that is marked as active and make it the current font.
    *
    * Returns:
    *   Returns the index of the new current font.
    ***************************************************************************/
    font_iter_t &FirstActive();

    /***************************************************************************
    * SetActive()
    *
    * Sets the current font to active/inactive.
    *
    * Arguments:
    *   active - 'true' to set active, 'false' otherwise.
    ***************************************************************************/
    void SetActive(bool active);

private:

    // Unimplimented methods.
    ClockFonts();
    ClockFonts(ClockFonts &r);
    ClockFonts &operator=(ClockFonts &r);

    font_vec_t    &m_Fonts;         // Reference to the array of font data.
    int16_t        m_NumActive;     // Number of fonts with 'active' flag set 'true'.
    font_iter_t    m_CurrentFont;   // Index of current font.

}; // End ClockFonts.


#endif // CLOCKFONTS_H.
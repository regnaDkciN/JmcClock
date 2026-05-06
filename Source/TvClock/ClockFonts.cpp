/*******************************************************************************
* ClockFonts.cpp
*
* Creates ClockFonts instances for all fonts used in the clock.
*
*-------------------------------------------------------------------------------
*
* Each of the fonts used by the clock was created from existing general purpose
* fonts using the "fontconvert" tool mentioned in the Adafruit graphics library
* instructions at:
*   https://learn.adafruit.com/adafruit-gfx-graphics-library/using-fonts
* the following video is also helpful:
*   https://www.youtube.com/watch?v=L8MmTISmwZ8
*
* The FONTCONVERT instructions were basically followed:
*    1. Copy the desired font to your working folder.
*    2. Launch a command prompt from your working folder (cmd).
*    3. Enter the following command on the command line:
*          FontConvert FontFile.ttf DesiredFontSize StartingASCIIChar EndingASCIIChar > DesiredName.h
*       In this case, the clock fonts were all of varying sizes, and different
*       DesiredFontSize values were tried for each until a useable size was found.
*       For example, the following command line was used to create the Miama
*       font below:
*          FontConvert Miama.ttf 95 0 : > Miama95pt7b.h
*       Note that for the clock display, only the ascii characters "0" through
*       "9" and ":" were needed.  Luckily the ":" ascii value occurs immediately
*       after the "9" character, so no unneeded characters were included.
*     4. Move the new .h file to a folder you will use for your project or
*        the Adafruit_GFX\Font folder.
*     5. Include the font name in your code (as is done below).
*
*-------------------------------------------------------------------------------
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
#include "ClockFonts.h"

// The icons used to describe each font below display "12:59" as a preview of
// how the font will look on the clock font web page.
// If a smaller memory size is neede, use the following include instead:
// #include "ClockIcons.h"      // For "12" icons.
// The smaller icons display only "12" in the specified font.
#include "ClockExtendedIcons.h"     // For "12:59" icons.

// Large fonts.
#include "ClockFonts/FRSCRIPT96pt7b.h"             //  0 French Script
#include "ClockFonts/Rochester_Regular72pt7b.h"    //  1 Rochester
#include "ClockFonts/tt0939m_118pt7b.h"            //  2 Typo Upright BT
#include "ClockFonts/ITCEDSCR75pt7b.h"             //  3 Edwardian Script
#include "ClockFonts/KUNSTLER108pt7b.h"            //  4 Kunstler Script

#include "ClockFonts/PALSCRI110pt7b.h"             //  5 Palace Script MT
#include "ClockFonts/Miama95pt7b.h"                //  6 Miama
#include "ClockFonts/RedMist_Regular78pt7b.h"      //  7 Red Mist
#include "ClockFonts/Rumburak80pt7b.h"             //  8 Rumburak
#include "ClockFonts/VIVALDII81pt7b.h"             //  9 Vivaldi

#include "ClockFonts/AlexBrush_Regular62pt7b.h"    // 10 Alex Brush
#include "ClockFonts/segoesc50pt7b.h"              // 11 Segoe Script
#include "ClockFonts/IMPRISHA72pt7b.h"             // 12 Imprint MT Shadow
#include "ClockFonts/Greek_i52pt7b.h"              // 13 Greek Diner
#include "ClockFonts/Playback75pt7b.h"             // 14 Playback

#include "ClockFonts/Member64pt7b.h"               // 15 Member
#include "ClockFonts/neon251pt7b.h"                // 16 Neon Lights
#include "ClockFonts/Skinny__56pt7b.h"             // 17 Skinny
#include "ClockFonts/NIAGENG86pt7b.h"              // 18 Niagara Engraved
#include "ClockFonts/quartz68pt7b.h"               // 19 Quartz

#include "ClockFonts/lcd80pt7b.h"                  // 20 LCD
#include "ClockFonts/OLDENGL72pt7b.h"              // 21 Old English MT
#include "ClockFonts/Manorly_60pt7b.h"             // 22 Manorly
#include "ClockFonts/Hombre__72pt7b.h"             // 23 Hombre
#include "ClockFonts/GLECB75pt7b.h"                // 24 Gloucester MT Extra Condensed

#include "ClockFonts/MOD2072pt7b.h"                // 25 Modern No. 20
#include "ClockFonts/GOUDOS72pt7b.h"               // 26 Goudy Old Style
#include "ClockFonts/SERomanc56pt7b.h "            // 27 SERomanc
#include "ClockFonts/SETxt64pt7b.h"                // 28 SETxt
#include "ClockFonts/Notram__58pt7b.h"             // 29 Notram

#include "ClockFonts/NovaCut62pt7b.h"              // 30 Nova Cut
#include "ClockFonts/ONYX80pt7b.h"                 // 31 Onyx
#include "ClockFonts/Alfredo_75pt7b.h"             // 32 Alfredo
#include "ClockFonts/aliee1368pt7b.h"              // 33 Alien Encounters
#include "ClockFonts/almosnow75pt7b.h"             // 34 Almonte Snow

#include "ClockFonts/Ameth___70pt7b.h"             // 35 Amethyst
#include "ClockFonts/Brand___50pt7b.h"             // 36 Brandish
#include "ClockFonts/BROADW56pt7b.h"               // 37 Broadway
#include "ClockFonts/CALLI___68pt7b.h"             // 38 Calligraphic
#include "ClockFonts/CURLZ___72pt7b.h"             // 39 Curlz MT

#include "ClockFonts/gazzarelli46pt7b.h"           // 40 Gazzarelli
#include "ClockFonts/JOKERMAN56pt7b.h"             // 41 Jokerman
#include "ClockFonts/Mycalc__58pt7b.h"             // 42 Mycalc
#include "ClockFonts/Pirate__56pt7b.h"             // 43 Pirate
#include "ClockFonts/RAVIE48pt7b.h"                // 44 Ravie

#include "ClockFonts/SNAP____46pt7b.h"             // 45 Snap ITC
#include "ClockFonts/Tarzan__62pt7b.h"             // 46 Tarzan
#include "ClockFonts/ALGER64pt7b.h"                // 47 Algerian
#include "ClockFonts/AmaticSC_Regular72pt7b.h"     // 48 Amatic
#include "ClockFonts/Archicoco56pt7b.h"            // 49 Archicoco

#include "ClockFonts/BALTH___72pt7b.h"             // 50 Balthazar
#include "ClockFonts/bnjinx65pt7b.h"               // 51 Jinx
#include "ClockFonts/candles_58pt7b.h"             // 52 Candles
#include "ClockFonts/Deneane_63pt7b.h"             // 53 Deneane
#include "ClockFonts/digifit52pt7b.h"              // 54 Digifit

#include "ClockFonts/Enliven_52pt7b.h"             // 55 Enliven
#include "ClockFonts/flubber63pt7b.h"              // 56 Flubber
#include "ClockFonts/tt1018m_72pt7b.h"             // 57 Freehand
#include "ClockFonts/Limou___68pt7b.h"             // 58 Limousine
#include "ClockFonts/mael____63pt7b.h"             // 59 Mael

#include "ClockFonts/POORICH75pt7b.h"              // 60 Poor Richard
#include "ClockFonts/Steppes54pt7b.h"              // 61 Steppes
#include "ClockFonts/Syirenata48pt7b.h"            // 62 Syirenata
#include "ClockFonts/VINERITC64pt7b.h"             // 63 Viner Hand


// Small fonts.
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSansOblique18pt7b.h>
#include <Fonts/FreeSerif18pt7b.h>
#include <Fonts/FreeSerifBold18pt7b.h>
#include <Fonts/FreeSerifBoldItalic18pt7b.h>
#include <Fonts/FreeSerifItalic18pt7b.h>


// Tiny fonts.
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansOblique9pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/FreeSerifBold9pt7b.h>
#include <Fonts/FreeSerifBoldItalic9pt7b.h>
#include <Fonts/FreeSerifItalic9pt7b.h>


/*******************************************************************************
* PrimaryFonts
*
* Vector of fonts that are used to display HH:MM data. These fonts only contain
* data for the digits 0-9, and ':".
*******************************************************************************/
font_vec_t PrimaryFonts =
{
    { &FRSCRIPT96pt7b,          "French Script",    true, FrenchScriptIcon },
    { &Rochester_Regular72pt7b, "Rochester",        true, RochesterIcon },
    { &tt0939m_118pt7b,         "Typo Upright",     true, TypoUprightIcon },
    { &ITCEDSCR75pt7b,          "Edwardian Script", true, EdwardianScriptIcon },
    { &KUNSTLER108pt7b,         "Kunstler",         true, KunstlerIcon },

    { &PALSCRI110pt7b,          "Palace Script",    true, PalaceScriptIcon },
    { &Miama95pt7b,             "Miama",            true, MiamaIcon },
    { &RedMist_Regular78pt7b,   "Red Mist",         true, RedMistIcon },
    { &Rumburak80pt7b,          "Rumburak",         true, RumburakIcon },
    { &VIVALDII81pt7b,          "Vivaldi",          true, VivaldiIcon },

    { &AlexBrush_Regular62pt7b, "Alex Brush",       true, AlexBrushIcon },
    { &segoesc50pt7b,           "Segoe Script",     true, SegoeScriptIcon },
    { &IMPRISHA72pt7b,          "Imprint Shadow",   true, ImprintShadowIcon },
    { &Greek_i52pt7b,           "Greek Diner",      true, GreekDinerIcon },
    { &Playback75pt7b,          "Playback",         true, PlaybackIcon },

    { &Member64pt7b,            "Member",           true, MemberIcon },
    { &neon251pt7b,             "Neon Lights",      true, NeonLightsIcon },
    { &Skinny__56pt7b,          "Skinny",           true, SkinnyIcon },
    { &NIAGENG86pt7b,           "Niagara Engraved", true, NiagaraEngravedIcon },
    { &quartz68pt7b,            "Quartz",           true, QuartzIcon },

    { &lcd80pt7b,               "LCD",              true, LCDIcon },
    { &OLDENGL72pt7b,           "Old English",      true, OldEnglishIcon },
    { &Manorly_60pt7b,          "Manorly",          true, ManorlyIcon },
    { &Hombre__72pt7b,          "Hombre",           true, HombreIcon },
    { &GLECB75pt7b,             "Gloucester",       true, GloucesterIcon },

    { &MOD2072pt7b,             "Modern",           true, ModernIcon },
    { &GOUDOS72pt7b,            "Goudy Old Style",  true, GoudyOldStyleIcon },
    { &SERomanc56pt7b,          "SERomanc",         true, SERomancIcon },
    { &SETxt64pt7b,             "SETxt",            true, SETxtIcon },
    { &Notram__58pt7b,          "Notram",           true, NotramIcon },

    { &NovaCut62pt7b,           "Nova Cut",         true, NovaCutIcon },
    { &ONYX80pt7b,              "Onyx",             true, OnyxIcon },
    { &Alfredo_75pt7b,          "Alfredo",          true, AlfredoIcon },
    { &aliee1368pt7b,           "Alien Encounters", true, AlienEncountersIcon },
    { &almosnow75pt7b,          "Almonte Snow",     true, AlmonteSnowIcon },

    { &Ameth___70pt7b,          "Amethyst",         true, AmethystIcon },
    { &Brand___50pt7b,          "Brandish",         true, BrandishIcon },
    { &BROADW56pt7b,            "Broadway",         true, BroadwayIcon },
    { &CALLI___68pt7b,          "Calligraphic",     true, CalligraphicIcon },
    { &CURLZ___72pt7b,          "Curlz",            true, CurlzIcon },

    { &gazzarelli46pt7b,        "Gazzarelli",       true, GazzarelliIcon },
    { &JOKERMAN56pt7b,          "Jokerman",         true, JokermanIcon },
    { &Mycalc__58pt7b,          "Mycalc",           true, MycalcIcon },
    { &Pirate__56pt7b,          "Pirate",           true, PirateIcon },
    { &RAVIE48pt7b,             "Ravie",            true, RavieIcon },

    { &SNAP____46pt7b,          "Snap",             true, SnapIcon },
    { &Tarzan__62pt7b,          "Tarzan",           true, TarzanIcon },
    { &ALGER64pt7b,             "Algerian",         true, AlgerianIcon },
    { &AmaticSC_Regular72pt7b,  "Amatic",           true, AmaticIcon },
    { &Archicoco56pt7b,         "Archicoco",        true, ArchicocoIcon },

    { &BALTH___72pt7b,          "Balthazar",        true, BalthazarIcon },
    { &bnjinx65pt7b,            "Jinx",             true, JinxIcon },
    { &candles_58pt7b,          "Candles",          true, CandlesIcon },
    { &Deneane_63pt7b,          "Deneane",          true, DeneaneIcon },
    { &digifit52pt7b,           "Digifit",          true, DigifitIcon },

    { &Enliven_52pt7b,          "Enliven",          true, EnlivenIcon },
    { &flubber63pt7b,           "Flubber",          true, FlubberIcon },
    { &tt1018m_72pt7b,          "Freehand",         true, FreehandIcon },
    { &Limou___68pt7b,          "Limousine",        true, LimousineIcon },
    { &mael____63pt7b,          "Mael",             true, MaelIcon },

    { &POORICH75pt7b,           "Poor Richard",     true, PoorRichardIcon },
    { &Steppes54pt7b,           "Steppes",          true, SteppesIcon },
    { &Syirenata48pt7b,         "Syirenata",        true, SyirenataIcon },
    { &VINERITC64pt7b,          "Viner Hand",       true, VinerHandIcon }
}; // End PrimaryFonts.


/*******************************************************************************
* SecondaryFonts
*
* Vector of fonts that are used to for displaying secondary information like
* day of week and date.
*******************************************************************************/
font_vec_t SecondaryFonts =
{
    {&FreeSans18pt7b,            "Sans Serif",      false, SansIcon },
    {&FreeSansOblique18pt7b,     "Sans Oblique",    false, SansObliqueIcon },
    {&FreeSerif18pt7b,           "Serif",           false, SerifIcon },
    {&FreeSerifBold18pt7b,       "Serif Bold",      false, SerifBoldIcon },
    {&FreeSerifBoldItalic18pt7b, "Serif Bold Ital", false, SerifBoldItalicIcon },
    {&FreeSerifItalic18pt7b,     "Serif Italic",    true,  SerifItalicIcon }
}; // End SecondaryFonts.


/*******************************************************************************
* TertiaryFonts
*
* Vector of fonts that are used to for displaying tertiary information like
* AM/PM and timezone.
*
* NOTE: This list must be identical to the SecondaryFonts vector, but use
*       9 point fonts in place of 18 point.
*******************************************************************************/
font_vec_t TertiaryFonts =
{
    {&FreeSans9pt7b,             "Sans Serif",      false, SansIcon },
    {&FreeSansOblique9pt7b,      "Sans Oblique",    false, SansObliqueIcon },
    {&FreeSerif9pt7b,            "Serif",           false, SerifIcon },
    {&FreeSerifBold9pt7b,        "Serif Bold",      false, SerifBoldIcon },
    {&FreeSerifBoldItalic9pt7b,  "Serif Bold Ital", false, SerifBoldItalicIcon },
    {&FreeSerifItalic9pt7b,      "Serif Italic",    true,  SerifItalicIcon }
}; // Ennd TertiaryFonts.


/*******************************************************************************
* Constructor
*
* Initializes a new ClockFonts instance.
*
* Arguments:
*   pFonts - A pointer to an array of ClockFontData structures that contains
*            information regarding supported fonts.
*
* Note that all font entries are initialized to being 'active'.
*******************************************************************************/
ClockFonts::ClockFonts(font_vec_t &fonts) :
                       m_Fonts(fonts), m_NumActive(0), m_CurrentFont(fonts.begin())
{
    // Count the number of active fonts.
    for (ClockFontData &f : m_Fonts)
    {
        if (f.m_Active)
        {
            m_NumActive++;
        }
    }

    // Select the first active font.
    FirstActive();
} // End Constructor.


/*******************************************************************************
* Next()
*
* Increments the current index and wraps if needed.
*
* Returns:
*   Returns the index of the new current font.
*******************************************************************************/
font_iter_t &ClockFonts::Next()
{
    if (m_CurrentFont == (m_Fonts.end() - 1))
    {
        m_CurrentFont = m_Fonts.begin();
    }
    else
    {
        ++m_CurrentFont;
    }
    return m_CurrentFont;
} // End Next().


/*******************************************************************************
* Prev()
*
* Deccrements the current index and wraps if needed.
*
* Returns:
*   Returns the index of the new current font.
*******************************************************************************/
font_iter_t &ClockFonts::Prev()
{
    if (m_CurrentFont == m_Fonts.begin())
    {
        m_CurrentFont = (m_Fonts.end() - 1);
    }
    else
    {
        --m_CurrentFont;
    }
    return m_CurrentFont;
} // End Prev().


/*******************************************************************************
* NextActive()
*
* Bump to next font that is marked as active, wrap if necessary.
*
* Returns:
*   Returns the index of the new current font.
*******************************************************************************/
font_iter_t &ClockFonts::NextActive()
{
    // Only change fonts if more than one is active.
    if(m_NumActive)
    {
        // Loop through fonts to find an active one.
        do
        {
            if (m_CurrentFont == (m_Fonts.end() - 1))
            {
                m_CurrentFont = m_Fonts.begin();
            }
            else
            {
                ++m_CurrentFont;
            }
        } while (!IsActive());
    }
    return m_CurrentFont;
} // End NextActive().


/*******************************************************************************
* PrevActive()
*
* Bump to first previous font that is marked as active, wrap if necessary.
*
* Returns:
*   Returns the index of the new current font.
*******************************************************************************/
font_iter_t &ClockFonts::PrevActive()
{
    // Only change fonts if more than one is active.
    if(m_NumActive)
    {
        // Loop through fonts to find an active one.
        do
        {
            if (m_CurrentFont == m_Fonts.begin())
            {
                m_CurrentFont = (m_Fonts.end() - 1);
            }
            else
            {
                --m_CurrentFont;
            }
        } while (!IsActive());
    }
    return m_CurrentFont;
} // End PrevActive().


/*******************************************************************************
* FirstActive()
*
* Find the first font that is marked as active and make it the current font.
*
* Returns:
*   Returns the index of the new current font.
*******************************************************************************/
font_iter_t &ClockFonts::FirstActive()
{
    size_t index = 0;
    for (ClockFontData &f : m_Fonts)
    {
        if (f.m_Active)
        {
            break;;
        }
        index++;
    }
    SetIndex(index);
    return m_CurrentFont;
} // End FirstActive().


/*******************************************************************************
* SetActive()
*
* Sets the current font to active/inactive.
*
* Arguments:
*   active - 'true' to set active, 'false' otherwise.
*******************************************************************************/
void ClockFonts::SetActive(bool active)
{
    // Set the current font as specified, keeping track of the active count.
    if (active && !IsActive())
    {
        m_CurrentFont->m_Active = true;
        ++m_NumActive;
    }
    else if (!active && IsActive())
    {
        m_CurrentFont->m_Active = false;
        --m_NumActive;
    }
} // End SetActive().


/*******************************************************************************
* SaveNvs()
*
* Saves our non-volatile settings to NVS.
*
* Agruments:
*   pBuf   This is a pointer to the NVS buffer where our NVS data will be stored.
*******************************************************************************/
void ClockFonts::SaveNvs(uint8_t *pBuf)
{
    // Save the index of the current font.
    int16_t activeFontIndex = Index();
    int16_t *pIndex = (int16_t *)pBuf;

    // Save the index of the current font.
    *pIndex++ = activeFontIndex;

    // Save the active state of each font.
    bool *pBool = (bool *)pIndex;
    for (ClockFontData &f : m_Fonts)
    {
        *pBool++ = f.m_Active;
    }

    // Restore the current font.
    SetIndex(activeFontIndex);
} // End SaveNvs().


/*******************************************************************************
* RestoreNvs()
*
* Retrieves previously stored NVS font settings.
*
*   pBuf   This is a pointer to the NVS buffer containing our restored NVS
*          values.
*******************************************************************************/
void ClockFonts::RestoreNvs(uint8_t *pBuf)
{
    // Retrieve the saved index of the current font.
    int16_t *pIndex = (int16_t *)pBuf;
    int16_t activeFontIndex = *pIndex++;

    // Restore the active state of ezch font.
    bool *pBool = (bool *)pIndex;
    m_NumActive = 0;
    for (ClockFontData &f : m_Fonts)
    {
        f.m_Active = *pBool++;
        m_NumActive += f.m_Active;
    }

    // Set the current font based on the restored index.
    SetIndex(activeFontIndex);
} // End RestoreNvs().


/*******************************************************************************
* Print()
*
* Print the index of the current font, the number of active fonts, and the
* active state for each font in the set.  This is mainly used for debugging.
*******************************************************************************/
void ClockFonts::Print()
{
    Serial.printf("Active index: %d   NumActive: %d\n", Index(), m_NumActive);
    for (ClockFontData &f : m_Fonts)
    {
        Serial.printf("%d\n", f.m_Active);
    }
    Serial.println();
} // End Print().



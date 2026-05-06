/******************************************************************************
* GuiScreens.cpp
*
* Contains code that implements all screens that are displayed on the clock.
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

#include <Fonts/FreeSerifBold18pt7b.h>  // Medium font for displaying settings.
#include <Fonts/FreeSerifBold9pt7b.h>   // Small font for displaying settings.
#include "GuiScreens.h"                 // Our include file.
#include "ClockWiFi.h"                  // WiFi object.
#include "ClockNvs.h"                   // NVS object.
#include "ClockWebServer.h"             // Our web server implementation.
#include "Backlight.h"                  // The clock backlight class.


// Define some colors that are used by the screens.
#define BLACK           (0x0000)
#define BLUE            (0x001F)
#define BLUE_MID        (0x000F)
#define BLUE_DARK       (0x0007)
#define RED             (0xF800)
#define RED_MID         (0x7800)
#define RED_DARK        (0x3800)
#define GREEN           (0x07E0)
#define GREEN_MID       (0x03E0)
#define GREEN_DARK      (0x01E0)
#define CYAN            (0x07FF)
#define MAGENTA         (0xF81F)
#define YELLOW          (0xFFE0)
#define WHITE           (0xFFFF)
#define GRAY            (0x4208)

// Setup some shortcut defines.
#define SYS_FONT_18 FreeSerifBold18pt7b
#define SYS_FONT_9  FreeSerifBold9pt7b
#define SYS_BTN_CLR GREEN
#define SYS_TXT_CLR GREEN
#define SYS_DRK_CLR GREEN_DARK

// BACKGROUND is always the first object on a screen.
static const size_t INDEX_BGND = 0;

// Small string buffer for miscellaneous use.
static const size_t STR_BUF_SIZE = 40;
static char StrBuf[STR_BUF_SIZE];

// Screen timeout set to 5 minutes.
static const int32_t TIMEOUT_MS = 60 * 5 * 1000;



/*******************************************************************************
* InputBuffer Class
*
* This class manages an input buffer object which is used to input user data
* (generally from the touch keypad).
*******************************************************************************/
class InputBuffer
{
public:
    // Constructor.
    InputBuffer() : m_Index(0)
    { memset(m_Buf, 0, MAX_BUF_LEN); }

    // Add a character to the buffer.
    void AddChar(char c)
    {
        switch (c)
        {
        case '\n':      // Ignore newline.
            break;
        case '\b':      // Handle back space.
            if (m_Index)
            {
                m_Buf[--m_Index] = '\0';
            }
            break;
        default:        // Handle normal characters.
            if (m_Index < MAX_BUF_LEN - 1)
            {
                m_Buf[m_Index++] = c;
                m_Buf[m_Index] = '\0';
            }
            break;
        }
    } // End AddChar().

    // Add a string to the buffer.
    void AddString(const char *pStr)
    {
        Clear();
        size_t len = strlen(pStr);
        if (len >= MAX_BUF_LEN - 1)
        {
            len = MAX_BUF_LEN - 1;
        }
        strlcpy(m_Buf, pStr, MAX_BUF_LEN - 1);
        m_Index = len;
    } // End AddString().

    // Clear the input buffer.
    void Clear() { memset(m_Buf, 0, MAX_BUF_LEN); m_Index = 0; }

    // Return a pointer to the buffer.
    char *GetBuf()  { return m_Buf; }

    // Maximum buffer size.
    static const size_t MAX_BUF_LEN = 33;
private:
    char   m_Buf[MAX_BUF_LEN];  // The input buffer.
    size_t m_Index;             // Index of next character to add.
}; // End InputBuffer class.


/*******************************************************************************
* ShowStatus()
*
* Displays a status screen with text centered on the screen.
*
* Arguments:
*   pMsg     - Pointer to the message to be displayed.
*   txtColor - Color (rgb16_t) of the message to be displayed.
*   pFont    - Pointer to the font to use for the message.
*   txtSize  - Size multiplier for the message being displayed.
*   bgColor  - Screen background color (rgb16_t).
*******************************************************************************/
static void ShowStatus(const char *pMsg, rgb16_t txtColor, const GFXfont *pFont,
                       uint16_t txtSize, rgb16_t bgColor)
{
    // Variables used for sizing the message text.
    int16_t x1 = 0;
    int16_t y1 = 0;
    uint16_t w = 0;
    uint16_t h = 0;

    // Fill the background and setup the font size and color.
    gTft.fillScreen(bgColor);
    gTft.setTextColor(txtColor);
    gTft.setFont(pFont);
    gTft.setTextSize(txtSize);
    gTft.setTextWrap(false);

    // Calculate the size of the message text.
    gTft.getTextBounds(pMsg, 0, 0, &x1, &y1, &w, &h);

    // Center the message within the screen and display it.
    gTft.setCursor((gTft.width() - w) / 2 - x1, (gTft.height() - h) / 2 - y1);
    gTft.print(pMsg);
} // End ShowStatus().



/*******************************************************************************
********************************************************************************
* Main Time Screen (Home Screen)
*
* This is the main clock screen.  It displays the time and contains several
* touch fields:
*   - Left side of display changes the font to the previous active font.
*   - Right side of display changes the font to the next active font.
*   - Center of the display selects the main SETUP screen.
*   - Top of the display increases brightness.
*   - Bottom of the display decreases brightness.
********************************************************************************
*******************************************************************************/
static void PrevMainFont(int16_t);      // Select the previous font.
static void NextMainFont(int16_t);      // Select the next font.
static void ExMainScreen(int16_t);      // Display the main (time) screen.
static void SelSetupScreen(int16_t);    // Select the setup screen.
static void IncBrightness(int16_t);     // Increment screen brightness.
static void DecBrightness(int16_t);     // Decrement screen brightness.

static const uint16_t BRIGHTNESS_INCREMENT = 10;

static scr_vec_t ScrMain =
{ //      obj          tX0  tX1  tY0  tY1  oX0   oW  oY0   oH   oE  oC         lX0  lY0  txt            fs       lc           onClick          oLoPct  oHiPct   val font        enabled touchIndex
    { BACKGROUND,        0,   0,   0,   0,   0,   0,   0,   0,   1, BLACK,       0,   0, NULL,           0,  BLACK,           ExMainScreen,      0,     0,     0, NULL,         true },// 0.
    { BUTTON_RECT,       0, 319,   0,  40,   0,   0,   0,   0,   0, BLACK,       0,   0, NULL,           1,  BLACK,           IncBrightness,     0,     0,     0, NULL,         true },// 1.
    { BUTTON_RECT,       0, 319, 200, 239,   0,   0,   0,   0,   0, BLACK,       0,   0, NULL,           1,  BLACK,           DecBrightness,     0,     0,     0, NULL,         true },// 1.
    { BUTTON_RECT,       0,  80,   0, 239,   0,   0,   0,   0,   0, BLACK,       0,   0, NULL,           1,  BLACK,           PrevMainFont,      0,     0,     0, NULL,         true },// 1.
    { BUTTON_RECT,     240, 319,   0, 239,   0,   0,   0,   0,   0, BLACK,       0,   0, NULL,           1,  BLACK,           NextMainFont,      0,     0,     0, NULL,         true },// 2.
    { BUTTON_RECT,      85, 235,   0, 239,   0,   0,   0,   0,   0, BLACK,       0,   0, NULL,           1,  BLACK,           SelSetupScreen,    0,     0,     0, NULL,         true },// 3.
}; // End ScrMain[].

// Select the main screen.
static void SelMainScreen(int16_t)
{
    gGui.DrawScreen(&ScrMain, SCR_REDRAW);
} // End SelMainScreen().

// Initialize the screen system by displaying the main screen.  Visible globally.
void InitScreens()
{
    SelMainScreen(0);
} // End InitScreens().

// Returns a pointer to the main screen.  Visible globally.
scr_vec_t *GetHomeScreen()
{
    return &ScrMain;
} // End GetHomeScreen().

// Returns an indication of whether the current screen is the home screen.
// Visible globally.
bool IsHomeScreen()
{
    return &ScrMain == gGui.GetScreenPtr();
} // End IsHomeScreen().

// Display the main (time) screen.
static void ExMainScreen(int16_t)
{
    gTd.DisplayTime();
} // End ExMainScreen().

// Select the previous font.
static void PrevMainFont(int16_t)
{
    TimeMainFonts.PrevActive();
    Serial.printf("%d - %s\n", TimeMainFonts.Index(), TimeMainFonts.Name());
} // End PrevMainFont().

// Select the next font.
static void NextMainFont(int16_t)
{
    TimeMainFonts.NextActive();
    Serial.printf("%d - %s\n", TimeMainFonts.Index(), TimeMainFonts.Name());
} // End NextMainFont().

// Increment screen brightness.
static void IncBrightness(int16_t)
{
    gBacklight.SetBrightness(gBacklight.GetBrightness() + BRIGHTNESS_INCREMENT);
} // End IncBrightness().

// Decrement screen brightness.
static void DecBrightness(int16_t)
{
    gBacklight.SetBrightness(gBacklight.GetBrightness() - BRIGHTNESS_INCREMENT);
} // End IncBrightness().



/*******************************************************************************
********************************************************************************
* Setup Selection Screen (main setup screen)
*
* This is the main setup screen.  It allows for selection of settings to change.
********************************************************************************
*******************************************************************************/
static void SelClkOpts1(int16_t p);         // Display the CLOCK setup screen.
static void SelWiFiScreen(int16_t p);       // Display the WiFi setup screen.
static void ExRstrSetup(int16_t p);         // RESTORE user settings.
static void SelColorCyclingScreen(int16_t p); // Display the COLOR setup screen.
static void SelTzScreen(int16_t p);         // Display the TIMEZONE setup screen.
static void ExSaveSetup(int16_t p);         // SAVE user settings.
static void ExResetSetup(int16_t p);        // Display the FACTORY RESET screen.
static void SelMainFontScreen(int16_t p);   // Display the FONT setup screen.

static scr_vec_t ScrSetup =
{ //      obj          tX0  tX1  tY0  tY1  oX0   oW  oY0   oH   oE  oC         lX0  lY0  txt            fs       lc           onClick          oLoPct  oHiPct   val font        enabled touchIndex
    { BACKGROUND,        0,   0,   0,   0,   0,   0,   0,   0,   0, BLACK,       0,   0,  NULL,          0, BLACK,            NULL,               0,    0,       0, NULL,         true },// 0.
    { LABEL,             0,   0,   0,   0,   0, 320,   0,  40,   0, BLACK,       0,   0,  "S E T U P",   1, SYS_TXT_CLR,      NULL,               0,    0,       0, &SYS_FONT_18, true },// 1.
    { BUTTON_RECT,      40, 140,  40,  70,  40, 100,  40,  30,   5, WHITE,       0,   0,  "CLOCK",       1, BLACK,            SelClkOpts1,        0,    0,       0, &SYS_FONT_9,  true },// 2.
    { BUTTON_RECT,      40, 140,  80, 110,  40, 100,  80,  30,   5, WHITE,       0,   0,  "FONTS",       1, BLACK,            SelMainFontScreen,  0,    0,       0, &SYS_FONT_9,  true },// 3.
    { BUTTON_RECT,      40, 140, 120, 150,  40, 100, 120,  30,   5, WHITE,       0,   0,  "WIFI",        1, BLACK,            SelWiFiScreen,      0,    0,       0, &SYS_FONT_9,  true },// 4.
    { BUTTON_RECT,      40, 140, 160, 190,  40, 100, 160,  30,   5, WHITE,       0,   0,  "RESTORE",     1, BLACK,            ExRstrSetup,        0,    0,       0, &SYS_FONT_9,  true },// 5.
    { BUTTON_RECT,     180, 280,  40,  70, 180, 100,  40,  30,   5, WHITE,       0,   0,  "COLORS",      1, BLACK,            SelColorCyclingScreen,0,  0,       0, &SYS_FONT_9,  true },// 6.
    { BUTTON_RECT,     180, 280,  80, 110, 180, 100,  80,  30,   5, WHITE,       0,   0,  "TZ/DST",      1, BLACK,            SelTzScreen,        0,    0,       0, &SYS_FONT_9,  true },// 7.
    { BUTTON_RECT,     180, 280, 120, 150, 180, 100, 120,  30,   5, WHITE,       0,   0,  "SAVE",        1, BLACK,            ExSaveSetup,        0,    0,       0, &SYS_FONT_9,  true },// 8.
    { BUTTON_RECT,     180, 280, 160, 190, 180, 100, 160,  30,   5, RED,         0,   0,  "RESET",       1, BLACK,            ExResetSetup,       0,    0,       0, &SYS_FONT_9,  true },// 9.
    { BUTTON_RECT_RND, 120, 200, 200, 230, 120,  80, 200,  30,   8, SYS_BTN_CLR, 0,   0,  "DONE",        1, BLACK,            SelMainScreen,      0,    0,       0, &SYS_FONT_9,  true },//10.
    { TIMEOUT,           0,   0,   0,   0,   0,   0,   0,   0,   0, BLACK,       0,   0,  NULL,          0, BLACK,            SelMainScreen,      0,    0, TIMEOUT_MS, NULL,      true } //11.
}; // End ScrSetup[].

// Display the main setup screen.
static void SelSetupScreen(int16_t)
{
    gGui.DrawScreen(&ScrSetup, SCR_REDRAW);
} // End SelSetupScreen().



/*******************************************************************************
********************************************************************************
* Setup Clock Options Screen 1
*
* This is the first screen of clock options.  It allows for enabling/disabling
* the following:
*   - 12/24 Hour format.
*   - Display AM/PM.
*   - Display running seconds.
*   - Display the date.
********************************************************************************
*******************************************************************************/
static void SelClkOpts2(int16_t p);         // Display 2nd screen of clock options.
static void SetupClkOpts1(int16_t p);       // Display 1st screen of clock options.
static void SelClkOpts1Item(int16_t index); // Handle clock options selection.

// Indices into ScrClkOpts1[].
static const size_t INDEX_FMT_12       = 2;
static const size_t INDEX_SHOW_AM_PM   = 3;
static const size_t INDEX_SHOW_SECONDS = 4;
static const size_t INDEX_SHOW_DATE    = 5;

static scr_vec_t ScrClkOpts1 =
{ //      obj          tX0  tX1  tY0  tY1  oX0   oW  oY0   oH   oE  oC         lX0  lY0  txt            fs       lc           onClick          oLoPct  oHiPct   val font        enabled touchIndex
    { BACKGROUND,        0,   0,   0,   0,   0,   0,   0,   0,   0, BLACK,       0,   0,  NULL,          0,      BLACK,       SetupClkOpts1,      0,     0,      0, NULL,         true },// 0.
    { LABEL,             0,   0,   0,   0,   0, 320,   0,  40,   0, BLACK,       0,   0,  "CLOCK  SETUP", 1,     GREEN,       NULL,               0,     0,      0, &SYS_FONT_18, true },// 1.
    { CHECKBOX,         60, 260,  45,  65,  60,  20,  45,  20,   5, WHITE,       0,   0,  "12 HR FMT",   2,      WHITE,       SelClkOpts1Item,    0,     0,      0, NULL,         true },// 2.
    { CHECKBOX,         60, 260,  85, 105,  60,  20,  85,  20,   5, WHITE,       0,   0,  "SHOW AM/PM",  2,      WHITE,       SelClkOpts1Item,    0,     0,      0, NULL,         true },// 3.
    { CHECKBOX,         60, 260, 125, 145,  60,  20, 125,  20,   5, WHITE,       0,   0,  "SHOW SECONDS",2,      WHITE,       SelClkOpts1Item,    0,     0,      0, NULL,         true },// 4.
    { CHECKBOX,         60, 260, 165, 185,  60,  20, 165,  20,   5, WHITE,       0,   0,  "SHOW DATE",   2,      WHITE,       SelClkOpts1Item,    0,     0,      0, NULL,         true },// 5.
    { BUTTON_RECT_RND,  20, 100, 200, 230,  20,  80, 200,  30,   8, SYS_BTN_CLR, 0,   0,  "PREV",        1,      BLACK,       SelSetupScreen,     0,     0,      0, &SYS_FONT_9,  true },// 6.
    { BUTTON_RECT_RND, 120, 200, 200, 230, 120,  80, 200,  30,   8, SYS_BTN_CLR, 0,   0,  "HOME",        1,      BLACK,       SelMainScreen,      0,     0,      0, &SYS_FONT_9,  true },// 7.
    { BUTTON_RECT_RND, 220, 300, 200, 230, 220,  80, 200,  30,   8, SYS_BTN_CLR, 0,   0,  "NEXT",        1,      BLACK,       SelClkOpts2,        0,     0,      0, &SYS_FONT_9,  true },// 8.
    { TIMEOUT,           0,   0,   0,   0,   0,   0,   0,   0,   0, BLACK,       0,   0,  NULL,          0,      BLACK,       SelMainScreen,      0,     0, TIMEOUT_MS, NULL,     true } // 9.
}; // End ScrClkOpts1[].

// Display the CLOCK setup screen.
static void SelClkOpts1(int16_t)
{
    gGui.DrawScreen(&ScrClkOpts1, SCR_REDRAW);
} // End SelClkOpts1()

// Update the checkboxes of the clock options 1 screen.
static void UpdateClkOpts1()
{
    // Only enable AM/PM selection if 12 hour display is selected.
    ScrClkOpts1[INDEX_FMT_12].m_Val = gTd.IsShowing12Hour();
    SCR_DATA *pScr = &ScrClkOpts1[INDEX_SHOW_AM_PM];
    if (gTd.IsShowing12Hour())
    {
        // 12 hour display is selected, enable AM/PM selection.
        ScrClkOpts1[INDEX_SHOW_AM_PM].m_Val = gTd.IsShowingAmPm();
        pScr->m_OE = 0;
        pScr->m_OC = WHITE;
        pScr->m_LC = WHITE;
    }
    else
    {
        // 24 hour display is selected, gray out AM/PM selection.
        pScr->m_OE = 1;
        pScr->m_Val = true;
        pScr->m_OC = GRAY;
        pScr->m_LC = GRAY;
    }
    // Setup the remainder of the check boxes on this screen.
    ScrClkOpts1[INDEX_SHOW_SECONDS].m_Val = gTd.IsShowingSeconds();
    ScrClkOpts1[INDEX_SHOW_DATE].m_Val = gTd.IsShowingDate();
} // End UpdateClkOpts1().

// Display 1st screen of clock options.
static void SetupClkOpts1(int16_t)
{
    gCanvas.fillScreen(ScrClkOpts1[INDEX_BGND].m_OC);
    UpdateClkOpts1();
} // End SetupClkOpts1

// Handle clock options 1 selection.
static void SelClkOpts1Item(int16_t index)
{
    // Toggle the selected checkbox.
    switch (index)
    {
        case INDEX_FMT_12:
            gTd.Show12Hour(gTd.IsShowing12Hour() ^ true);
            ScrClkOpts1[index].m_Val = gTd.IsShowing12Hour();
            break;
        case INDEX_SHOW_AM_PM:
            // Ignore selection if 24 hour display is selected.
            if (gTd.IsShowing12Hour())
            {
                gTd.ShowAmPm(gTd.IsShowingAmPm() ^ true);
                ScrClkOpts1[index].m_Val = gTd.IsShowingAmPm();
            }
            break;
        case INDEX_SHOW_SECONDS:
            gTd.ShowSeconds(gTd.IsShowingSeconds() ^ true);
            ScrClkOpts1[index].m_Val = gTd.IsShowingSeconds();
            break;
        case INDEX_SHOW_DATE:
            gTd.ShowDate(gTd.IsShowingDate() ^ true);
            ScrClkOpts1[index].m_Val = gTd.IsShowingDate();
            break;
        default:
            break;
    }
    // Update the checkboxes on the screen.
    UpdateClkOpts1();
    gGui.DrawScreen(&ScrClkOpts1, SCR_REDRAW);
} // End SelClkOpts1Item().



/*******************************************************************************
********************************************************************************
* Setup Clock Options Screen 2
*
* This is the second screen of clock options.  It allows for enabling/disabling
* the following:
*   - Display the day of week.
*   - Display timezone.
*   - Display temperature.
*   - Display the temperature in degrees C or F.
********************************************************************************
*******************************************************************************/
static void SetupClkOpts2(int16_t p);       // Display 2nd screen of clock options.
static void SelClkOpts3(int16_t p);         // Display 3rd screen of clock options.
static void SelClkOpts2Item(int16_t index); // Handle clock options selection.

// Indices into ScrClkOpts2[].
static const size_t INDEX_SHOW_WKDAY = 2;
static const size_t INDEX_SHOW_TZ    = 3;
static const size_t INDEX_SHOW_TEMP  = 4;
static const size_t INDEX_SHOW_DEG_F = 5;

static scr_vec_t ScrClkOpts2 =
{ //      obj          tX0  tX1  tY0  tY1  oX0   oW  oY0   oH   oE  oC         lX0  lY0  txt            fs       lc           onClick          oLoPct  oHiPct   val font        enabled touchIndex
    { BACKGROUND,        0,   0,   0,   0,   0,   0,   0,   0,   0, BLACK,       0,   0,  NULL,          0,      BLACK,       SetupClkOpts2,      0,     0,      0, NULL,         true },// 0.
    { LABEL,             0,   0,   0,   0,   0, 320,   0,  40,   0, BLACK,       0,   0,  "CLOCK  SETUP",1,      SYS_TXT_CLR, NULL,               0,     0,      0, &SYS_FONT_18, true },// 1.
    { CHECKBOX,         60, 260,  45,  65,  60,  20,  45,  20,   5, WHITE,       0,   0,  "SHOW WKDAY",  2,      WHITE,       SelClkOpts2Item,    0,     0,      0, NULL,         true },// 2.
    { CHECKBOX,         60, 260,  85, 105,  60,  20,  85,  20,   5, WHITE,       0,   0,  "SHOW TIME ZONE",2,    WHITE,       SelClkOpts2Item,    0,     0,      0, NULL,         true },// 3.
    { CHECKBOX,         60, 260, 125, 145,  60,  20, 125,  20,   5, WHITE,       0,   0,  "SHOW TEMP",   2,      WHITE,       SelClkOpts2Item,    0,     0,      0, NULL,         true },// 4.
    { CHECKBOX,         60, 260, 165, 185,  60,  20, 165,  20,   5, WHITE,       0,   0,  "SHOW DEG F",  2,      WHITE,       SelClkOpts2Item,    0,     0,      0, NULL,         true },// 5.
    { BUTTON_RECT_RND,  20, 100, 200, 230,  20,  80, 200,  30,   8, SYS_BTN_CLR, 0,   0,  "PREV",        1,      BLACK,       SelClkOpts1,        0,     0,      0, &SYS_FONT_9,  true },// 6.
    { BUTTON_RECT_RND, 120, 200, 200, 230, 120,  80, 200,  30,   8, SYS_BTN_CLR, 0,   0,  "HOME",        1,      BLACK,       SelMainScreen,      0,     0,      0, &SYS_FONT_9,  true },// 7.
    { BUTTON_RECT_RND, 220, 300, 200, 230, 220,  80, 200,  30,   8, SYS_BTN_CLR, 0,   0,  "NEXT",        1,      BLACK,       SelClkOpts3,        0,     0,      0, &SYS_FONT_9,  true },// 8.
    { TIMEOUT,           0,   0,   0,   0,   0,   0,   0,   0,   0, BLACK,       0,   0,  NULL,          0,      BLACK,       SelMainScreen,      0,     0, TIMEOUT_MS, NULL,     true } // 8.
}; // End ScrClkOpts2[].

// Display 2nd screen of clock options.
static void SelClkOpts2(int16_t)
{
    gGui.DrawScreen(&ScrClkOpts2, SCR_REDRAW);
} // End SelClkOpts2().

// Update the check boxes of the clock options 2 screen.
static void UpdateClkOpts2()
{
    // Setup the values for the checkboxes on this screen.
    ScrClkOpts2[INDEX_SHOW_WKDAY].m_Val = gTd.IsShowingWkDay();
    ScrClkOpts2[INDEX_SHOW_TZ].m_Val = gTd.IsShowingTz();
    ScrClkOpts2[INDEX_SHOW_TEMP].m_Val = gTd.IsShowingTemp();

    // Only enable degree type if temperature display is selected.
    SCR_DATA *pScr = &ScrClkOpts2[INDEX_SHOW_DEG_F];
    if (gTd.IsShowingTemp())
    {
        // Temperature display is enabled, enable C/F selection.
        pScr->m_Val = gTd.IsShowingDegreesF();
        pScr->m_OE = 0;
        pScr->m_OC = WHITE;
        pScr->m_LC = WHITE;
    }
    else
    {
        // Temperature display is disabled, gray out C/F selection.
        pScr->m_OE = 1;
        pScr->m_Val = true;
        pScr->m_OC = GRAY;
        pScr->m_LC = GRAY;
    }
} // End UpdateClkOpts2()

// Display 2nd screen of clock options.
static void SetupClkOpts2(int16_t)
{
    gCanvas.fillScreen(ScrClkOpts2[INDEX_BGND].m_OC);
    UpdateClkOpts2();
} // End SetupClkOpts2().

// Handle clock options selection.
static void SelClkOpts2Item(int16_t index)
{
    switch (index)
    {
        // Toggle the selected checkbox.
        case INDEX_SHOW_WKDAY:
            gTd.ShowWkDay(gTd.IsShowingWkDay() ^ true);
            ScrClkOpts2[index].m_Val = gTd.IsShowingWkDay();
            break;
        case INDEX_SHOW_TZ:
            gTd.ShowTz(gTd.IsShowingTz() ^ true);
            ScrClkOpts2[index].m_Val = gTd.IsShowingTz();
            break;
        case INDEX_SHOW_TEMP:
            gTd.ShowTemp(gTd.IsShowingTemp() ^ true);
            ScrClkOpts2[index].m_Val = gTd.IsShowingTemp();
            break;
        case INDEX_SHOW_DEG_F:
            // Ignore selection if temperature is not being displayed.
            if (gTd.IsShowingTemp())
            {
                gTd.ShowDegreesF(gTd.IsShowingDegreesF() ^ true);
                ScrClkOpts2[index].m_Val = gTd.IsShowingDegreesF();
            }
            break;
        default:
            break;
    }
    // Update the checkboxes on the screen.
    UpdateClkOpts2();
    gGui.DrawScreen(&ScrClkOpts2, SCR_REDRAW);
} // End SelClkOpts2Item().



/*******************************************************************************
********************************************************************************
* Setup Clock Options Screen 3 (Brightness)
*
* This is the third screen of clock options.  It allows for setting the screen
* brightness.
********************************************************************************
*******************************************************************************/
static void SelBrightnessChange(int16_t);   //Handle screen brightness changes.
static void SelAutoBriteChange(int16_t);    // Handle auto brightness on/off.

// Indices into ScrClkOpts3[].
static const size_t INDEX_AUTO_BRIGHTNESS   = 2;
static const size_t INDEX_SLIDER_LABEL      = 3;
static const size_t INDEX_BRIGHTNESS_SLIDER = 4;

static scr_vec_t ScrClkOpts3 =
{ //      obj          tX0  tX1  tY0  tY1  oX0   oW  oY0   oH   oE  oC         lX0  lY0  txt            fs       lc           onClick          oLoPct  oHiPct   val font        enabled touchIndex
    { BACKGROUND,        0,   0,   0,   0,   0,   0,   0,   0,   0, BLACK,       0,   0,  NULL,          0,      BLACK,       NULL,               0,     0,      0, NULL,         true },// 0.
    { LABEL,             0,   0,   0,   0,   0, 320,   0,  40,   0, BLACK,       0,   0,  "BRIGHTNESS",  1,      GREEN,       NULL,               0,     0,      0, &SYS_FONT_18, true },// 1.
    { CHECKBOX,         60, 260,  50,  70,  60,  20,  50,  20,   5, WHITE,       0,   0,  "  AUTO BRIGHTNESS", 2,  WHITE,     SelAutoBriteChange, 0,     0,      0, NULL,         true },// 2.
    { LABEL,             0,   0,   0,   0,   0, 320,  90,  20,   0, BLACK,       0,   0,  NULL,          1,      GREEN,       NULL,               0,     0,      0, &SYS_FONT_9,  true },// 3.
    { SLIDERBAR_H,      50, 270, 125, 145,  50, 220, 125,  20,   4, WHITE,       0,   0,  NULL,          1,      WHITE,       SelBrightnessChange, -100, 100,    0, NULL,         true },// 4.
    { BUTTON_RECT_RND,  20, 100, 200, 230,  20,  80, 200,  30,   8, SYS_BTN_CLR, 0,   0,  "PREV",        1,      BLACK,       SelClkOpts2,        0,     0,      0, &SYS_FONT_9,  true },// 5.
    { BUTTON_RECT_RND, 120, 200, 200, 230, 120,  80, 200,  30,   8, SYS_BTN_CLR, 0,   0,  "HOME",        1,      BLACK,       SelMainScreen,      0,     0,      0, &SYS_FONT_9,  true },// 6.
    { TIMEOUT,           0,   0,   0,   0,   0,   0,   0,   0,   0, BLACK,       0,   0,  NULL,          0,      BLACK,       SelMainScreen,      0,     0, TIMEOUT_MS, NULL,     true } // 7.
}; // End ScrClkOpts3[].

// Set display values based on current settings.
static void SetupBrightnessDisplay()
{
    // If the LDRis present, then we enable the selection checkbox and set up
    // limits based on whether or not the LDR is selected to adjust the brightness.
    ScrClkOpts3[INDEX_AUTO_BRIGHTNESS].m_Enable = gBacklight.IsLdrPresent();

    if (gBacklight.IsLdrUsed())
    {
        // LDR is seleected to adjust the brightness.
        ScrClkOpts3[INDEX_SLIDER_LABEL].m_pTxt = "OFFSET";
        ScrClkOpts3[INDEX_BRIGHTNESS_SLIDER].m_LoPct = -100;
        ScrClkOpts3[INDEX_BRIGHTNESS_SLIDER].m_Val =
            (gBacklight.GetBrightness() * 200) / gBacklight.GetRange();
    }
    else
    {
        // LDR is present but not selected to adjust the brightness.
        ScrClkOpts3[INDEX_SLIDER_LABEL].m_pTxt = "BRIGHTNESS";
        ScrClkOpts3[INDEX_BRIGHTNESS_SLIDER].m_LoPct = 0;
        ScrClkOpts3[INDEX_BRIGHTNESS_SLIDER].m_Val =
            (gBacklight.GetBrightness() * 100) / gBacklight.GetRange();
    }
} // End SetupBrightnessDisplay().


// Handle auto brightness on/off changes.
static void SelAutoBriteChange(int16_t)
{
    ScrClkOpts3[INDEX_AUTO_BRIGHTNESS].m_Val ^= true;
    gBacklight.UseLdr(ScrClkOpts3[INDEX_AUTO_BRIGHTNESS].m_Val);
    SetupBrightnessDisplay();
    gGui.DrawScreen(&ScrClkOpts3, SCR_REDRAW);
} // End SelAutoBriteChange().

// Initialize the brightness setting screen.  Called when this screen is
// initially selected.
static void SelClkOpts3(int16_t)
{
    gCanvas.fillScreen(ScrClkOpts3[INDEX_BGND].m_OC);
    ScrClkOpts3[INDEX_AUTO_BRIGHTNESS].m_Val = gBacklight.IsLdrUsed();
    SetupBrightnessDisplay();
    gGui.DrawScreen(&ScrClkOpts3, SCR_REDRAW);
} // End SelClkOpts3().

// Handle screen brightness changes.
static void SelBrightnessChange(int16_t)
{
    // Calculate the brightness value and use it.
    gGui.DrawScreen(&ScrClkOpts3, SCR_REDRAW);
    int conv = ScrClkOpts3[INDEX_BRIGHTNESS_SLIDER].m_Val;
    conv *= gBacklight.GetRange();
    conv /= (gBacklight.IsLdrUsed() ? 200 : 100);
    gBacklight.SetBrightness(conv);
} // End SelBrightnessChange().



/*******************************************************************************
********************************************************************************
* Setup Main Fonts Screen
*
* This is the first of the 3 font setup screens.  This screen allows for selection
* of which of the main fonts may be displayed.  Fonts are displayed individually,
* with a corresonding checkbox.  If the checkbox is enabled, then the font is
* able to be displayed.  The font name and a sample of the font are also diaplayed.
********************************************************************************
*******************************************************************************/
static void SelNextMainFont(int16_t);       // Select the next main font.
static void SelPrevMainFont(int16_t);       // Select the previous main font.
static void ToggleMainFontActive(int16_t);  // Toggles selection of the current main font.
static void SelAuxFontScreen(int16_t);      // Select the 2nd font setup screen.

// Indices into ScrMainFont[].
static const size_t INDEX_CLOCK_DISP            = 3;
static const size_t INDEX_MAIN_FONT_PREV_BUTTON = 4;
static const size_t INDEX_MAIN_FONT_NEXT_BUTTON = 5;
static const size_t INDEX_MAIN_FONT_CKBOX       = 6;

static scr_vec_t ScrMainFont =
{ //      obj          tX0  tX1  tY0  tY1  oX0   oW  oY0   oH   oE  oC         lX0  lY0  txt            fs       lc           onClick          oLoPct  oHiPct   val font        enabled touchIndex
    { BACKGROUND,        0,   0,   0,   0,   0,   0,   0,   0,   0, BLACK,       0,   0,  NULL,          0,       BLACK,       NULL,              0,     0,      0, NULL,         true },// 0.
    { LABEL,             0,   0,   0,   0,   0, 320,   0,  40,   0, BLACK,       0,   0,  "MAIN  FONT",  1,       SYS_TXT_CLR, NULL,              0,     0,      0, &SYS_FONT_18, true },// 1.
    { LABEL,             0,   0,   0,   0,   0, 320,  40,  20,   0, BLACK,       0,   0,  "PICK AT LEAST 1", 1,   SYS_TXT_CLR, NULL,              0,     0,      0, NULL,         true },// 2.
    { LABEL,             0, 319, 110, 190,   0, 320,  80, 120,   0, BLACK,       0,   0,  "12:34",       1,       WHITE,       ToggleMainFontActive,0,   0,      0, NULL,         true },// 3.
    { BUTTON_ROUND,      5,  35,  55,  85,   5,  30,  55,   0,   0, SYS_BTN_CLR, 0,   0,  "<",           1,       BLACK,       SelPrevMainFont,    0,    0,      0, &SYS_FONT_9,  true },// 4.
    { BUTTON_ROUND,    285, 315,  55,  85, 285,  30,  55,   0,   0, SYS_BTN_CLR, 0,   0,  ">",           1,       BLACK,       SelNextMainFont,    0,    0,      0, &SYS_FONT_9,  true },// 5.
    { CHECKBOX,         50, 270,  60,  80,  50,  20,  60,  20,   5, WHITE,       0,   0,  NULL,          2,       WHITE,       ToggleMainFontActive,0,   0,      0, NULL,         true },// 6.
    { BUTTON_RECT_RND,  20, 100, 200, 230,  20,  80, 200,  30,   8, SYS_BTN_CLR, 0,   0,  "PREV",        1,       BLACK,       SelSetupScreen,     0,    0,      0, &SYS_FONT_9,  true },// 7.
    { BUTTON_RECT_RND, 120, 200, 200, 230, 120,  80, 200,  30,   8, SYS_BTN_CLR, 0,   0,  "HOME",        1,       BLACK,       SelMainScreen,      0,    0,      0, &SYS_FONT_9,  true },// 8.
    { BUTTON_RECT_RND, 220, 300, 200, 230, 220,  80, 200,  30,   8, SYS_BTN_CLR, 0,   0,  "NEXT",        1,       BLACK,       SelAuxFontScreen,   0,    0,      0, &SYS_FONT_9,  true },// 9.
    { TIMEOUT,           0,   0,   0,   0,   0,   0,   0,   0,   0, BLACK,       0,   0,  NULL,          0,       BLACK,       SelMainScreen,      0,    0, TIMEOUT_MS, NULL,     true } //10.
}; // End ScrMainFont[].

// Update the main font screen.
static void UpdateMainFont()
{
    // Display the screen.
    ScrMainFont[INDEX_CLOCK_DISP].m_pFont = TimeMainFonts.Font();
    ScrMainFont[INDEX_MAIN_FONT_CKBOX].m_Val = TimeMainFonts.IsActive();
    ScrMainFont[INDEX_MAIN_FONT_CKBOX].m_pTxt = TimeMainFonts.Name();

    // First and last fonts are marked by the corresponding arrow icon being
    // darkened.  This does not disable the next/prev use.  Font selection
    // will always wrap around to the last/first font if a darkened icon is
    // selected.
    if (TimeMainFonts.IsLast())
    {
        // Last font, darken the NEXT button.
        ScrMainFont[INDEX_MAIN_FONT_PREV_BUTTON].m_OC = SYS_BTN_CLR;
        ScrMainFont[INDEX_MAIN_FONT_NEXT_BUTTON].m_OC = SYS_DRK_CLR;
    }
    else if (TimeMainFonts.IsFirst())
    {
        // First font, darken the PREV button.
        ScrMainFont[INDEX_MAIN_FONT_PREV_BUTTON].m_OC = SYS_DRK_CLR;
        ScrMainFont[INDEX_MAIN_FONT_NEXT_BUTTON].m_OC = SYS_BTN_CLR;
    }
    else
    {
        // Not first or last font.  Don't darken any.
        ScrMainFont[INDEX_MAIN_FONT_PREV_BUTTON].m_OC = SYS_BTN_CLR;
        ScrMainFont[INDEX_MAIN_FONT_NEXT_BUTTON].m_OC = SYS_BTN_CLR;
    }
    // Draw the screen.
    gGui.DrawScreen(&ScrMainFont, SCR_REDRAW);
} // End UpdateMainFont().

// Display the first FONT setup screen.
static void SelMainFontScreen(int16_t)
{
    TimeMainFonts.Begin();
    UpdateMainFont();
} // End SelMainFontScreen().

// Select the next main font.
static void SelNextMainFont(int16_t)
{
    TimeMainFonts.Next();
    UpdateMainFont();
} // End SelNextMainFont().

// Select the previous main font.
static void SelPrevMainFont(int16_t)
{
    TimeMainFonts.Prev();
    UpdateMainFont();
} // End SelPrevMainFont().

// Toggle the current font's active state.
static void ToggleMainFontActive(int16_t)
{
    TimeMainFonts.SetActive(TimeMainFonts.IsActive() ^ 1);
    ScrMainFont[INDEX_MAIN_FONT_CKBOX].m_Val = TimeMainFonts.IsActive();
    UpdateMainFont();
} // End ToggleMainFontActive().



/*******************************************************************************
********************************************************************************
* Setup Aux Font Screen
*
* This is the second of the 3 font setup screens.  This screen allows for selection
* of the aux font.  This is the font that is used to display the day of week,
* date, temperature, and running seconds on the main time screen.  The font name
* and a sample of the font are also displayed.
********************************************************************************
*******************************************************************************/
static void SelNextAuxFont(int16_t);        // Select the next aux font.
static void SelPrevAuxFont(int16_t);        // Select the previous aux font.
static void ActivateFont(int16_t);          // Makes the selected font active.
static void SelFontCyclingScreen(int16_t);  // Select the 3rd font setup screen.

// Indices into ScrAuxFont[].
static const size_t INDEX_AUX_FONT_WDKAY       = 3;
static const size_t INDEX_AUX_FONT_DATE        = 4;
static const size_t INDEX_AUX_FONT_PREV_BUTTON = 5;
static const size_t INDEX_AUX_FONT_NEXT_BUTTON = 6;
static const size_t INDEX_AUX_FONT_CKBOX       = 7;

static scr_vec_t ScrAuxFont =
{ //      obj          tX0  tX1  tY0  tY1  oX0   oW  oY0   oH   oE  oC         lX0  lY0  txt            fs       lc           onClick          oLoPct  oHiPct   val font        enabled touchIndex
    { BACKGROUND,        0,   0,   0,   0,   0,   0,   0,   0,   0, BLACK,       0,   0, NULL,           0,      BLACK,       NULL,               0,     0,      0, NULL,         true },// 0.
    { LABEL,             0,   0,   0,   0,   0, 320,   0,  40,   0, BLACK,       0,   0, "AUX  FONT",    1,      SYS_TXT_CLR, NULL,               0,     0,      0, &SYS_FONT_18, true },// 1.
    { LABEL,             0,   0,   0,   0,   0, 320,  40,  20,   0, BLACK,       0,   0, "PICK ONLY 1",  1,      SYS_TXT_CLR, NULL,               0,     0,      0, NULL,         true },// 2.
    { LABEL,            60, 260, 100, 140,  60, 200, 100,  40,   0, BLACK,       0,   0, "Monday",       1,      WHITE,       ActivateFont,       0,     0,      0, NULL,         true },// 3.
    { LABEL,             0, 319, 140, 180,   0, 320, 140,  40,   0, BLACK,       0,   0, "November 16, 1953", 1, WHITE,       ActivateFont,       0,     0,      0, NULL,         true },// 4.
    { BUTTON_ROUND,      5,  35,  55,  85,   5,  30,  55,   0,   0, SYS_BTN_CLR, 0,   0, "<",            1,      BLACK,       SelPrevAuxFont,     0,     0,      0, &SYS_FONT_9,  true },// 5.
    { BUTTON_ROUND,    285, 315,  55,  85, 285,  30,  55,   0,   0, SYS_BTN_CLR, 0,   0, ">",            1,      BLACK,       SelNextAuxFont,     0,     0,      0, &SYS_FONT_9,  true },// 6.
    { CHECKBOX,         50, 270,  60,  80,  50,  20,  60,  20,   5, WHITE,       0,   0, NULL,           2,      WHITE,       ActivateFont,       0,     0,      0, NULL,         true },// 7.
    { BUTTON_RECT_RND,  20, 100, 200, 230,  20,  80, 200,  30,   8, SYS_BTN_CLR, 0,   0, "PREV",         1,      BLACK,       SelMainFontScreen,  0,     0,      0, &SYS_FONT_9,  true },// 8.
    { BUTTON_RECT_RND, 120, 200, 200, 230, 120,  80, 200,  30,   8, SYS_BTN_CLR, 0,   0, "HOME",         1,      BLACK,       SelMainScreen,      0,     0,      0, &SYS_FONT_9,  true },// 9.
    { BUTTON_RECT_RND, 220, 300, 200, 230, 220,  80, 200,  30,   8, SYS_BTN_CLR, 0,   0, "NEXT",         1,      BLACK,       SelFontCyclingScreen,0,    0,      0, &SYS_FONT_9,  true },//10.
    { TIMEOUT,           0,   0,   0,   0,   0,   0,   0,   0,   0, BLACK,       0,   0,  NULL,          0,      BLACK,       SelMainScreen,      0,     0, TIMEOUT_MS, NULL,     true } //11.
}; // End ScrAuxFont[].

// Forces only one aux (and minor) font to be active.
static void MakeOneFontActive()
{
    TimeMinorFonts.Begin();
    TimeSecondaryFonts.Begin();
    bool gotOne = false;
    // Loop through the aux fonts.
    for ( ; !TimeSecondaryFonts.IsEnd(); ++TimeSecondaryFonts, ++TimeMinorFonts)
    {
        // The first active font found will be the only one selected.
        // All others will be made inactive.
        if (!gotOne && TimeSecondaryFonts.IsActive())
        {
            gotOne = true;
            TimeMinorFonts.SetActive(true);
        }
        else
        {
            TimeSecondaryFonts.SetActive(false);
            TimeMinorFonts.SetActive(false);
        }
    }
} // End MakeOneFontActive().

// Makes the selected font active.
static void ActivateFont(int16_t)
{
    // Remember which font should be active.
    uint32_t index = TimeMinorFonts.Index();
    ScrAuxFont[INDEX_AUX_FONT_CKBOX].m_Val = true;
    TimeSecondaryFonts.Begin();
    TimeMinorFonts.Begin();

    // Mark all fonts inactive.
    for ( ; !TimeSecondaryFonts.IsEnd(); ++TimeSecondaryFonts, ++TimeMinorFonts)
    {
        TimeSecondaryFonts.SetActive(false);
        TimeMinorFonts.SetActive(false);
    }

    // Set the font that we remembered to be active.
    TimeSecondaryFonts[index].SetActive(true);
    TimeMinorFonts[index].SetActive(true);

    // Display the screen.
    gGui.DrawScreen(&ScrAuxFont, SCR_REDRAW);
} // End ActivateFont().

// Updates the aux font setup screen.
static void UpdateAuxFont()
{
    // Update the sample text and checkbox.
    ScrAuxFont[INDEX_AUX_FONT_WDKAY].m_pFont = TimeSecondaryFonts.Font();
    ScrAuxFont[INDEX_AUX_FONT_DATE].m_pFont = TimeSecondaryFonts.Font();
    ScrAuxFont[INDEX_AUX_FONT_CKBOX].m_pTxt = TimeSecondaryFonts.Name();
    ScrAuxFont[INDEX_AUX_FONT_CKBOX].m_Val = TimeSecondaryFonts.IsActive();

    // First and last fonts are marked by the corresponding arrow icon being
    // darkened.  This does not disable the next/prev use.  Font selection
    // will always wrap around to the last/first font if a darkened icon is
    // selected.
    if (TimeSecondaryFonts.IsLast())
    {
        // Last font, darken the NEXT button.
        ScrAuxFont[INDEX_AUX_FONT_PREV_BUTTON].m_OC = SYS_BTN_CLR;
        ScrAuxFont[INDEX_AUX_FONT_NEXT_BUTTON].m_OC = SYS_DRK_CLR;
    }
    else if (TimeSecondaryFonts.IsFirst())
    {
        // First font, darken the PREV button.
        ScrAuxFont[INDEX_AUX_FONT_PREV_BUTTON].m_OC = SYS_DRK_CLR;
        ScrAuxFont[INDEX_AUX_FONT_NEXT_BUTTON].m_OC = SYS_BTN_CLR;
    }
    else
    {
        // Not first or last font.  Don't darken any.
        ScrAuxFont[INDEX_AUX_FONT_PREV_BUTTON].m_OC = SYS_BTN_CLR;
        ScrAuxFont[INDEX_AUX_FONT_NEXT_BUTTON].m_OC = SYS_BTN_CLR;
    }

    // Draw the screen.
    gGui.DrawScreen(&ScrAuxFont, SCR_REDRAW);
} // End UpdateAuxFont().

// Select the 2nd font setup screen.
static void SelAuxFontScreen(int16_t)
{
    MakeOneFontActive();
    TimeSecondaryFonts.FirstActive();
    TimeMinorFonts.FirstActive();
    UpdateAuxFont();
} // End SelAuxFontScreen().

// Select the next aux font.
static void SelNextAuxFont(int16_t)
{
    TimeSecondaryFonts.Next();
    TimeMinorFonts.Next();
    UpdateAuxFont();
} // End  SelNextAuxFont().

// Select the previous aux font.
static void SelPrevAuxFont(int16_t)
{
    TimeSecondaryFonts.Prev();
    TimeMinorFonts.Prev();
    UpdateAuxFont();
} // End SelPrevAuxFont().



/*******************************************************************************
********************************************************************************
* Setup Font Cycling Screen
*
* This is the third of the 3 font setup screens.  This screen allows for selection
* of rate at which the main font will be cycled.  Options are:
*   - Never
*   - Every second
*   - Every minute - at the start of each minute
*   - Every hour   - at the start of each hour
*   - Every day    - at the start of each day (midnight)
*   - Every week   - at the start of each week (Sunday 12 AM)
*   - Every month  - at the start of each month
*   - Every year   - at the start of each year
********************************************************************************
*******************************************************************************/
static void SelFontCyclingItem(int16_t index); // Handle selection of cycling rate.

// Indices into ScrMainFont[].
static const size_t INDEX_NEVER  = 2;
static const size_t INDEX_SECOND = 3;
static const size_t INDEX_MINUTE = 4;
static const size_t INDEX_HOUR   = 5;
static const size_t INDEX_DAY    = 6;
static const size_t INDEX_WEEK   = 7;
static const size_t INDEX_MONTH  = 8;
static const size_t INDEX_YEAR   = 9;

static scr_vec_t ScrSelFontCycling =
{ //      obj          tX0  tX1  tY0  tY1  oX0   oW  oY0   oH   oE  oC         lX0  lY0  txt            fs       lc           onClick          oLoPct  oHiPct   val font        enabled touchIndex
    { BACKGROUND,        0,   0,   0,   0,   0,   0,   0,   0,   0, BLACK,       0,   0,  NULL,          0,      BLACK,       NULL,               0,     0,       0, NULL,         true },// 0.
    { LABEL,             0,   0,   0,   0,   0, 320,   0,  40,   0, BLACK,       0,   0,  "FONT CYCLING",1,      SYS_TXT_CLR, NULL,               0,     0,       0, &SYS_FONT_18, true },// 1.
    { RADIO_BUTTON,     25, 145,  40,  70,  25,  30,  40,  30,   5, WHITE,       0,   0,  "NEVER",       2,      WHITE,       SelFontCyclingItem, 0,     0,       0, NULL,         true },// 2.
    { RADIO_BUTTON,    190, 310,  40,  70, 190,  30,  40,  30,   5, WHITE,       0,   0,  "SECOND",      2,      WHITE,       SelFontCyclingItem, 0,     0,       0, NULL,         true },// 3.
    { RADIO_BUTTON,     25, 145,  80, 110,  25,  30,  80,  30,   5, WHITE,       0,   0,  "MINUTE",      2,      WHITE,       SelFontCyclingItem, 0,     0,       0, NULL,         true },// 4.
    { RADIO_BUTTON,    190, 310,  80, 110, 190,  30,  80,  30,   5, WHITE,       0,   0,  "HOUR",        2,      WHITE,       SelFontCyclingItem, 0,     0,       0, NULL,         true },// 5.
    { RADIO_BUTTON,     25, 145, 120, 150,  25,  30, 120,  30,   5, WHITE,       0,   0,  "DAY",         2,      WHITE,       SelFontCyclingItem, 0,     0,       0, NULL,         true },// 6.
    { RADIO_BUTTON,    190, 310, 120, 150, 190,  30, 120,  30,   5, WHITE,       0,   0,  "WEEK",        2,      WHITE,       SelFontCyclingItem, 0,     0,       0, NULL,         true },// 7.
    { RADIO_BUTTON,     25, 145, 160, 190,  25,  30, 160,  30,   5, WHITE,       0,   0,  "MONTH",       2,      WHITE,       SelFontCyclingItem, 0,     0,       0, NULL,         true },// 8.
    { RADIO_BUTTON,    190, 310, 160, 190, 190,  30, 160,  30,   5, WHITE,       0,   0,  "YEAR",        2,      WHITE,       SelFontCyclingItem, 0,     0,       0, NULL,         true },// 9.
    { BUTTON_RECT_RND,  20, 100, 200, 230,  20,  80, 200,  30,   8, SYS_BTN_CLR, 0,   0,  "PREV",        1,      BLACK,       SelAuxFontScreen,   0,     0,       0, &SYS_FONT_9,  true },//10.
    { BUTTON_RECT_RND, 120, 200, 200, 230, 120,  80, 200,  30,   8, SYS_BTN_CLR, 0,   0,  "HOME",        1,      BLACK,       SelMainScreen,      0,     0,       0, &SYS_FONT_9,  true },//11.
    { TIMEOUT,           0,   0,   0,   0,   0,   0,   0,   0,   0, BLACK,       0,   0,  NULL,          0,      BLACK,       SelMainScreen,      0,     0, TIMEOUT_MS, NULL,      true } //12.
}; // End ScrSelFontCycling[].

// Clears all font cycling rate radio buttons.
static void ClearFontCycling()
{
    // Clear all cycling selections.
    for (size_t i = INDEX_NEVER; i <= INDEX_YEAR; i++)
    {
        ScrSelFontCycling[i].m_Val = 0;
    }
} // End ClearFontCycling().

// Select and display the 3rd font setup screen.
static void SelFontCyclingScreen(int16_t)
{
    ClearFontCycling();
    FontCycleTime v = gTd.FontCycle();
    if (v == FC_NEVER)
    {
        ScrSelFontCycling[INDEX_NEVER].m_Val = 1;
    }
    else
    {
        ScrSelFontCycling[v + INDEX_NEVER].m_Val = 1;
    }

    gGui.DrawScreen(&ScrSelFontCycling, SCR_REDRAW);
} // End SelFontCyclingScreen().

// Handle selection of cycling rate.
static void SelFontCyclingItem(int16_t index)
{
    ClearFontCycling();
    ScrSelFontCycling[index].m_Val = true;
    gTd.SetFontCycle((FontCycleTime)(index - INDEX_NEVER));
    gGui.DrawScreen(&ScrSelFontCycling, SCR_REDRAW);
} // End SelFontCyclingItem().



/*******************************************************************************
********************************************************************************
* Setup Timezone Screen
*
* This screen allow for selection of the local timezone.
********************************************************************************
*******************************************************************************/
static void SelNextTz(int16_t);     // Select the next timezone.
static void SelPrevTz(int16_t);     // Select the previous timezone.
static void SelActiveTz(int16_t);   // Activate (use) the selected timezone.
static void SelTzSort(int16_t);     // Select type of sort for traversing timezones.

// Indices into ScrTz[].
static const size_t INDEX_TZ_LOCATION = 2;
static const size_t INDEX_TZ_PREV     = 3;
static const size_t INDEX_TZ_NEXT     = 4;
static const size_t INDEX_TZ_CKBOX    = 5;
static const size_t INDEX_TZ_ALPHA    = 6;
static const size_t INDEX_TZ_NUMERIC  = 7;

static scr_vec_t ScrTz =
{ //      obj          tX0  tX1  tY0  tY1  oX0   oW  oY0   oH   oE  oC         lX0  lY0  txt            fs       lc           onClick          oLoPct  oHiPct   val font        enabled touchIndex
    { BACKGROUND,        0,   0,   0,   0,   0,   0,   0,   0,   0, BLACK,       0,   0, NULL,           0,      BLACK,       NULL,               0,     0,      0, NULL,         true },// 0.
    { LABEL,             0,   0,   0,   0,   0, 320,   0,  40,   0, BLACK,       0,   0, "TIME ZONE",    1,      SYS_TXT_CLR, NULL,               0,     0,      0, &SYS_FONT_18, true },// 1.
    { LABEL,             0, 319, 120, 180,   0, 320, 120,  60,   0, BLACK,       0,   0, NULL,           1,      WHITE,       SelActiveTz,        0,     0,      0, &SYS_FONT_18, true },// 2.
    { BUTTON_ROUND,      5,  35,  85, 115,   5,  30,  85,   0,   0, SYS_BTN_CLR, 0,   0, "<",            1,      BLACK,       SelPrevTz,          0,     0,      0, &SYS_FONT_9,  true },// 3.
    { BUTTON_ROUND,    285, 315,  85, 115, 285,  30,  85,   0,   0, SYS_BTN_CLR, 0,   0, ">",            1,      BLACK,       SelNextTz,          0,     0,      0, &SYS_FONT_9,  true },// 4.
    { CHECKBOX,         50, 270,  90, 100,  50,  20,  90,  20,   5, WHITE,       0,   0, NULL,           2,      WHITE,       SelActiveTz,        0,     0,      0, NULL,         true },// 5.
    { RADIO_BUTTON,     55, 155,  50,  70,  55,  15,  50,  15,   5, WHITE,       0,   0, "ALPHA SORT",   1,      WHITE,       SelTzSort,          0,     0,      0, NULL,         true },// 6.
    { RADIO_BUTTON,    180, 280,  50,  70, 180,  15,  50,  15,   5, WHITE,       0,   0, "OFFSET SORT",  1,      WHITE,       SelTzSort,          0,     0,      0, NULL,         true },// 7.
    { BUTTON_RECT_RND,  20, 100, 200, 230,  20,  80, 200,  30,   8, SYS_BTN_CLR, 0,   0, "PREV",         1,      BLACK,       SelSetupScreen,     0,     0,      0, &SYS_FONT_9,  true },// 8.
    { BUTTON_RECT_RND, 120, 200, 200, 230, 120,  80, 200,  30,   8, SYS_BTN_CLR, 0,   0, "HOME",         1,      BLACK,       SelMainScreen,      0,     0,      0, &SYS_FONT_9,  true },// 9.
    { TIMEOUT,           0,   0,   0,   0,   0,   0,   0,   0,   0, BLACK,       0,   0,  NULL,          0,      BLACK,       SelMainScreen,      0,     0, TIMEOUT_MS, NULL,     true } //10.
}; // End ScrTz[].

// Remember which timezone is currently displayed.
static ClockTz::tz_iter_t ScrTzIter = ClockTz::TzTable.begin();

// Updates the timezone screen based on current values.
static void UpdateTz()
{
    // Setup the sort type radio buttons.
    ScrTz[INDEX_TZ_ALPHA].m_Val = (ClockTz::CurTzSortType == ClockTz::SORT_ALPHA);
    ScrTz[INDEX_TZ_NUMERIC].m_Val = !ScrTz[INDEX_TZ_ALPHA].m_Val;

    // Setup the pertinent timezone fields.
    ScrTz[INDEX_TZ_LOCATION].m_pTxt = ScrTzIter->Location();
    float_t offset = ScrTzIter->Offset();
    int hours = (int32_t)trunc(offset);
    int seconds = abs((int32_t)((offset - hours) * 60));
    snprintf(StrBuf, STR_BUF_SIZE, "GMT %+d:%02d", hours, seconds);
    ScrTz[INDEX_TZ_CKBOX].m_pTxt = StrBuf;
    ScrTz[INDEX_TZ_CKBOX].m_Val = (ClockTz::ActiveTzId == ScrTzIter->Id());

    // First and last timezones are marked by the corresponding arrow icon being
    // darkened.  This does not disable the next/prev use.  Timezone selection
    // will always wrap around to the last/first timezone if a darkened icon is
    // selected.
    if (ScrTzIter == (ClockTz::TzTable.end() - 1))
    {
        // Last timezone, darken the NEXT button.
        ScrTz[INDEX_TZ_PREV].m_OC = SYS_BTN_CLR;
        ScrTz[INDEX_TZ_NEXT].m_OC = SYS_DRK_CLR;
    }
    else if (ScrTzIter == ClockTz::TzTable.begin())
    {
        // First timezone, darken the PREV button.
        ScrTz[INDEX_TZ_PREV].m_OC = SYS_DRK_CLR;
        ScrTz[INDEX_TZ_NEXT].m_OC = SYS_BTN_CLR;
    }
    else
    {
        // Not first or last timezone.  Don't darken any.
        ScrTz[INDEX_TZ_PREV].m_OC = SYS_BTN_CLR;
        ScrTz[INDEX_TZ_NEXT].m_OC = SYS_BTN_CLR;
    }

    // Draw the screen/
    gGui.DrawScreen(&ScrTz, SCR_REDRAW);
} // End UpdateTz().

// Display the TIMEZONE setup screen.
static void SelTzScreen(int16_t)
{
    ScrTzIter = ClockTz::FindTzById(ClockTz::ActiveTzId);
    UpdateTz();
} // End SelTzScreen().

// Activate (use) the selected timezone.
static void SelActiveTz(int16_t)
{
    ClockTz::SetTz(ScrTzIter->Id());
    ClockTz::ActiveTzId = ScrTzIter->Id();
    UpdateTz();
} // End SelActiveTz().

 // Select the next timezone.
static void SelNextTz(int16_t)
{
    if (++ScrTzIter == ClockTz::TzTable.end())
    {
        ScrTzIter = ClockTz::TzTable.begin();
    }
    UpdateTz();
} // End SelNextTz().

// Select the previous timezone.
static void SelPrevTz(int16_t)
{
    if (ScrTzIter == ClockTz::TzTable.begin())
    {
        ScrTzIter = ClockTz::TzTable.end() - 1;
    }
    else
    {
        --ScrTzIter;
    }
    UpdateTz();
} // End SelPrevTz().

// Select type of sort for traversing timezones.
static void SelTzSort(int16_t index)
{
    if (index == INDEX_TZ_ALPHA)
    {
        ClockTz::TzSortByLocation();
    }
    else
    {
        ClockTz::TzSortByOffset();
    }
    ScrTzIter = ClockTz::FindTzById(ClockTz::ActiveTzId);
    UpdateTz();
} // End SelTzSort().



/*******************************************************************************
********************************************************************************
* Setup Color Cycling Screen
*
* This is the first of 2 color selection screens.  It allow for selection of
* whether or not main font colors will cycle on the main time screen, and  the
* rate at which they will do so.
********************************************************************************
*******************************************************************************/
static void SelSetupColors(int16_t);        // Enter the 2nd color selection screen.
static void SelColorCyclingItem(int16_t);   // Select whether or not to cycle.
static void SelIncPeriod(int16_t);          // Increment the color cycle period.
static void SelDecPeriod(int16_t);          // Decrement the color cycle period.
static void SelLargeIncPeriod(int16_t);     // Large increment color cycle period.
static void SelLargeDecPeriod(int16_t);     // Large decrement color cycle period.

// Indices into ScrColorCycling[].
static const size_t INDEX_FIXED        = 2;
static const size_t INDEX_CYCLE        = 3;
static const size_t INDEX_PERIOD_LABEL = 4;
static const size_t INDEX_PERIOD_VALUE = 5;
static const size_t INDEX_PERIOD_LESS  = 6;
static const size_t INDEX_PERIOD_MORE  = 7;
static const size_t INDEX_PERIOD_LLESS = 8;
static const size_t INDEX_PERIOD_MMORE = 9;

static scr_vec_t ScrColorCycling =
{ //      obj          tX0  tX1  tY0  tY1  oX0   oW  oY0   oH   oE  oC         lX0  lY0  txt            fs       lc           onClick          oLoPct  oHiPct   val font        enabled touchIndex
    { BACKGROUND,        0,   0,   0,   0,   0,   0,   0,   0,   0, BLACK,       0,   0, NULL,          0,      BLACK,       NULL,               0,     0,       0, NULL,         true },// 0.
    { LABEL,             0,   0,   0,   0,   0, 320,   0,  40,   0, BLACK,       0,   0, "COLOR CYCLING",1,     SYS_TXT_CLR, NULL,               0,     0,       0, &SYS_FONT_18, true },// 1.
    { RADIO_BUTTON,     30, 139,  55,  85,  30,  30,  55,  30,   5, WHITE,       0,   0, "FIXED",       2,      WHITE,       SelColorCyclingItem,0,     0,       0, NULL,         true },// 2.
    { RADIO_BUTTON,    180, 290,  55,  85, 180,  30,  55,  30,   5, WHITE,       0,   0, "CYCLE",       2,      WHITE,       SelColorCyclingItem,0,     0,       0, NULL,         true },// 3.
    { LABEL,             0,   0,   0,   0,   0, 310, 100,  40,   0, BLACK,       0,   0, "CYCLE PERIOD:",2,     WHITE,       NULL,               0,     0,       0, NULL,         true },// 4.
    { BUTTON_RECT,     120, 200, 140, 170, 120,  80, 140,  30,   0, WHITE,       0,   0, NULL,          2,      BLACK,       NULL,               0,     0,       0, NULL,         true },// 5.
    { BUTTON_ROUND,     80, 110, 140, 170,  80,  30, 140,   0,   0, WHITE,       0,   0, "<",           1,      BLACK,       SelDecPeriod,       0,     0,       0, &SYS_FONT_9,  true },// 6.
    { BUTTON_ROUND,    210, 240, 140, 170, 210,  30, 140,   0,   0, WHITE,       0,   0, ">",           1,      BLACK,       SelIncPeriod,       0,     0,       0, &SYS_FONT_9,  true },// 7.
    { BUTTON_ROUND,     40,  70, 140, 170,  40,  30, 140,   0,   0, WHITE,       0,   0, "<<",          1,      BLACK,       SelLargeDecPeriod,  0,     0,       0, &SYS_FONT_9,  true },// 8.
    { BUTTON_ROUND,    250, 280, 140, 170, 250,  30, 140,   0,   0, WHITE,       0,   0, ">>",          1,      BLACK,       SelLargeIncPeriod,  0,     0,       0, &SYS_FONT_9,  true },// 9.
    { BUTTON_RECT_RND,  20, 100, 200, 230,  20,  80, 200,  30,   8, SYS_BTN_CLR, 0,   0, "PREV",        1,      BLACK,       SelSetupScreen,     0,     0,       0, &SYS_FONT_9,  true },//10.
    { BUTTON_RECT_RND, 120, 200, 200, 230, 120,  80, 200,  30,   8, SYS_BTN_CLR, 0,   0, "HOME",        1,      BLACK,       SelMainScreen,      0,     0,       0, &SYS_FONT_9,  true },//11.
    { BUTTON_RECT_RND, 220, 300, 200, 230, 220,  80, 200,  30,   8, SYS_BTN_CLR, 0,   0, "NEXT",        1,      BLACK,       SelSetupColors,     0,     0,       0, &SYS_FONT_9,  true },//12.
    { TIMEOUT,           0,   0,   0,   0,   0,   0,   0,   0,   0, BLACK,       0,   0, NULL,          0,      BLACK,       SelMainScreen,      0,     0, TIMEOUT_MS, NULL,      true } //13.
}; // End ScrColorCycling[].

// Update the color cycling period based on its current value.
static void UpdatePeriodValue()
{
    div_t d = div((int)gTd.GetColorCyclePeriod(), 60);
    snprintf(StrBuf, STR_BUF_SIZE, "%02dm%02ds", d.quot, d.rem);
    ScrColorCycling[INDEX_PERIOD_VALUE].m_pTxt = StrBuf;
} // End UpdatePeriodValue().

// Display the COLOR setup screen.
static void SelColorCyclingScreen(int16_t)
{
    // Update the fixed/cycling radio buttons.
    bool cycleColors = gTd.IsColorCycling();
    ScrColorCycling[INDEX_FIXED].m_Val = !cycleColors;
    ScrColorCycling[INDEX_CYCLE].m_Val = cycleColors;

    // Enable/disable cycle period objects based on whether or not we're cycling.
    for (size_t i = INDEX_PERIOD_LABEL; i <= INDEX_PERIOD_MMORE; i++)
    {
        ScrColorCycling[i].m_Enable = cycleColors;
    }

    // Update the cycle period value and display the screen.
    UpdatePeriodValue();
    gGui.DrawScreen(&ScrColorCycling, SCR_REDRAW);
} // End SelColorCyclingScreen().

// Select whether or not to cycle.
static void SelColorCyclingItem(int16_t index)
{
    gTd.CycleColors(index == INDEX_CYCLE);
    SelColorCyclingScreen(0);
} // End SelColorCyclingItem().

// Constants for incrementing/decrementing the cycle rate.
static const int32_t PERIOD_SMALL_INC = 30;
static const int32_t PERIOD_LARGE_INC = 300;
static const int32_t PERIOD_MAX       = 3600;
static const int32_t PERIOD_MIN       = 10;

// Increment the color cycle period.
static void SelIncPeriod(int16_t)
{
    int32_t period = (int32_t)gTd.GetColorCyclePeriod();

    // Limit the size of the cycle period.
    if (period > PERIOD_MAX - PERIOD_SMALL_INC)
    {
        period = PERIOD_MAX;
    }
    else if (period < PERIOD_SMALL_INC)
    {
        period = PERIOD_SMALL_INC;
    }
    else
    {
        period += PERIOD_SMALL_INC;
    }

    // Update the period value and display the screen.
    gTd.SetColorCyclePeriod((float_t)period);
    UpdatePeriodValue();
    gGui.DrawScreen(&ScrColorCycling, SCR_REDRAW);
} // End SelIncPeriod().

// Decrement the color cycle period.
static void SelDecPeriod(int16_t)
{
    int32_t period = (int32_t)gTd.GetColorCyclePeriod();

    // Limit the size of the cycle period.
    if (period <= PERIOD_SMALL_INC)
    {
        period = PERIOD_MIN;
    }
    else
    {
        period -= PERIOD_SMALL_INC;
    }

    // Update the period value and display the screen.
    gTd.SetColorCyclePeriod((float_t)period);
    UpdatePeriodValue();
    gGui.DrawScreen(&ScrColorCycling, SCR_REDRAW);
} // End SelDecPeriod().

// Large increment color cycle period.
static void SelLargeIncPeriod(int16_t)
{
    int32_t period = (int32_t)gTd.GetColorCyclePeriod();

    // Limit the size of the cycle period.
    if (period > PERIOD_MAX - PERIOD_LARGE_INC)
    {
        period = PERIOD_MAX;
    }
    else if (period < PERIOD_SMALL_INC)
    {
        period = PERIOD_SMALL_INC;
    }
    else
    {
        period += PERIOD_LARGE_INC;
    }

    // Update the period value and display the screen.
    gTd.SetColorCyclePeriod((float_t)period);
    UpdatePeriodValue();
    gGui.DrawScreen(&ScrColorCycling, SCR_REDRAW);
} // End SelLargeIncPeriod().

// Large decrement color cycle period.
static void SelLargeDecPeriod(int16_t)
{
    int32_t period = (int32_t)gTd.GetColorCyclePeriod();

    // Limit the size of the cycle period.
    if (period <= PERIOD_LARGE_INC)
    {
        period = PERIOD_MIN;
    }
    else
    {
        period -= PERIOD_LARGE_INC;
    }

    // Update the period value and display the screen.
    gTd.SetColorCyclePeriod((float_t)period);
    UpdatePeriodValue();
    gGui.DrawScreen(&ScrColorCycling, SCR_REDRAW);
} // End SelLargeDecPeriod().



/*******************************************************************************
********************************************************************************
* Color Selection Screen
*
* This screen supports changing background, and optionally font colors.
* If color cycling is enabled, then only the background color may be changed.
* if color cycling is disabled, then background color, main (primary) color, and
* aus (secondary) colors may be be changed.
********************************************************************************
*******************************************************************************/
static void SelNextColor(int16_t);          // Select the next color.
static void SelPrevColor(int16_t);          // Select the previous color.
static void SelHSVColorChange(int16_t);     // Handle HSV color change.

// Indices into SelColorSelect[].
static const size_t INDEX_COLOR_LABEL = 1;
static const size_t INDEX_COLOR_PREV  = 2;
static const size_t INDEX_COLOR_NEXT  = 3;
static const size_t INDEX_H_SLIDER    = 4;
static const size_t INDEX_S_SLIDER    = 5;
static const size_t INDEX_V_SLIDER    = 6;
static const size_t INDEX_PRI_SWATCH  = 7;
static const size_t INDEX_SEC_SWATCH  = 8;

static scr_vec_t SelColorSelect =
{ //      obj          tX0  tX1  tY0  tY1  oX0   oW  oY0   oH   oE  oC         lX0  lY0  txt            fs       lc           onClick          oLoPct  oHiPct   val font        enabled touchIndex
    { BACKGROUND,        0,   0,   0,   0,   0,   0,   0,   0,   0, BLACK,       0,   0, NULL,           0,      BLACK,       NULL,               0,     0,      0, NULL,         true },// 0.
    { LABEL,             0,   0,   0,   0,   0, 320,   0,  40,   0, BLACK,       0,   0, NULL,           1,      SYS_TXT_CLR, NULL,               0,     0,      0, &SYS_FONT_18, true },// 1.
    { BUTTON_ROUND,      5,  35,  45,  75,   5,  30,  45,   0,   0, SYS_BTN_CLR, 0,   0, "<",            1,      BLACK,       SelPrevColor,       0,     0,      0, &SYS_FONT_9,  true },// 2.
    { BUTTON_ROUND,    285, 315,  45,  85, 285,  30,  45,   0,   0, SYS_BTN_CLR, 0,   0, ">",            1,      BLACK,       SelNextColor,       0,     0,      0, &SYS_FONT_9,  true },// 3.
    { BUTTON_SLH,       50, 270,  50,  70,  50,  13,  50,  20,   5, WHITE,       0,   0, "H",            2,      BLACK,       SelHSVColorChange,  0,   100,      0, NULL,         true },// 4.
    { BUTTON_SLH,       50, 270,  75,  95,  50,  13,  75,  20,   5, WHITE,       0,   0, "S",            2,      BLACK,       SelHSVColorChange,  0,   100,      0, NULL,         true },// 5.
    { BUTTON_SLH,       50, 270, 100, 120,  50,  13, 100,  20,   5, WHITE,       0,   0, "V",            2,      BLACK,       SelHSVColorChange,  0,   100,      0, NULL,         true },// 6.
    { BUTTON_RECT,       0,   0,   0,   0,   0, 160, 125,  70,   0, WHITE,       0,   0, "12:34",        2,      SYS_TXT_CLR, NULL,               0,     0,      0, &SYS_FONT_18, true },// 7.
    { BUTTON_RECT,       0,   0,   0,   0, 160, 160, 125,  70,   0, WHITE,       0,   0, "Monday",       2,      SYS_TXT_CLR, NULL,               0,     0,      0, &SYS_FONT_9,  true },// 8.
    { BUTTON_RECT_RND,  20, 100, 200, 230,  20,  80, 200,  30,   8, SYS_BTN_CLR, 0,   0, "PREV",         1,      BLACK,       SelColorCyclingScreen,0,   0,      0, &SYS_FONT_9,  true },// 9.
    { BUTTON_RECT_RND, 120, 200, 200, 230, 120,  80, 200,  30,   8, SYS_BTN_CLR, 0,   0, "HOME",         1,      BLACK,       SelMainScreen,      0,     0,      0, &SYS_FONT_9,  true },//10.
    { TIMEOUT,           0,   0,   0,   0,   0,   0,   0,   0,   0, BLACK,       0,   0,  NULL,          0,      BLACK,       SelMainScreen,      0,     0, TIMEOUT_MS, NULL,     true } //11.
}; // End SelColorSelect[].

// Enum for selecting which color to work on.
enum COLOR_TYPE
{
    COLOR_BACKGROUND = 0,
    COLOR_PRIMARY,
    COLOR_SECONDARY
}; // End COLOR_TYPE.
// Variable to keep track of current color being worked on.
static COLOR_TYPE ColorSelector = COLOR_BACKGROUND;

// Set up the display and its colors based on current settings.
// Argument updateHSV enables the slider positions if 'true'.
static void SetupFixedColors(bool updateHSV)
{
    rgb16_t bgColor  = BLACK;
    rgb16_t priColor = BLACK;
    rgb16_t secColor = BLACK;
    hsv_t   h = 0.0;
    hsv_t   s = 0.0;
    hsv_t   v = 0.0;

    // Fetch the current colors and set up our screen sample.
    gTd.GetColors(priColor, secColor, bgColor);
    SelColorSelect[INDEX_PRI_SWATCH].m_OC = bgColor;
    SelColorSelect[INDEX_SEC_SWATCH].m_OC = bgColor;
    SelColorSelect[INDEX_PRI_SWATCH].m_LC = priColor;
    SelColorSelect[INDEX_SEC_SWATCH].m_LC = secColor;

    // Setup our screen title and next/prev button color based on working color type.
    switch (ColorSelector)
    {
    case COLOR_BACKGROUND:
        ClockHelper::Rgb162Hsv(bgColor, h, s, v);
        SelColorSelect[INDEX_COLOR_LABEL].m_pTxt = "BG COLOR";
        SelColorSelect[INDEX_COLOR_PREV].m_OC = SYS_DRK_CLR;
        SelColorSelect[INDEX_COLOR_NEXT].m_OC = SYS_BTN_CLR;
        break;
    case COLOR_SECONDARY:
        ClockHelper::Rgb162Hsv(secColor, h, s, v);
        SelColorSelect[INDEX_COLOR_LABEL].m_pTxt = "SEC COLOR";
        SelColorSelect[INDEX_COLOR_PREV].m_OC = SYS_BTN_CLR;
        SelColorSelect[INDEX_COLOR_NEXT].m_OC = SYS_DRK_CLR;
        break;
    case COLOR_PRIMARY:
        ClockHelper::Rgb162Hsv(priColor, h, s, v);
        SelColorSelect[INDEX_COLOR_LABEL].m_pTxt = "PRI COLOR";
        SelColorSelect[INDEX_COLOR_PREV].m_OC = SYS_BTN_CLR;
        SelColorSelect[INDEX_COLOR_NEXT].m_OC = SYS_BTN_CLR;
        break;
    default:
        break;
    }

    // Update the color slider positions only if told to do so.
    if (updateHSV)
    {
        SelColorSelect[INDEX_H_SLIDER].m_Val = (int32_t)(h * 100);
        SelColorSelect[INDEX_S_SLIDER].m_Val = (int32_t)(s * 100);
        SelColorSelect[INDEX_V_SLIDER].m_Val = (int32_t)(v * 100);
    }

    // Only allow background color to change if cycling is enabled.
    bool cycleColors = gTd.IsColorCycling();
    SelColorSelect[INDEX_COLOR_PREV].m_Enable = !cycleColors;
    SelColorSelect[INDEX_COLOR_NEXT].m_Enable = !cycleColors;
} // End SetupFixedColors().

// Enter the 2nd color selection screen.
static void SelSetupColors(int16_t)
{
    ColorSelector = COLOR_BACKGROUND;
    SetupFixedColors(true);
    gGui.DrawScreen(&SelColorSelect, SCR_REDRAW);
} // End SelSetupColors().

// Select the next color.
static void SelNextColor(int16_t)
{
    // Wrap around if last color is currently active.
    if (ColorSelector == COLOR_SECONDARY)
    {
        ColorSelector = COLOR_BACKGROUND;
    }
    else
    {
        int s = (int)ColorSelector;
        ColorSelector = (COLOR_TYPE)(s + 1);
    }

    // Display changes.
    SetupFixedColors(true);
    gGui.DrawScreen(&SelColorSelect, SCR_REDRAW);
} // End SelNextColor().

// Select the previous color.
static void SelPrevColor(int16_t)
{
    // Wrap around if first color is currently active.
    if (ColorSelector == COLOR_BACKGROUND)
    {
        ColorSelector = COLOR_SECONDARY;
    }
    else
    {
        int s = (int)ColorSelector;
        ColorSelector = (COLOR_TYPE)(s - 1);
    }

    // Display changes.
    SetupFixedColors(true);
    gGui.DrawScreen(&SelColorSelect, SCR_REDRAW);
} // End SelPrevColor().

 // Handle HSV color change.
static void SelHSVColorChange(int16_t)
{
    // Calculate the HSV values of the current slider positions.
    hsv_t h = (hsv_t)SelColorSelect[INDEX_H_SLIDER].m_Val / 100.0;
    hsv_t s = (hsv_t)SelColorSelect[INDEX_S_SLIDER].m_Val / 100.0;
    hsv_t v = (hsv_t)SelColorSelect[INDEX_V_SLIDER].m_Val / 100.0;

    // Fetch the currently active display colors.
    rgb16_t bgColor  = BLACK;
    rgb16_t priColor = BLACK;
    rgb16_t secColor = BLACK;
    gTd.GetColors(priColor, secColor, bgColor);

    // Convert the current slider HSV values to an RGB color and save it
    // as the current color type value.
    rgb16_t rgb = ClockHelper::Hsv2Rgb16(h, s, v);
    if (ColorSelector == COLOR_BACKGROUND)
    {
        bgColor = rgb;
    }
    else if (ColorSelector == COLOR_PRIMARY)
    {
        priColor = rgb;
    }
    else
    {
        secColor = rgb;
    }

    // Save the (possibly) new color values and display the screen.
    gTd.SetColors(priColor, secColor, bgColor);
    SetupFixedColors(false);
    gGui.DrawScreen(&SelColorSelect, SCR_REDRAW);
} // End SelHSVColorChange().



/*******************************************************************************
********************************************************************************
* WiFi Credentials Setup Screen
*
* This screen allows the user to specify an SSID and password for the WiFi
* network that the clock will connect to.  The user can scan for local networks,
* or enter an SSID via the touch keypad.  Note that this screen sets the 'oE'
* value of the BACKGROUND object, which causes the screen to be updated on
* each scan in order to service the touchpad keyboard.
********************************************************************************
*******************************************************************************/
static void ExWifiSetup(int16_t);           // Display the credentials screen.
static void SelClearWiFiBuf(int16_t index); // Clear an input buffer.
static void SelWiFiInput(int16_t index);    // Select an input buffer.
static void SelNextSsid(int16_t);           // Display next scanned network SSID.
static void SelPrevSsid(int16_t);           // Display previous scanned network SSID.
static void SelScanNets(int16_t);           // Scan for local networks.
static void SelUseNet(int16_t);             // Test/use current credentials.
static void SelNtpScreen(int16_t);          // Select the NTP server screen.

// Indices into ScrWiFi[].
static const size_t INDEX_PREV_SSID  = 2;
static const size_t INDEX_CLEAR_SSID = 3;
static const size_t INDEX_SSID       = 4;
static const size_t INDEX_NEXT_SSID  = 5;
static const size_t INDEX_CLEAR_PWD  = 6;
static const size_t INDEX_PWD        = 7;

// Local data for managing the WiFi related string data.
static const size_t MAX_LARGE_CHARS  = 18; // This is the maximum number of large
                                           // font characters that can be displayed
                                           // in a prompt box.  Strings longer
                                           // than this will be displayed in a
                                           // smaller font.

// Input buffers for SSID, password, and NVS server URLs.
static InputBuffer SsidBuf;
static InputBuffer PwdBuf;
static InputBuffer Ntp1Buf;
static InputBuffer Ntp2Buf;

static size_t ActiveWiFiInput = INDEX_SSID; // The currently active input buffer.
static size_t CurrentNetIndex = 0;          // Index of current scanned network.
static size_t NumNets = 0;                  // Number of scanned networks found.

static scr_vec_t ScrWiFi =
{ //      obj          tX0  tX1  tY0  tY1  oX0   oW  oY0   oH   oE  oC          lX0  lY0  txt            fs       lc           onClick          oLoPct  oHiPct   val font        enabled touchIndex
    { BACKGROUND,        0,   0,   0,   0,   0,   0,   0,   0,   1, BLACK,       0,   0, NULL,           0,      BLACK,       ExWifiSetup,         0,      0,     0, NULL,         true },// 0.
    { LABEL,             0,   0,   0,   0,   0, 320,   0,  40,   0, BLACK,       0,   0, "WiFi  SETUP",  1,      SYS_TXT_CLR, NULL,                0,      0,     0, &SYS_FONT_18, true },// 1.
    { BUTTON_ROUND,      5,  25,  40,  60,   5,  20,  40,   0,   0, SYS_BTN_CLR, 0,   0, "<",            2,      BLACK,       SelPrevSsid,         0,      0,     0, NULL,         true },// 2.
    { BUTTON_ROUND,     35,  55,  40,  60,  35,  20,  40,   0,   0, RED,         0,   0, "x",            2,      BLACK,       SelClearWiFiBuf,     0,      0,     0, NULL,         true },// 3.
    { BUTTON_RECT,      60, 290,  40,  60,  60, 230,  40,  20,   0, WHITE,       0,   0, NULL,           2,      BLACK,       SelWiFiInput,        0,      0,     0, NULL,         true },// 4.
    { BUTTON_ROUND,    295, 315,  40,  60, 295,  20,  40,   0,   0, SYS_BTN_CLR, 0,   0, ">",            2,      BLACK,       SelNextSsid,         0,      0,     0, NULL,         true },// 5.
    { BUTTON_ROUND,     35,  55,  70,  90,  35,  20,  70,   0,   0, RED,         0,   0, "x",            2,      BLACK,       SelClearWiFiBuf,     0,      0,     0, NULL,         true },// 6.
    { BUTTON_RECT,      60, 290,  70,  90,  60, 230,  70,  20,   0, WHITE,       0,   0, NULL,           2,      BLACK,       SelWiFiInput,        0,      0,     0, NULL,         true },// 7.
    { BUTTON_RECT_RND,   4,  64,  95, 115,   4,  60,  95,  20,   6, SYS_BTN_CLR, 0,   0, "PREV",         1,      BLACK,       SelSetupScreen,      0,      0,     0, &SYS_FONT_9,  true },// 8.
    { BUTTON_RECT_RND,  67, 127,  95, 115,  67,  60,  95,  20,   6, SYS_BTN_CLR, 0,   0, "HOME",         1,      BLACK,       SelMainScreen,       0,      0,     0, &SYS_FONT_9,  true },// 9.
    { BUTTON_RECT_RND, 130, 190,  95, 115, 130,  60,  95,  20,   6, SYS_BTN_CLR, 0,   0, "SCAN",         1,      BLACK,       SelScanNets,         0,      0,     0, &SYS_FONT_9,  true },//10.
    { BUTTON_RECT_RND, 193, 253,  95, 115, 193,  60,  95,  20,   6, SYS_BTN_CLR, 0,   0, "USE",          1,      BLACK,       SelUseNet,           0,      0,     0, &SYS_FONT_9,  true },//11.
    { BUTTON_RECT_RND, 256, 316,  95, 115, 256,  60,  95,  20,   6, SYS_BTN_CLR, 0,   0, "NEXT",         1,      BLACK,       SelNtpScreen,        0,      0,     0, &SYS_FONT_9,  true },//12.
    { TIMEOUT,           0,   0,   0,   0,   0,   0,   0,   0,   0, BLACK,       0,   0, NULL,           0,      BLACK,       SelMainScreen,       0,      0, TIMEOUT_MS, NULL,    true } //13.
}; // End ScrWiFi[].

// Update the input buffers and next/prev icons of the WiFi credentials screen.
static void UpdateWiFiScreen()
{
    // Initialize the SSID and password buffers to currently saved values.
    // Adjust the font size of each based on size of the currently buffered string.
    ScrWiFi[INDEX_SSID].m_pTxt = SsidBuf.GetBuf();
    ScrWiFi[INDEX_SSID].m_FS = strlen(ScrWiFi[INDEX_SSID].m_pTxt) > MAX_LARGE_CHARS ? 1 : 2;
    ScrWiFi[INDEX_PWD].m_pTxt = PwdBuf.GetBuf();
    ScrWiFi[INDEX_PWD].m_FS = strlen(ScrWiFi[INDEX_PWD].m_pTxt) > MAX_LARGE_CHARS ? 1 : 2;

    // If a scan has been performed and there are multiple networks found,
    // enable the next and previous network icons.
    if (NumNets > 1)
    {
        // Multiple networks found. Enable next/prev icons.
        ScrWiFi[INDEX_NEXT_SSID].m_Enable = true;
        ScrWiFi[INDEX_PREV_SSID].m_Enable = true;
        ScrWiFi[INDEX_NEXT_SSID].m_OC =
            (CurrentNetIndex == NumNets - 1) ? SYS_DRK_CLR : SYS_BTN_CLR;
        ScrWiFi[INDEX_PREV_SSID].m_OC = CurrentNetIndex ? SYS_BTN_CLR : SYS_DRK_CLR;
    }
    else
    {
        // None or one network found.  Disable next/prev icons.
        ScrWiFi[INDEX_NEXT_SSID].m_Enable = false;
        ScrWiFi[INDEX_PREV_SSID].m_Enable = false;
    }

    // Highlight the selected input buffer.
    ScrWiFi[INDEX_SSID].m_OC = (ActiveWiFiInput == INDEX_SSID) ? WHITE : GRAY;
    ScrWiFi[INDEX_PWD].m_OC  = (ActiveWiFiInput == INDEX_PWD) ? WHITE : GRAY;
} // End UpdateWiFiScreen().

// Display the WiFi credentials setup screen.
static void SelWiFiScreen(int16_t)
{
    // Set the strings to default values if they are currently empty.
    char *s = ClockWiFi::GetSsid();
    SsidBuf.AddString((s && *s) ? s : "SSID");
    s = ClockWiFi::GetPwd();
    PwdBuf.AddString((s && *s) ? s : "PWD");
    s = ClockWiFi::GetNtpServer1();
    Ntp1Buf.AddString((s && *s) ? s : "NTP SERVER 1");
    s = ClockWiFi::GetNtpServer2();
    Ntp2Buf.AddString((s && *s) ? s : "NTP SERVER 2");

    // Start with the SSID input active and no networks found.
    SelWiFiInput(INDEX_SSID);
    NumNets = 0;
    CurrentNetIndex = 0;
    ActiveWiFiInput = INDEX_SSID;

    // Update the screen and display it.
    UpdateWiFiScreen();
    gGui.DrawScreen(&ScrWiFi, SCR_REDRAW);
} // End SelWiFiScreen().

// Display the credentials screen and handle touch keypad input.
static void ExWifiSetup(int16_t)
{
    // Clear out the upper part of the display which holds the buffer info
    // and navigation buttons.
    gCanvas.fillRect(0, 0, gCanvas.width(), gCanvas.height() - gTs.GetKbHeight(),
                     BLACK);

    // Show the keypad in the lower part of the screen.
    gTs.SetBgColor(BLACK);
    gTs.SetKeyColor(BLUE);
    gTs.ShowKeyboard();

    // Handle any key presses by sending the input to the currently selected buffer.
    char c = gTs.GetKeyPress();
    if (c)
    {
        if (ActiveWiFiInput == INDEX_SSID)
        {
            SsidBuf.AddChar(c);
        }
        else
        {
            PwdBuf.AddChar(c);
        }
    }
    // Update the screen.
    UpdateWiFiScreen();
} // End ExWifiSetup().

// Select an input buffer (SSID or password).
static void SelWiFiInput(int16_t index)
{
    ActiveWiFiInput = index;
    UpdateWiFiScreen();
} // End SelWiFiInput().

 // Clear an input buffer (SSID or password).
static void SelClearWiFiBuf(int16_t index)
{
    if (index == INDEX_CLEAR_SSID)
    {
        ActiveWiFiInput = INDEX_SSID;
        SsidBuf.Clear();
    }
    else
    {
        ActiveWiFiInput = INDEX_PWD;
        PwdBuf.Clear();
    }
    UpdateWiFiScreen();
} // End SelClearWiFiBuf().


// Display next scanned network SSID.
static void SelNextSsid(int16_t)
{
    // Index to the next network, wrapping around if needed.
    if (NumNets > 1)
    {
        if (CurrentNetIndex < NumNets - 1)
        {
            ++CurrentNetIndex;
        }
        else
        {
            CurrentNetIndex = 0;
        }
        // Copy the SSID to the input buffer and update the screen.
        SsidBuf.AddString(ClockWiFi::GetNet(CurrentNetIndex));
        UpdateWiFiScreen();
    }
} // End SelNextSsid().

// Display previous scanned network SSID.
static void SelPrevSsid(int16_t)
{
    // Index to the previous network, wrapping around if needed.
    if (NumNets > 1)
    {
        if (CurrentNetIndex > 0)
        {
            --CurrentNetIndex;
        }
        else
        {
            CurrentNetIndex = NumNets - 1;
        }
        // Copy the SSID to the input buffer and update the screen.
        SsidBuf.AddString(ClockWiFi::GetNet(CurrentNetIndex));
        UpdateWiFiScreen();
    }
} // End SelPrevSsid().

// Scan for local networks.
static void SelScanNets(int16_t)
{
    // Perform the actual network scan.
    ClockWiFi::Scan();
    CurrentNetIndex = 0;

    // Were any networks found?
    NumNets = ClockWiFi::GetNumNets();
    if (NumNets)
    {
        // Found at least one network.  Copy its SSID to our input buffer and
        // display a status screen know how many nets were found.
        SsidBuf.AddString(ClockWiFi::GetNet(CurrentNetIndex));
        snprintf(StrBuf, STR_BUF_SIZE, "Found %d %s", NumNets, NumNets > 1 ? "Networks" : "Network");
        ShowStatus(StrBuf, WHITE, &SYS_FONT_18, 1, BLUE_MID);
    }
    else
    {
        // No networks were found.  Let the user know.
        ShowStatus("No Networks Found", WHITE, &SYS_FONT_18, 1, RED_MID);
    }
    // Delay to let the user see the results, then go back to our setup screen.
    delay(3000);
    UpdateWiFiScreen();
} // End SelScanNets().

// Test/use current credentials.
static void SelUseNet(int16_t)
{
    // Copy our input buffer strings to the WiFi object to make them current.
    ClockWiFi::SetSsid(SsidBuf.GetBuf());
    ClockWiFi::SetPwd(PwdBuf.GetBuf());
    ClockWiFi::SetNtpServer1(Ntp1Buf.GetBuf());
    ClockWiFi::SetNtpServer2(Ntp2Buf.GetBuf());

    // Attempt to connect to the network and keep the user informed about what
    // is happening.
    ShowStatus("Testing...", WHITE, &SYS_FONT_18, 1, BLUE_MID);
    ClockWiFi::Connect();

    // Was the connection successful?
    if (ClockWiFi::WiFiConnected())
    {
        // Yes, connection was successful.  Let the user know.
        ShowStatus("Network Connected", WHITE, &SYS_FONT_18, 1, GREEN_MID);
        delay(3000);

        // Let's press our luck and see if we also found an NTP server.
        if (ClockWiFi::NtpConnected())
        {
            // Yes, we found an NTP server.  Let the user knoww.
            ClockRtc &rtc = ClockRtc::Instance();
            rtc.SetClock();
            ShowStatus("NTP Connected", WHITE, &SYS_FONT_18, 1, GREEN_MID);
        }
        else
        {
            // Didn't find an NTP server.  Let the user know.
            ShowStatus("NTP Failed", WHITE, &SYS_FONT_18, 1, RED_MID);
        }

        // Connection is successful so start the server.
        InitClockWebServer();
    }
    else
    {
        // Couldn't connect to a network.  Let the user know.
        ShowStatus("Network Failed", WHITE, &SYS_FONT_18, 1, RED_MID);
    }
    // Delay a while so the user can see the results.
    delay(3000);
} // End SelUseNet().



/*******************************************************************************
********************************************************************************
* NTP Servers Setup Screen
*
* This screen allows the user to specify up to 2 NTP server URLs.
* Note that this screen sets the 'oE' value of the BACKGROUND object, which
* causes the screen to be updated on each scan in order to service the touchpad
* keyboard.
********************************************************************************
*******************************************************************************/
static void ExNtpSetup(int16_t);            // Display the NTP server screen.
static void SelClearNtpBuf(int16_t index);  // Clear an NTP input buffer.
static void SelNtpInput(int16_t index);     // Select an NTP input buffer.
static void SelUseNtp(int16_t);             // Test/use current credentials.

// Indices into ScrNtp[].
static const size_t INDEX_CLEAR_NTP1 = 2;
static const size_t INDEX_NTP1       = 3;
static const size_t INDEX_CLEAR_NTP2 = 4;
static const size_t INDEX_NTP2       = 5;

static size_t ActiveNtpInput = INDEX_NTP1;

static scr_vec_t ScrNtp =
{ //      obj          tX0  tX1  tY0  tY1  oX0   oW  oY0   oH   oE  oC          lX0  lY0  txt            fs       lc           onClick          oLoPct  oHiPct   val font        enabled touchIndex
    { BACKGROUND,        0,   0,   0,   0,   0,   0,   0,   0,   1, BLACK,       0,   0, NULL,           0,      BLACK,       ExNtpSetup,          0,      0,     0, NULL,         true },// 0.
    { LABEL,             0,   0,   0,   0,   0, 320,   0,  40,   0, BLACK,       0,   0, "NTP SETUP",    1,      SYS_TXT_CLR, NULL,                0,      0,     0, &SYS_FONT_18, true },// 1.
    { BUTTON_ROUND,     35,  55,  40,  60,  35,  20,  40,   0,   0, RED,         0,   0, "x",            2,      BLACK,       SelClearNtpBuf,      0,      0,     0, NULL,         true },// 2.
    { BUTTON_RECT,      60, 290,  40,  60,  60, 230,  40,  20,   0, WHITE,       0,   0, NULL,           2,      BLACK,       SelNtpInput,         0,      0,     0, NULL,         true },// 3.
    { BUTTON_ROUND,     35,  55,  70,  90,  35,  20,  70,   0,   0, RED,         0,   0, "x",            2,      BLACK,       SelClearNtpBuf,      0,      0,     0, NULL,         true },// 4.
    { BUTTON_RECT,      60, 290,  70,  90,  60, 230,  70,  20,   0, WHITE,       0,   0, NULL,           2,      BLACK,       SelNtpInput,         0,      0,     0, NULL,         true },// 5.
    { BUTTON_RECT_RND,  20, 100,  95, 115,  20,  80,  95,  20,   6, SYS_BTN_CLR, 0,   0, "PREV",         1,      BLACK,       SelWiFiScreen,       0,      0,     0, &SYS_FONT_9,  true },// 6.
    { BUTTON_RECT_RND, 120, 200,  95, 115, 120,  80,  95,  20,   6, SYS_BTN_CLR, 0,   0, "HOME",         1,      BLACK,       SelMainScreen,       0,      0,     0, &SYS_FONT_9,  true },// 7.
    { BUTTON_RECT_RND, 220, 300,  95, 115, 220,  80,  95,  20,   6, SYS_BTN_CLR, 0,   0, "USE",          1,      BLACK,       SelUseNtp,           0,      0,     0, &SYS_FONT_9,  true },// 8.
    { TIMEOUT,           0,   0,   0,   0,   0,   0,   0,   0,   0, BLACK,       0,   0,  NULL,          0,      BLACK,       SelMainScreen,       0,      0, TIMEOUT_MS, NULL,    true } // 9.
}; // End ScrNtp[].

// Update the input buffers and next/prev icons of the NTP server screen.
static void UpdateNtpScreen()
{
    // Initialize both NTP server input buffers and adjust the font size of each
    // based on size of the currently buffered string.
    ScrNtp[INDEX_NTP1].m_pTxt = Ntp1Buf.GetBuf();
    ScrNtp[INDEX_NTP1].m_FS = strlen(ScrWiFi[INDEX_NTP1].m_pTxt) > MAX_LARGE_CHARS ? 1 : 2;
    ScrNtp[INDEX_NTP1].m_OC = (ActiveNtpInput == INDEX_NTP1) ? WHITE : GRAY;
    ScrNtp[INDEX_NTP2].m_pTxt = Ntp2Buf.GetBuf();
    ScrNtp[INDEX_NTP2].m_FS = strlen(ScrWiFi[INDEX_NTP2].m_pTxt) > MAX_LARGE_CHARS ? 1 : 2;
    ScrNtp[INDEX_NTP2].m_OC = (ActiveNtpInput == INDEX_NTP2) ? WHITE : GRAY;
} // End UpdateNtpScreen().

// Select the NTP server screen.
static void SelNtpScreen(int16_t)
{
    SelNtpInput(INDEX_NTP1);
    ActiveNtpInput = INDEX_NTP1;
    UpdateNtpScreen();
    gGui.DrawScreen(&ScrNtp, SCR_REDRAW);
} // End SelNtpScreen().

// Display the NTP server screen.
static void ExNtpSetup(int16_t)
{
    // Clear out the upper part of the display which holds the buffer info
    // and navigation buttons.
    gCanvas.fillRect(0, 0, gCanvas.width(), gCanvas.height() - gTs.GetKbHeight(),
                     BLACK);

    // Show the keypad in the lower part of the screen.
    gTs.SetBgColor(BLACK);
    gTs.SetKeyColor(BLUE);
    gTs.ShowKeyboard();

    // Handle any key presses by sending the input to the currently selected buffer.
    char c = gTs.GetKeyPress();
    if (c)
    {
        if (ActiveNtpInput == INDEX_NTP1)
        {
            Ntp1Buf.AddChar(c);
        }
        else
        {
            Ntp2Buf.AddChar(c);
        }
    }
    // Update the screen.
    UpdateNtpScreen();
} // End ExNtpSetup().

// Clear an NTP input buffer.
static void SelClearNtpBuf(int16_t index)
{
    if (index == INDEX_CLEAR_NTP1)
    {
        ActiveNtpInput = INDEX_NTP1;
        Ntp1Buf.Clear();
    }
    else
    {
        ActiveNtpInput = INDEX_NTP2;
        Ntp2Buf.Clear();
    }
    UpdateNtpScreen();
} // End SelClearNtpBuf().

// Select an NTP input buffer.
static void SelNtpInput(int16_t index)
{
    ActiveNtpInput = index;
    UpdateNtpScreen();
} // End SelNtpInput().

// Test/use current credentials.
static void SelUseNtp(int16_t)
{
    SelUseNet(0);
} // End SelUseNtp().



/*******************************************************************************
********************************************************************************
* SAVE user settings.
********************************************************************************
*******************************************************************************/
static void ExSaveSetup(int16_t)
{
    // Attempt to save our setup data.
    ClockNvs &nvs = ClockNvs::Instance();
    if (nvs.Save())
    {
        // Save was successful, let the user know.
        ShowStatus("Save succeeded.", WHITE, &SYS_FONT_18, 1, GREEN_MID);
    }
    else
    {
        // Save failed, let the user know.
        ShowStatus("Save FAILED.", WHITE, &SYS_FONT_18, 1, RED_MID);
    }
    // Delay a while so the user can see the results.
    delay(3000);
    SelSetupScreen(0);
} // End ExSaveSetup().



/*******************************************************************************
********************************************************************************
* RESTORE user settings.
********************************************************************************
*******************************************************************************/
static void ExRstrSetup(int16_t)
{
    // Let the user know we're trying to restore.
    ShowStatus("Restoring settings...", WHITE, &SYS_FONT_18, 1, BLUE_MID);
    ClockNvs &nvs = ClockNvs::Instance();
    if (nvs.Restore())
    {
        // Restore was successful, let the user know.
         ShowStatus("Restore succeeded.", WHITE, &SYS_FONT_18, 1, GREEN_MID);
    }
    else
    {
        // Restore failed.  Let the user know.
         ShowStatus("Restore FAILED.", WHITE, &SYS_FONT_18, 1, RED_MID);
    }
    // Delay a while so the user can see the results.
    delay(3000);
    SelSetupScreen(0);
} // End ExRstrSetup().



/*******************************************************************************
********************************************************************************
* Select Restore Screen
*
* Since this operation will cause the loss of all user settings, a verification
* screen is added.  This warns the user of the consequences andforces the user
* to confirm it.
********************************************************************************
*******************************************************************************/
static void SelResetContinue(int16_t);  // Perform the actual reset operation.

static scr_vec_t ScrReset =
{ //      obj          tX0  tX1  tY0  tY1  oX0   oW  oY0   oH   oE  oC         lX0  lY0  txt            fs       lc           onClick          oLoPct  oHiPct   val font        enabled touchIndex
    { BACKGROUND,        0,   0,   0,   0,   0,   0,   0,   0,   0, RED,         0,   0, NULL,           0,  BLACK,           NULL,              0,     0,     0, NULL,         true },// 0.
    { LABEL,             0,   0,   0,   0,   0, 320,   0,  40,   0, RED,         0,   0, "FACTORY RESET",1,  GREEN,           NULL,              0,     0,     0, &SYS_FONT_18, true },// 1.
    { LABEL,             0,   0,   0,   0,   0, 320,  50,  40,   0, RED,         0,   0, "! WARNING !",  1,  WHITE,           NULL,              0,     0,     0, &SYS_FONT_18, true },// 2.
    { LABEL,             0,   0,   0,   0,   0, 320, 100,  20,   0, RED,         0,   0, "WILL RESET ALL SETTINGS", 1, WHITE, NULL,              0,     0,     0, &SYS_FONT_9,  true },// 3.
    { LABEL,             0,   0,   0,   0,   0, 320, 120,  20,   0, RED,         0,   0, "TO FACTORY DEFAULTS", 1, WHITE,     NULL,              0,     0,     0, &SYS_FONT_9,  true },// 4.
    { BUTTON_RECT_RND,  33, 143, 170, 210,  33, 110, 170,  40,   8, BLACK,       0,   0, NULL,           1,  BLACK,           SelSetupScreen,    0,     0,     0, NULL,         true },// 5.
    { BUTTON_RECT_RND,   0,   0,   0,   0,  38, 100, 175,  30,   8, GRAY,        0,   0, "ABORT",        1,  GREEN,           NULL,              0,     0,     0, &SYS_FONT_9,  true },// 6.
    { BUTTON_RECT_RND, 177, 287, 170, 210, 177, 110, 170,  40,   8, BLACK,       0,   0, NULL,           1,  BLACK,           SelResetContinue,  0,     0,     0, NULL,         true },// 7.
    { BUTTON_RECT_RND,   0,   0,   0,   0, 182, 100, 175,  30,   8, GRAY,        0,   0, "CONTINUE",     1,  RED,             NULL,              0,     0,     0, &SYS_FONT_9,  true },// 8.
    { TIMEOUT,           0,   0,   0,   0,   0,   0,   0,   0,   0, BLACK,       0,   0, NULL,           0,  BLACK,           SelMainScreen,     0,     0, TIMEOUT_MS, NULL,    true } // 9.
}; // End ScrReset[].

// Display the FACTORY RESET screen.
static void ExResetSetup(int16_t)
{
    gGui.DrawScreen(&ScrReset, SCR_REDRAW);
} // End ExResetSetup().

// Perform the actual reset operation.
static void SelResetContinue(int16_t)
{
    // Let the user know we're resetting.
    ShowStatus("Resetting...", WHITE, &SYS_FONT_18, 1, BLUE_MID);
    delay(1000);

    // Perform the reset.
    ClockNvs &nvs = ClockNvs::Instance();
    nvs.FactoryReset();

    // Factory reset should never return.  If it does, then it failed.
    ShowStatus("Reset FAILED.", WHITE, &SYS_FONT_18, 1, RED_MID);
    delay(3000);
    SelSetupScreen(0);
} // End SelResetContinue().


/*******************************************************************************
* TsKeypad.h
*
* Declares the TsKeypad class.  This class implements a soft keypad
* containing all lower and upper case letters, numbers, and special characters.
* Based on work by HazardsMinds at
*   https://forum.arduino.cc/t/adafruit-tft-touchscreen-keypad-for-touchscreen/347024
*
* Changes include:
*   - Making colors and vertical offset settable.
*   - Adding optional key repeat.
*   - Use of graphics canvas to eliminate flicker on screen updates.
*   - Replacement of most magic numbers with named constants.
*   - Changed labels on special keys (special, spacebar, return, backspace).
*   - Changed to singleton class.
*   - Many optimizations and improvements.
* Assumes:
*   - Adafruit 2.8" TFT display (320w x 240h) with resistive touch screen:
*      https://www.adafruit.com/product/1770
*   - Screen uses landscape layout only (rotation either 1 or 3).
*   - Mostly uses default font at 2x size.
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

#if !defined TSKEYPAD_H
#define TSKEYPAD_H

#include <Adafruit_GFX.h>       // Core graphics library.
#include <Adafruit_ILI9341.h>   // For Adafruit 2.8" TFT display.
#include "TouchScreen.h"        // For the display resistive touch screen.
#include "ClockHelper.h"        // For hsv_t and rgb16_t.


/*******************************************************************************
* TsKeypad class
*
* This class implements a touch keyboard for the Adafruit 2.8" TFT display with
* resistive touch pad.  It is implemented as a singleton.
*
* Note: This class only works with the display in landscape mode (rotation 1 or 3).
*******************************************************************************/
class TsKeypad : public TouchScreen
{
public:
    /***************************************************************************
    * Instance()
    *
    * This class is implemented as a singleton.  In order to create the single
    * instance of the class, one must get an instance of it by calling this method.
    *
    * Arguments:
    *   rDisp   - An instance of the Adafruit_ILI9341 class used for the display.
    *   rCanvas - Reference to the canvas used to display the keypad.
    *   xp    - Pin driving touch screen X+, any digital pin.
    *   yp    - Pin driving touch screen Y+, must be analog (An).
    *   xm    - Pin driving touch screen X-, must be analog (An).
    *   ym    - Pin driving touch screen Y-, any digital pin.
    *   paint - 'true' to paint KB when done filling canvas.
    *   vOfst - Optional vertical offset, in pixels, of keyboard.
    *
    * Returns:
    *   Always returns a reference to the single instance of this class.
    *
    * Note: This class only operates correctly when the display is in one of the
    *       landscape orientations (1 or 3).  However, we don't check orientation
    *       here.  It is up to the user to insure that the orientation is set
    *       correctly.
    ***************************************************************************/
    static TsKeypad &Instance(Adafruit_SPITFT &rDisp, GFXcanvas16 &rCanvas,
                     uint8_t xp, uint8_t yp, uint8_t xm, uint8_t ym,
                     bool paint = true, uint16_t vOfst = DFLT_V_OFST);


    /*******************************************************************************
    * ShowKeyboard()
    *
    * Displays the keyboard on the screen at the current vertical offset location.
    * Displays the keyboard based on the currently selected shift and special key
    * states.  This method is overloaded.  This version uses the currently active
    * layout.
    *******************************************************************************/
    void ShowKeyboard();


    /***************************************************************************
    * ShowKeyboard()
    *
    * Displays the keyboard on the screen at the current vertical offset location.
    * Displays the keyboard based on the received arguments.  This method is
    * overloaded.  This version allows the user to select the desired starting layout.
    *
    * Arguments:
    *   shift   - Set 'true' to force shift, 'false' otherwise.
    *   special - Set 'true' to force special keys, 'false' to select letters.
    *
    * The following shows which layout will be selected based on arguments:
    *
    *   shift   special   layout
    *   ------------------------
    *    true    false    capital letters
    *    false   false    lower case letters
    *    true    true     symbols
    *    false   true     numbers
    ***************************************************************************/
    void ShowKeyboard(bool shift, bool special);


    /***************************************************************************
    * ClearKbCanvas()
    *
    * Clears the canvas by filling it with the background color.  Does not write
    * the cleared canvas to the display.
    ***************************************************************************/
    void ClearKbCanvas();


    /***************************************************************************
    * GetKeyPress()
    *
    * Maps any touch screen press to a keyboard key value, and if valid, returns
    * the resulting ASCII key value.
    *
    * Returns:
    *   Returns the ASCII key value if a touch is detected and it correspons to a
    *   valid key location.  Returns '\0' otherwise.
    ***************************************************************************/
    char GetKeyPress();


    /***************************************************************************
    * GetTouch()
    *
    * Returns an indication of whether the screen is currently being touched.  If
    * so, the coordinates and pressure of the touch is also returned.
    *
    * Arguments:
    *   rTp - A reference to the TSPoint structure which will return coordinates and
    *         pressure data if a touch is detected.
    *
    * Returns:
    *   Returns 'true' if a touch is detected, and the coordinates and pressure of
    *   the touch.  Returns 'false' otherwise, in which case the contents of 'p'
    *   are unmodified.
    ***************************************************************************/
    bool GetTouch(TSPoint &rTp);


    /***************************************************************************
    * WaitKeyRelease()
    *
    * Polls, waiting for all keys to be released.  Will exit after a user specified
    * timeout.  This can be used to affect a simple automatic key repeat.
    *
    * Arguments:
    *   timeoutMs - The maximum time in milliseconds to wait for the key release.
    *
    * Returns:
    *   Returns 'true' if the key is still pressed, or 'false' otherwise.
    ***************************************************************************/
    bool WaitKeyRelease(uint32_t timeoutMs);


    /***************************************************************************
    * WaitKeyRelease()
    *
    * Polls, waiting for all keys to be released.  Will exit after the current
    * timeout value specified by m_KeyRepeatDelayMs.
    a user specified
    * timeout.  This can be used to affect a simple automatic key repeat.
    *
    * Returns:
    *   Returns 'true' if the key is still pressed, or 'false' otherwise.
    ***************************************************************************/
    bool WaitKeyRelease();


    /***************************************************************************
    * IsWithin()
    *
    * Helper function to determine if a value is within a specified range.
    *
    * Arguments:
    *   x - Value to check against range specified by 'a' and 'b'.
    *   a - Range lower limit (inclusive).
    *   b - Range upper limit (inclusive).
    ***************************************************************************/
    bool IsWithin(int16_t x, int16_t a, int16_t b) const { return ((x >= a) && (x <= b)); }


    /***************************************************************************
    * Getters and setters
    ***************************************************************************/
    bool GetShift()           const { return m_Shift; }
    bool GetSpecial()         const { return m_Special; }
    rgb16_t GetBgColor()      const { return m_BgColor; }
    rgb16_t GetOutlineColor() const { return m_OutlineColor; }
    rgb16_t GetKeyColor()     const { return m_KeyColor; }
    rgb16_t GetTextColor()    const { return m_TextColor; }
    rgb16_t GetShadowtColor() const { return m_ShadowColor; }
    rgb16_t GetShiftColor()   const { return m_ShiftColor; }
    void GetButtonColors(rgb16_t &bg, rgb16_t &outln, rgb16_t &key, rgb16_t &txt,
                         rgb16_t &shdw, rgb16_t &shft) const ;
    uint32_t GetKeyReleaseDelayMs() const { return m_KeyRepeatDelayMs; }
    int16_t  GetVerticalOffset()    const { return m_VerticalOffset; }
    uint16_t GetKbHeight()          const { return KB_HEIGHT; }

    void SetBgColor(rgb16_t c)      { m_BgColor = c; }
    void SetOutlineColor(rgb16_t c) { m_OutlineColor = c; }
    void SetKeyColor(rgb16_t c)     { m_KeyColor = c; }
    void SetTextColor(rgb16_t c)    { m_TextColor = c; }
    void SetShadowColor(rgb16_t c)  { m_ShadowColor = c; }
    void SetShiftColor(rgb16_t c)   { m_ShiftColor = c; }
    void SetButtonColors(rgb16_t bg, rgb16_t outln, rgb16_t key, rgb16_t txt,
                         rgb16_t shdw, rgb16_t shft);
    void SetKeyReleaseDelayMs(uint32_t delay);
    void SetVerticalOffset(int16_t v);

private:
    // Unimplemented methods.
    TsKeypad(TsKeypad &r);
    TsKeypad &operator=(TsKeypad &r);

    // Private constructor and destructor for singleton.
    TsKeypad(Adafruit_SPITFT &disp,  GFXcanvas16 &canvas, uint8_t xp, uint8_t yp,
             uint8_t xm, uint8_t ym, bool paint = true, uint16_t vOfst = DFLT_V_OFST);
    ~TsKeypad() { }

    // This is calibration data for the raw touch data to the screen coordinates
    static const int16_t  TS_MINX = 120;        // Touch X minimum analog value.
    static const int16_t  TS_MINY = 890;        // Touch Y minimum analog value.
    static const int16_t  TS_MAXX = 920;        // Touch X maximum analog value.
    static const int16_t  TS_MAXY = 170;        // Touch Y maximum analog value.
    static const int16_t  MIN_PRESSURE = 7;     // Touch Z minimum analog value.
    static const int16_t  MAX_PRESSURE = 1200;  // Touch Z maximum analog value.

    static const uint16_t HALF_KEY_WIDTH = 15;  // Half of a key width (pixels).
    static const uint16_t HORIZONTAL_PAD = 10;  // Number X pixels between keys.
    static const uint16_t VERTICAL_PAD = 5;     // Number Y pixels between keys.
    static const uint16_t KEY_WIDTH = HALF_KEY_WIDTH * 2;
                                                // Width of a key (pixels).
    static const uint16_t KEY_HEIGHT = KEY_WIDTH;
                                                // Height of a key (pixels).
    static const uint16_t BUTTON_WIDTH = KEY_WIDTH - HORIZONTAL_PAD;
                                                // Width of a key excluding pad.
    static const uint16_t BUTTON_HEIGHT = KEY_HEIGHT - VERTICAL_PAD;
                                                // Height of a key excluding pad.
    static const int16_t  KB_HEIGHT = 4 * KEY_WIDTH;
                                                // Height of the full keyboard.

    static const char    *BS_LABEL;             // Label string for backspace key.
    static const char    *SHIFT_LABEL;          // Label string for shift key.

    static const uint8_t  KB_TEXT_SIZE = 2;     // Normal key text size.
    static const uint16_t SCREEN_WIDTH = 320;   // Width of screen in pixels.
    static const uint16_t SCREEN_HEIGHT = 240;  // Height of screen in pixels.
    static const uint16_t DFLT_V_OFST = SCREEN_HEIGHT - KB_HEIGHT;
                                                // Default vertical offset of
                                                // keyboard in pixels.

    // For better pressure precision, we need to know the resistance
    // between X+ and X- Use any multimeter to read it
    // For the one we're using, its 300 ohms across the X plate
    static const uint16_t  RESISTANCE = 300;

    // Convenience structure containing key placement and size data.
    struct KeyPosAndSize
    {
        int16_t  x;     // X position of key in pixels.
        int16_t  y;     // Y position of key in pixels.
        uint16_t w;     // Width of key in pixels.
        uint16_t h;     // Height of key in pixels.
    };  // End KeyPosAndSize.

    // Private methods.
    uint16_t CenterStringX(const char *pStr, int16_t keyX, uint16_t width,
                           uint16_t textSize = KB_TEXT_SIZE);
    uint16_t CenterStringY(const char *pStr, int16_t keyY, uint16_t height,
                           uint16_t textSize = KB_TEXT_SIZE);
    void     DrawButton(int16_t x, int16_t y, int16_t w, int16_t h);
    void     DrawButton(const KeyPosAndSize &rKps);
    bool     CheckButtonTouch(TSPoint &rTp, int16_t x, int16_t y, int16_t w, int16_t h);
    bool     CheckButtonTouch(int16_t x, int16_t y, int16_t w, int16_t h);
    bool     CheckButtonTouch(TSPoint &rTp, const KeyPosAndSize &rKps);

    bool            m_Paint;            // 'true' to paint the KB to the display.
    bool            m_Shift;            // True if keyboard is shifted.
    bool            m_Special;          // True if special keyboard is selected.
    uint32_t        m_KeyRepeatDelayMs; // Milliseconds to hold key before repeat.
    rgb16_t         m_BgColor;          // Background color.
    rgb16_t         m_OutlineColor;     // Key outline color.
    rgb16_t         m_KeyColor;         // Key color.
    rgb16_t         m_TextColor;        // Key text color.
    rgb16_t         m_ShadowColor;      // Key shadow color.
    rgb16_t         m_ShiftColor;       // Shift key label color when shifted.
    int16_t         m_VerticalOffset;   // Vertical offset (pixels) for kbd display.
    GFXcanvas16     &m_Canvas;          // Canvas used to display the keyboard.
    Adafruit_SPITFT &m_Display;         // The display object.

    // Structure to specify a row of keys to display.
    struct KbLayout
    {
        int16_t  m_Offset;          // Row start X offset.
        uint16_t m_NumKeys;         // Number of keys in the row.
        const char *m_KeyLabels;    // String of key values.  1 char per key.
    }; // End KbLayout.

    const KbLayout *m_Layout;       // Currently active keyboard layout.

    // Letter keys layout (used for both lower and upper case letters).
    const KbLayout LetterKeys[3] =
    {
        {0 * HALF_KEY_WIDTH, 10, "QWERTYUIOP"},
        {1 * HALF_KEY_WIDTH,  9, "ASDFGHJKL"},
        {3 * HALF_KEY_WIDTH,  7, "ZXCVBNM"}
    }; // End LetterKeys[].

    // Number keys layout.
    const KbLayout NumberKeys[3] =
    {
        {0 * HALF_KEY_WIDTH, 10, "1234567890"},
        {0 * HALF_KEY_WIDTH, 10, "-/:;()$&@\""},
        {5 * HALF_KEY_WIDTH,  5, ".,?!'"}
    }; // End NumberKeys[].

    // Special keys layout.
    const KbLayout SpecialKeys[3] =
    {
        {0 * HALF_KEY_WIDTH, 10, "[]{}#%^*+="},
        {4 * HALF_KEY_WIDTH,  6, "_\\|~<>"},
        {5 * HALF_KEY_WIDTH,  5, ".,?!\'"}
    }; // End  SpecialKeys[].

    // Structs for position and size of keys that are handled specially.
    const KeyPosAndSize m_SpecialKey   = {HALF_KEY_WIDTH, 3 * KEY_HEIGHT, 65,  BUTTON_HEIGHT};
    const KeyPosAndSize m_BackspaceKey = {270,            2 * KEY_HEIGHT, 35,  BUTTON_HEIGHT};
    const KeyPosAndSize m_ReturnKey    = {240,            3 * KEY_HEIGHT, 65,  BUTTON_HEIGHT};
    const KeyPosAndSize m_SpacebarKey  = {90,             3 * KEY_HEIGHT, 140, BUTTON_HEIGHT};
    const KeyPosAndSize m_ShiftKey     = {HALF_KEY_WIDTH, 2 * KEY_HEIGHT, 35,  BUTTON_HEIGHT};

};  // End TsKeypad class.


#endif // TSKEYPAD_H.
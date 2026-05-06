/*******************************************************************************
* TsKeypad.cpp
*
* Implements the TsKeypad class methods.  This class implements a soft keypad
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

#include "TsKeypad.h"   // Our class declaration.

// Label strings for special keys.  Use special chars from standard font.
const char *TsKeypad::BS_LABEL    = "\x11";     // Backspace.
const char *TsKeypad::SHIFT_LABEL = "\x18";     // Shift.


/*******************************************************************************
* Instance()
*
* This class is implemented as a singleton.  In order to create the single instance
* of the class, one must get an instance of it by calling this method.
*
* Arguments:
*   rDisp   - An instance of the Adafruit_ILI9341 class used for the display.
*   rCanvas - Reference to the canvas used to display the keypad.
*   xp      - Pin driving touch screen X+, any digital pin.
*   yp      - Pin driving touch screen Y+, must be analog (An).
*   xm      - Pin driving touch screen X-, must be analog (An).
*   ym      - Pin driving touch screen Y-, any digital pin.
*   paint   - Set 'true' to paint the canvas, 'false' otherwise.
*   vOfst   - Optional vertical offset, in pixels, of keyboard.
*
* Returns:
*   Always returns a reference to the single instance of this class.
*
* Note: This class only operates correctly when the display is in one of the
*       landscape orientations (1 or 3).  However, we don't check orientation
*       here.  It is up to the user to insure that the orientation is set
*       correctly.
*******************************************************************************/
TsKeypad &TsKeypad::Instance(Adafruit_SPITFT &rDisp, GFXcanvas16 &rCanvas,
                             uint8_t xp, uint8_t yp, uint8_t xm, uint8_t ym,
                             bool paint, uint16_t vOfst)
{
    // Static so that only one instance is ever created.
    static TsKeypad kp = TsKeypad(rDisp, rCanvas, xp, yp, xm, ym, paint, vOfst);
    return kp;
} // End Instance().


/*******************************************************************************
* Constructor (private)
*
* Arguments:
*   rDisp   - An instance of the Adafruit_ILI9341 class used for the display.
*   rCanvas - Reference to the canvas used to display the keypad.
*   xp      - Pin driving touch screen X+, any digital pin.
*   yp      - Pin driving touch screen Y+, must be analog (An).
*   xm      - Pin driving touch screen X-, must be analog (An).
*   ym      - Pin driving touch screen Y-, any digital pin.
*   paint   - Set 'true' to paint the canvas, 'false' otherwise.
*   vOfst   - Optional vertical offset, in pixels, of keyboard.
*
*******************************************************************************/
TsKeypad::TsKeypad(Adafruit_SPITFT &rDisp, GFXcanvas16 &rCanvas, uint8_t xp,
                   uint8_t yp, uint8_t xm, uint8_t ym, bool paint, uint16_t vOfst) :
    TouchScreen(xp, yp, xm, ym, RESISTANCE), m_Paint(paint), m_Shift(true),
    m_Special(false), m_KeyRepeatDelayMs(500), m_BgColor(ILI9341_BLUE),
    m_OutlineColor(ILI9341_WHITE), m_KeyColor(ILI9341_RED), m_TextColor(ILI9341_WHITE),
    m_ShadowColor(ILI9341_DARKGREY), m_ShiftColor(ILI9341_GREEN),
    m_VerticalOffset(vOfst), m_Canvas(rCanvas),
    m_Display(rDisp), m_Layout(LetterKeys)
{
} // End Constructor.


/*******************************************************************************
* SetVerticalOffset()
*
* Sets/changes the vertical offset of the keyboard.
*
* Arguments:
*   ofst - Vertical offset, in pixels, at which the keyboard will be displayed.
*******************************************************************************/
void TsKeypad::SetVerticalOffset(int16_t ofst)
{
    // Make sure we can fit our keyboard on the screen.
    ofst = constrain(ofst, 0, m_Display.height() - KB_HEIGHT);

    // If the offset has changed, erase the old keyboard and display it in the
    // new position.
    if (ofst != m_VerticalOffset)
    {
        // Erase the old kb before drawing the new one.
        ClearKbCanvas();
        m_VerticalOffset = ofst;
        ShowKeyboard();
    }
} // End SetVerticalOffset().


/*******************************************************************************
* CenterStringX()
*
* Calculate the horizontal position to start placing a string given the string,
* the start X position of the key, the width of the key, and the text size.
*
* Arguments:
*   pStr     - The string to be displayed.
*   keyX     - The X position of the key (in pixels).
*   width    - The width, in pixels, of the key.
*   textSize - The size of the font as set in setTextSize().
*******************************************************************************/
uint16_t TsKeypad::CenterStringX(const char *pStr, int16_t keyX, uint16_t width,
                                 uint16_t textSize)
{
    // Determine the width of the string.
    int16_t  x1;    // Unused returned X position of the string.
    int16_t  y1;    // Unused returned Y position of the string.
    uint16_t w;     // Returned width of the string, in pixels.
    uint16_t h;     // Unused returned height of the string, in pixels.
     m_Canvas.getTextBounds(pStr, 0, 0, &x1, &y1, &w, &h);

     // We account for the extra space at the end of the final character
     // by reducing the width by the size of the font.
     w -= textSize;

     // Return the starting X coordinate of the text.
     return keyX + (width - w) / 2;
} // End CenterStringX().


/*******************************************************************************
* CenterStringY()
*
* Calculate the vertical position to start placing a string given the string,
* the start Y position of the key, the height of the key, and the text size.
*
* Arguments:
*   pStr     - The string to be displayed.
*   keyY     - The Y position of the key (in pixels).
*   height   - The height, in pixels, of the key.
*   textSize - The size of the font as set in setTextSize().
*******************************************************************************/
uint16_t TsKeypad::CenterStringY(const char *pStr, int16_t keyY, uint16_t height,
                                 uint16_t textSize)
{
    // Determine the height of the string.
    int16_t  x1;    // Unused returned X position of the string.
    int16_t  y1;    // Unused returned Y position of the string.
    uint16_t w;     // Unused teturned width of the string, in pixels.
    uint16_t h;     // Returned height of the string, in pixels.
     m_Canvas.getTextBounds(pStr, 0, 0, &x1, &y1, &w, &h);

     // We account for the extra space at the bottom of the string
     // by reducing the height by the size of the font.
     h -= textSize;

     // Return the starting Y coordinate of the text.
     return keyY + (height - h) / 2;
} // End CenterStringY().


/*******************************************************************************
* ClearKbCanvas()
*
* Clears the canvas by filling it with the background color.  Does not write
* the cleared canvas to the display.
*******************************************************************************/
void TsKeypad::ClearKbCanvas()
{
    m_Canvas.fillRect(0, m_VerticalOffset, m_Canvas.width(), KB_HEIGHT, m_BgColor);
} // End ClearKbCanvas().


/*******************************************************************************
* ShowKeyboard()
*
* Displays the keyboard on the screen at the current vertical offset location.
* Displays the keyboard based on the currently selected shift and special key
* states.  This method is overloaded.  This version uses the currently active
* layout.
*******************************************************************************/
void TsKeypad::ShowKeyboard()
{
    // Start with the currently active key layout.
    const KbLayout *pKbd = m_Layout;

    // Initialize text and color settings.
    m_Canvas.setFont();
    m_Canvas.setTextSize(KB_TEXT_SIZE);
    m_Canvas.setTextColor(m_TextColor, m_KeyColor);

    // Clear the canvas to start fresh.
    ClearKbCanvas();

    // Step through each row of keys.  All layouts use 4 rows of keys.
    for (size_t y = 0; y < 3; y++, pKbd++)
    {
        // Step through each key in the current row.
        const char *pKey = pKbd->m_KeyLabels;
        for (uint16_t x = 0; *pKey != '\0'; x++, pKey++)
        {
            // Use lower case keys if we're not shifted and it's a letter.
            char c = *pKey;
            if ((!m_Shift && isupper(c)))
            {
                c = tolower(c);
            }

            // Draw the button on the screen.
            int16_t keyX = HALF_KEY_WIDTH + (KEY_WIDTH * x) + pKbd->m_Offset;
            int16_t keyY = KEY_WIDTH * y;
            DrawButton(keyX, keyY, BUTTON_WIDTH, BUTTON_HEIGHT);
            m_Canvas.setCursor(keyX + 5, m_VerticalOffset + keyY + 5);
            m_Canvas.print(c);
        }
    }

    // Now handle the special keys (shift, special, space, backspace, and return).
    const char *label =  "'\0'";

    // Beware!  We are changing the text size here.  It will stay in effect
    // for the Special Characters, Return, and Spacebar buttons.  Be sure to
    // restore the KB_TEXT_SIZE before displaying the remaining buttons.
    m_Canvas.setTextSize(1);
    {
        // Special characters key.
        DrawButton(m_SpecialKey);
        label = ((m_Layout == LetterKeys) ? "123" : "ABC");
        m_Canvas.setCursor(CenterStringX(label, m_SpecialKey.x, m_SpecialKey.w, 1),
                           m_VerticalOffset +
                              CenterStringY(label, m_SpecialKey.y, m_SpecialKey.h, 1));
        m_Canvas.print(label);

        // Return key.
        DrawButton(m_ReturnKey);
        label = "return";
        m_Canvas.setCursor(CenterStringX(label, m_ReturnKey.x, m_ReturnKey.w, 1),
                           m_VerticalOffset +
                              CenterStringY(label, m_ReturnKey.y, m_ReturnKey.h, 1));
        m_Canvas.print(label);

        // Spacebar key
        DrawButton(m_SpacebarKey);
        label = "space";
        m_Canvas.setCursor(CenterStringX(label, m_SpacebarKey.x, m_SpacebarKey.w, 1),
                           m_VerticalOffset +
                              CenterStringY(label, m_SpacebarKey.y, m_SpacebarKey.h, 1));
        m_Canvas.print(label);
    }
    m_Canvas.setTextSize(2);

    // BackSpace key.
    DrawButton(m_BackspaceKey);
    label = BS_LABEL;
    m_Canvas.setCursor(CenterStringX(BS_LABEL, m_BackspaceKey.x, m_BackspaceKey.w),
                       m_VerticalOffset +
                          CenterStringY(BS_LABEL, m_BackspaceKey.y, m_BackspaceKey.h));
    m_Canvas.print(label);

    // Shift key.
    // This key is handled specially since we want to change the label color
    // to indicate whether or not shift is active.
    DrawButton(m_ShiftKey);
    label = SHIFT_LABEL;
    m_Canvas.setCursor(CenterStringX(label, m_ShiftKey.x, m_ShiftKey.w),
                       m_VerticalOffset +
                          CenterStringY(label, m_ShiftKey.y, m_ShiftKey.h));
    if (m_Shift)
    {
        // We're shifted, so use the shift color.
        m_Canvas.setTextColor(m_ShiftColor, m_KeyColor);
    }
    else
    {
        // We're not shifted, so use the normal text color.
        m_Canvas.setTextColor(m_TextColor, m_KeyColor);
    }
    m_Canvas.print(label);

    // Finally, display all the keys at once if allowed.
    if (m_Paint)
    {
        m_Display.drawRGBBitmap(0, m_VerticalOffset, m_Canvas.getBuffer() +
                                m_Canvas.width() * m_VerticalOffset,
                                m_Canvas.width(), m_Canvas.height());
    }
} // End ShowKeyboard().


/*******************************************************************************
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
*******************************************************************************/
void TsKeypad::ShowKeyboard(bool shift, bool special)
{
    // Save the (possibly) new shift and special values.
    m_Shift   = shift;
    m_Special = special;

    // Determine which layout to use.
    if (m_Special)
    {
        m_Layout = m_Shift ? SpecialKeys : NumberKeys;
    }
    else
    {
        m_Layout = LetterKeys;
    }

    // Display the keyboard.
    ShowKeyboard();
} // End ShowKeyboard(bool, bool).


/*******************************************************************************
* DrawButton()
*
* Draws a button of the given size, without text, at the given location.
*
* Arguments:
*   x - X pixel coordinate of the button.
*   y - Y pixel coordinate of the button.
*   w - Width, in pixels, of the button.
*   h - Height, in pixels, of the button.
*******************************************************************************/
void TsKeypad::DrawButton(int16_t x, int16_t y, int16_t w, int16_t h)
{
    // Show button shading if its color differs from the background color.
    if (m_ShadowColor != m_BgColor)
    {
        m_Canvas.fillRoundRect(x - 3, m_VerticalOffset + y + 3, w, h, 3,
                               m_ShadowColor);
    }

    // Show the button outline.
    m_Canvas.drawRoundRect(x, m_VerticalOffset + y, w, h, 3, m_OutlineColor);

    // Show the button inner ccolor.
    m_Canvas.fillRoundRect(x + 1, m_VerticalOffset + y + 1, w - 1 * 2, h - 1 * 2,
                           3, m_KeyColor);
} // End DrawButton(int16_t, int16_t, int16_t, int16_t).


/*******************************************************************************
* DrawButton()
*
* Draws a button of the given size, without text, at the given location.
*
* Arguments:
*   k - Reference to a KeyPosAndSize structure that contains the button's
*       position and size.
*******************************************************************************/
void TsKeypad::DrawButton(const KeyPosAndSize &rKps)
{
    DrawButton(rKps.x, rKps.y, rKps.w, rKps.h);
} // End DrawButton(const KeyPosAndSize).


/*******************************************************************************
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
*******************************************************************************/
bool TsKeypad::WaitKeyRelease(uint32_t timeoutMs)
{
    uint32_t startTime = millis();
    m_KeyRepeatDelayMs = timeoutMs;
    while ((pressure() < MAX_PRESSURE) && (millis() - startTime < timeoutMs))
    {
        delay(10);
    }
    // Return state of key - true if key still pressed, false otherwise.
    return (pressure() < MAX_PRESSURE);
} // End WaitKeyRelease(uint32_t).


/*******************************************************************************
* WaitKeyRelease()
*
* Polls, waiting for all keys to be released.  Will exit after the current
* timeout value specified by m_KeyRepeatDelayMs.  This can be used to affect a
* simple automatic key repeat.
*
* Returns:
*   Returns 'true' if the key is still pressed, or 'false' otherwise.
*******************************************************************************/
bool TsKeypad::WaitKeyRelease()
{
    return WaitKeyRelease(m_KeyRepeatDelayMs);
} // End WaitKeyRelease().


/*******************************************************************************
* SetKeyReleaseDelayMs()
*
* Sets the key repeat value used by WaitKeyRelease().
*
* Arguments:
*   delayMs - The new delay value to be assigned to m_KeyRepeatDelayMs,
*             (in milliseconds).
*******************************************************************************/
void TsKeypad::SetKeyReleaseDelayMs(uint32_t delayMs)
{
    m_KeyRepeatDelayMs = constrain(delayMs, 100, 999999);
} // End SetKeyReleaseDelayMs().


/*******************************************************************************
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
*******************************************************************************/
bool TsKeypad::GetTouch(TSPoint &rTp)
{
    // Assume no touch.
    bool isTouched = false;

    // Get the current touch value.
    rTp = getPoint();

    // If pressed, return the remapped coordinates.
    // Note: Touch values are only returned if the display is in landscape mode
    //       (1 or 3).  Any other orientation will return a 'false' value, even
    //       if a press was detected.
    if ((rTp.z >= MIN_PRESSURE) && (rTp.z <= MAX_PRESSURE))
    {
        uint16_t temp = rTp.x;
        rTp.x = rTp.y;
        rTp.y = temp;

        if (m_Display.getRotation() == 1)
        {
            // Scale from ~0->1000 to tft.width using the calibration #'s.
            rTp.x = map(rTp.x, TS_MINX, TS_MAXX, 0, m_Display.width());
            rTp.y = map(rTp.y, TS_MINY, TS_MAXY, 0, m_Display.height());
            isTouched = true;
        }
        else if (m_Display.getRotation() == 3)
        {
            rTp.x = map(rTp.x, TS_MINX, TS_MAXX, m_Display.width(), 0);
            rTp.y = map(rTp.y, TS_MINY, TS_MAXY, m_Display.height(), 0);
            isTouched = true;
        }
    }
    else if (rTp.z > MAX_PRESSURE)
    {
        Serial.printf("Max Pressure Exceeded: %d\n", rTp.z);
    }
    return isTouched;
} // End GetTouch().


/*******************************************************************************
* GetKeyPress()
*
* Maps any touch screen press to a keyboard key value, and if valid, returns the
* resulting ASCII key value.
*
* Returns:
*   Returns the ASCII key value if a touch is detected and it correspons to a
*   valid key location.  Returns '\0' otherwise.
*******************************************************************************/
char TsKeypad::GetKeyPress()
{
    bool layoutChanged   = false;       // Layout hasn't changed.
    const KbLayout *pRow = m_Layout;    // Point to the currently selected layout.
    char keyVal = '\0';                 // Assume no valid key is pressed.

    // See if we got a touch andd it's within the keyboard boundaries.
    TSPoint p;
    if (GetTouch(p) && (p.y >= m_VerticalOffset) &&
         (p.y <= (m_VerticalOffset + KB_HEIGHT)))
    {
        // Check the Shift key.
        if (CheckButtonTouch(p, m_ShiftKey))
        {
            m_Shift = !m_Shift;
            layoutChanged = true;
        }

        // Check the Special Characters key.
        else if (CheckButtonTouch(p, m_SpecialKey))
        {
            m_Special = !m_Special;
            layoutChanged = true;
        }

        // Check teh Spacebar.
        else if (CheckButtonTouch(p, m_SpacebarKey))
        {
            keyVal = ' ';
        }

        // Check the Backspace key.
        else if (CheckButtonTouch(p, m_BackspaceKey))
        {
            keyVal = '\b';
        }

        // Check the Return key.
        else if (CheckButtonTouch(p, m_ReturnKey))
        {
            keyVal = '\n';
        }

        // Check normal keys.
        else
        {
            // Determine which row of the layout is selected.
            uint16_t row = (p.y - m_VerticalOffset) / KEY_WIDTH;;

            // Limit the row value.  This shouldn't be necessary, but just in case...
            row = min(row, 2);

            // Point to the row that is selected.
            pRow += row;

            // Is the touch within the bounds of this row?
            if ((p.x >= HALF_KEY_WIDTH + pRow->m_Offset) &&
                (p.x < HALF_KEY_WIDTH + pRow->m_Offset + pRow->m_NumKeys * KEY_WIDTH - HORIZONTAL_PAD))
            {
                // Within horizontal bounds, calculate the column.
                uint16_t col = (p.x - (HALF_KEY_WIDTH + pRow->m_Offset - HORIZONTAL_PAD)) / KEY_WIDTH;
                // Limit the column.  This shouldn't be necessary, but just in case...
                col = min(col, pRow->m_NumKeys - 1);

                // Get the selected character ASCII code and convert case if needed.
                char c = pRow->m_KeyLabels[col];
                keyVal = (m_Shift && isupper(c)) ? c : tolower(c);
            }
        }

        // If the shift or special key was selected, then we need to update the
        // layout.
        if (layoutChanged)
        {
            ClearKbCanvas();
            if (m_Special)
            {
                // Special layout is needed.  See which layout based on shift key.
                if (m_Shift)
                {
                    m_Layout = SpecialKeys;
                }
                else
                {
                    m_Layout = NumberKeys;
                }
            }
            else
            {
                // Not special.  Use letter layout.
                m_Layout = LetterKeys;
            }

            // Change shift key color based on shift state.
            if (m_Shift)
            {
                m_Canvas.setTextColor(m_ShiftColor, m_KeyColor);
            }
            else
            {
                m_Canvas.setTextColor(m_TextColor, m_KeyColor);
            }
            m_Canvas.setFont();
            m_Canvas.setCursor(CenterStringX(SHIFT_LABEL, m_ShiftKey.x, m_ShiftKey.w),
                               m_VerticalOffset +
                                 CenterStringY(SHIFT_LABEL, m_ShiftKey.y, m_ShiftKey.h));
            m_Canvas.print(SHIFT_LABEL);

            ShowKeyboard();
        }

        // Wait for the key to be released before returning.
        WaitKeyRelease();
    }

    // Return the detected key value or NULL if no or non-ASCII key.
    return keyVal;
} // End GetKeyPress().


/*******************************************************************************
* CheckButtonTouch()
*
* Overridden method that checks whether a touch value is within specific key
* bounds.
*
* Arguments:
*   The three overridden methods use the following arguments:
*       CheckButtonTouch(TSPoint &p, int16_t x, int16_t y, int16_t w, int16_t h)
*           rTp - Reference to TSPoint struct containing touch coordinates.
*           x   - X coordinate of the key being tested.
*           y   - Y coordinate of the key being tested.
*           w   - Width of the key being tested.
*           h   - Height of the key being tested.
*
*       CheckButtonTouch(int16_t x, int16_t y, int16_t w, int16_t h)
*       This version calls getPoint() to get the current touch point, if any.
*           x - X coordinate of the key being tested.
*           y - Y coordinate of the key being tested.
*           w - Width of the key being tested.
*           h - Height of the key being tested.
*
*       CheckButtonTouch(TSPoint &p, const KeyPosAndSize &k)
*           rTp  - Reference to TSPoint struct containing touch coordinates.
*           rKps - Reference to KeyPosAndSize struct containing key information.
*
* Returns:
*   Returns 'true' if the touch is within the bounds of the specified button.
*   Returns 'false' otherwise.
*******************************************************************************/
bool TsKeypad::CheckButtonTouch(TSPoint &rTp, int16_t x, int16_t y,
                                int16_t w, int16_t h)
{
    // Assume no key press.
    bool keyPressed = false;

    // If touch is detected, determine if touch is within key bounds.
    if ((rTp.z >= MIN_PRESSURE) && (rTp.z <= MAX_PRESSURE))
    {
        keyPressed = (IsWithin(rTp.x, x, x + w) & IsWithin(rTp.y, y, y + h));
    }

    // Return perss status.
    return keyPressed;
} // End CheckButtonTouch(TSPoint &, int16_t, int16_t, int16_t, int16_t).


bool TsKeypad::CheckButtonTouch(int16_t x, int16_t y, int16_t w, int16_t h)
{
    // Retrieve a point, then check if it is within key bounds.
    TSPoint p = getPoint();
    GetTouch(p);
    return CheckButtonTouch(p, x, y, w, h);
} // End CheckButtonTouch(int16_t, int16_t, int16_t, int16_t, ).


bool TsKeypad::CheckButtonTouch(TSPoint &rTp, const KeyPosAndSize &rKps)
{
    return CheckButtonTouch(rTp, rKps.x, rKps.y + m_VerticalOffset, rKps.w, rKps.h);
} // End CheckButtonTouch(TSPoint &, const KeyPosAndSize &).


/*******************************************************************************
* GetButtonColors()
*
* Returns the colors used by the keyboard.
*
* Arguments:
*   rBg    - Reference to variable where background color will be returned.
*   outln - Reference to variable where key outline color will be returned.
*   rKey   - Reference to variable where key color will be returned.
*   rTxt   - Reference to variable where key text color will be returned.
*   rShdw  - Reference to variable where key shadow color will be returned.
*   rShft  - Reference to variable where shifted shift key color will be returned.
*
* Returns:
*    Returns currently selected colors in the associated variables.
*******************************************************************************/
void TsKeypad::GetButtonColors(rgb16_t &rBg, rgb16_t &rOutln, rgb16_t &rKey,
               rgb16_t &rTxt, rgb16_t &rShdw, rgb16_t &rShft) const
{
    rBg    = m_BgColor;
    rOutln = m_OutlineColor;
    rKey   = m_KeyColor;
    rTxt   = m_TextColor;
    rShdw  = m_ShadowColor;
    rShft  = m_ShiftColor;
} // End GetButtonColors().


/*******************************************************************************
* SetButtonColors()
*
* Sets the colors used by the keyboard.
*
* Arguments:
*   rBg    - Reference to the new background color.
*   rOutln - Reference to the new key outline color.
*   rKey   - Reference to the new key color.
*   rTxt   - Reference to the new key text color.
*   rShdw  - Reference to the new key shadow color.
*   rShft  - Reference to the new shifted shift key color.

*******************************************************************************/
void TsKeypad::SetButtonColors(rgb16_t rBg, rgb16_t rOutln, rgb16_t rKey, rgb16_t rTxt,
               rgb16_t rShdw, rgb16_t rShft)
{
    m_BgColor      = rBg;
    m_OutlineColor = rOutln;
    m_KeyColor     = rKey;
    m_TextColor    = rTxt;
    m_ShadowColor  = rShdw;
    m_ShiftColor   = rShft;
} // End SetButtonColors().

/******************************************************************************
* GuiDriver.h
*
* Contains methods that implement a library of GUI items that may be displayed
* on the screens.
*
* This code was adapted from work by Erno Gilissen Belgium as it appeared on
* Instructables: https://www.instructables.com/Micro-GUI-on-Arduino-Mega-Uno/
* It has been heavily modified to run out of RAM instead of PROGMEM since this
* application uses the Raspberry Pi Pico 2W processor which has abundant memory
* and speed.  Several improvements have also been added.
*
* This file defines several global data objects and contains the GuiDriver
* class that handles all screen GUI interactions.
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

#if !defined (GUI_DRIVER_H)
#define GUI_DRIVER_H

#include <vector>           // For vector template.
#include "TsKeypad.h"       // For touch screen.

// Uncomment the following line to show touch boundaries for debugging and testing.
// #define TOUCH_HIGHLIGHT ILI9341_RED


// Forward declaration for the SCR_DATA struct.
struct __attribute__((__packed__)) SCR_DATA;

// Typedefs used to make interfacing to the vector containing SCR_DATA easier.
typedef std::vector<SCR_DATA> scr_vec_t;
typedef scr_vec_t::iterator scr_iter_t;


/*******************************************************************************
* When the user touches the display, the touch position is used to determine what
* must be changed on the screen (since this requires minimal calculations).
* If the object is bar or slider and must be re-drawn at a specific percentage,
* the correlating touch position gets calculated.
*******************************************************************************/
enum TOUCH_REFERENCE
{
    // BARs switchover position is taken from xTouch or yTouch coordinate.
    TOUCHPOS_REF,
    // Compute SLIDERVAR switchover position (can be used to preset a SLIDERBAR
    // when no touch involved).
    TOUCHPOS_CALC,
}; // End TOUCH_REFERENCE.


/*******************************************************************************
* SCR_OBJECT
*
* Specifies the different objects the GUI can handle. For space saving, consider
* removing or commenting out unused objects.
*
* Note:
* - LINE is slower than LINE_V or LINE_H, but can draw lines at an angle.
*******************************************************************************/
enum SCR_OBJECT         //Each screen can be built out of these 'objects'.
{
    BACKGROUND = 0,     // Only as 1st statement (clears screen).
    BUTTON_RECT,        // Button (Filled) Rectangle.
    BUTTON_RECT_RND,    // Button (Filled) Rectangle with Rounded Corners.
    BUTTON_ROUND,       // Round Button.
    CHECKBOX,           // Check Box.
    RADIO_BUTTON,       // Radio button.
    INPUT_DATA,         // Mark area for input. (ignored if 1st member in array).
    LABEL,              // Label w/o shape.
    LINE,               // Exception: sl=X0; sw=X1; st=Y0; sh=Y1.
    LINE_H,             // Uses only sl, sw and st (X, Width, Y).
    LINE_V,             // Uses only sl, st and sh (X, Y, Height).
    RECTANGLE,          // Hollow Rectangle.
    RECTANGLE_FILL,     // Filled Rectangle.
    BUTTON_SLH,         // Horizontal slider button.
    BUTTON_SLV,         // Vertical slider button.
    SLIDERBAR_H,        // Horizontal Slider Bar.
    SLIDERBAR_V,        // Vertical Slider bar..
    TIMEOUT             // Perform operation on timeout.
}; // End SCR_OBJECT.


/*******************************************************************************
* SCR_UPDATE
*
* If the screen built-up function is called, there are 3 options supported:
*                  |  Screen already on top      | Actual Screen Different
* SCR_DRAW         |        Do nothing           | Clear screen & draw requested screen.
* SCR_REDRAW,      |    Clear screen & re-draw   | Clear screen & draw requested screen.
* SCR_REDRAW_W_CLR |  Re-draw w/o clearing first | Clear screen & draw requested screen.
*******************************************************************************/
enum SCR_UPDATE
{
    SCR_DRAW = 0,
    SCR_REDRAW,
    SCR_REDRAW_W_CLR,
}; // End SCR_UPDATE.


/*******************************************************************************
* SCR_DATA
*
* This structure defines one particular item to (possibly) be displayed on the
* screen.  An array, or vector, of these objects completely defines a screen.
*
* The structure is compiled packed, so that every type is stored in the most
* dense format.
*******************************************************************************/
struct __attribute__((__packed__)) SCR_DATA
{
    SCR_OBJECT  m_Obj;     // Screen Object (uint16_t).
    uint16_t    m_TX0;     // X-ABS Touch Left (LABEL, LINE: keep 0).
    uint16_t    m_TX1;     // X-ABS Touch Right (LABEL, LINE: keep 0).
    uint16_t    m_TY0;     // Y-ABS Touch Top (LABEL, LINE: keep 0).
    uint16_t    m_TY1;     // Y-ABS Touch Bottom (LABEL, LINE: keep 0).
    uint16_t    m_OX0;     // X-ABS Object Left.
    uint16_t    m_OW;      // X-REL Object Width (BUTTON_ROUND: Dia; LINE: X-endpoint).
    uint16_t    m_OY0;     // Y-ABS Object Top.
    uint16_t    m_OH;      // X-REL Object Height (BUTTON_ROUND: N/A; LINE: Y-endpoint)).
    uint16_t    m_OE;      // Object Extra: - Rounding (BUTTON_RECT_RND)
                           //               - division (SLIDERBAR_?)
                           //               - 3D effect (BUTTON_RECT; 0 = none)
                           //               - Update every iteration (BACKGROUND)
    uint16_t    m_OC;      // Object Color.
    uint16_t    m_LX0;     // Label X Pos (0 = automatic).
    uint16_t    m_LY0;     // Label Y Pos (0 = automatic).
    const char* m_pTxt;    // Pointer to Text (len cannot exceed 31 chars).
    uint16_t    m_FS;      // Font Size (1 ... 4).
    uint16_t    m_LC;      // Label Text Color.
    void  (*m_pClick) (int16_t); // Callback function when the object is selected.
    int16_t     m_LoPct;   // SLIDERBAR_?: left / bottom value (= lowest).
    int16_t     m_HiPct;   // SLIDERBAR_?: right / top value (= highest).
    int32_t     m_Val;     // Current value for checkboxes, radio buttons, sliders
                           // and timers.
    const GFXfont *m_pFont; // Font to use for label.
    bool        m_Enable;  // 'true' to enable, 'false' to disable.
}; // End SCR_DATA.


/*******************************************************************************
* GuiDriver
*
* This class maintains the current screen.  It contains methods to manage the
* display and scanning of the screen.
*
*******************************************************************************/
class GuiDriver
{
public:
    /***************************************************************************
    * Constructor
    *
    * Arguments:
    *   rTft    - Reference to the TFT display being used.
    *   rCanvas - Reference to the canvas used with the tft display.
    *   rTs     - Reference to the touchscreen being used.
    ***************************************************************************/
    GuiDriver(Adafruit_ILI9341 &rTft, GFXcanvas16 &rCanvas, TsKeypad &rTs);


    /***************************************************************************
    * ButtonSLHPct()
    *
    * Displays the BUTTON_SLH object on the screen.
    *
    * Screen View:
    *  tX0, tY0              xt          oHiPct   - Object size determined by oW and oH.
    *        ----------------|------------        - Horizontal line located at tY0 + (oH/2).
    *    oLoPct                       tX1, tY1    - Range: tX0 (= oLoPct) to (tX1 - oW) (= oHiPct).
    *
    * Arguments:
    *   pObj     - Pointer to the BUTTON_SLH object. Must be a BUTTON_SLH object.
    *   pct      - Percentage Fill for the sliderbar (required to determine left
    *              or right slider if LoPct is negative and HiPct is positive).
    *   enXtouch - TOUCHPOS_REF uses actual xTouch position to determine where to
    *              relocate the button object (fastest, only possible by touch
    *              operation).
    *              TOUCHPOS_CALC (default): compute a virtual touch position
    *              (slower, can be used to put the object at a specific position).
    *
    * Note:
    *    If the BUTTON_SLH is not correctly configured, it may not be drawn on
    *    the screen.
    ***************************************************************************/
    void ButtonSLHPct(SCR_DATA* pObj, int16_t pct, TOUCH_REFERENCE enXtouch);


    /***************************************************************************
    * ButtonSLVPct()
    *
    * Displays the BUTTON_SLV object on the screen.
    *
    * Screen View:
    *    tX0, tY0    |     oHiPct   - Object size determined by oW and oH.
    *                |              - Vertical line located at tY0 - (oW/2).
    *               -+- yt          - Range: tY0 (= oHiPct) to (tY1 - oH) (= oLoPct).
    *                |
    *                |
    *      oLoPct    |      tX1, tY1
    *
    * Arguments:
    *   pObj     - Pointer to the BUTTON_SLV object.  Must be a BUTTON_SLV object.
    *   pct      - Percentage Fill for the sliderbar (required to determine up
    *              or down slider if LoPct is negative and HiPct is positive).
    *              Must always be provided as the % click on the bar
    *              (max range -128 to + 127).
    *   enXtouch - TOUCHPOS_REF uses actual xTouch position to determine where to
    *              relocate the button object (fastest, only possible by touch
    *              operation).
    *              TOUCHPOS_CALC (default): compute a virtual touch position
    *              (slower, can be used to put the object at a specific position).
    *
    * Note:
    *    If the BUTTON_SLV is not correctly configured, it may not be drawn on
    *    the screen.
    ***************************************************************************/
    void ButtonSLVPct(SCR_DATA* pObj, int16_t pct, TOUCH_REFERENCE enXtouch);


    /***************************************************************************
    * UpdateCheckbox()
    *
    * Displays the CHECKBOX object on the screen.
    *
    * Arguments:
    *   pSd - Pointer to the CHECKBOX object.  Must be a CHECKBOX object.
    ***************************************************************************/
    void UpdateCheckbox(SCR_DATA* pSd);


    /***************************************************************************
    * UpdateRadioButton()
    *
    * Displays the RADIO_BUTTON object on the screen.
    *
    * Arguments:
    *   pSd - Pointer to the RADIO_BUTTON object.  Must be a RADIO_BUTTON object.
    ***************************************************************************/
    void UpdateRadioButton(SCR_DATA* pSd);


    /***************************************************************************
    * SliderBarHPct()
    *
    * Displays the SLIDERBAR_H object on the screen.
    *
    * Screen View:
    *   tX0, tY0             xt          oHiPct  - Frame from (tX0, tY0) width (oW to right) height (oH down).
    *         +---------+----+------------+      - Touch area is from (tX0, tY0) to (tX1, tY1).
    *         |         ||||||            | oW   - Range is tX0 (= oLoPct) to tX1 (= oHiPct).
    *         +---------+----+------------+ oH   - If oLoPct is negative and oHiPct
    *     oLoPct       xPos0            tX1, tY1   is positive, a virtual zero xPos0
    *                                              is used to draw a bi-directional bar.
    *                                              Otherwise the bar fills left-right.
    *
    * Arguments:
    *    pObj       Must point to a SLIDERBAR_H object.
    *    pct        Percentage fill for the sliderbar (required to determine left
    *               or right slider if LoPct is negative and HiPct is positive).
    *               Must always be provided as the % click on the bar
    *               (max range -128 to + 127).
    *    enXtouch   If this is non-zero, xTouch is used as the reference instead of
    *               pct computed.
    *               - TOUCHPOS_REF Use actual xTouch position to determine where
    *                 to relocate the button object (fastest, only possible by
    *                 touch operation).
    *               - TOUCHPOS_CALC (default): compute a virtual touch position
    *                 (slower, can be used to put the object at a specific position).
    *
    * Sample code:
    *    SCR_DATA* p = &sd[?];         // ? points to an existing SLIDERBAR_H object.
    *    int16_t  pct = 0;
    *    sliderbar_h_fill_pct (p, 75); // Fills the sliderbar to 75%.
    ***************************************************************************/
    void SliderBarHPct(SCR_DATA *pObj, int16_t pct, TOUCH_REFERENCE enXtouch);


    /***************************************************************************
    * SliderBarVPct()
    *
    * Displays the SLIDERBAR_V object on the screen.
    *
    * Screen View:
    *   tX0, tY0     oW    - Frame from (tX0, tY0) width (oW to right) height (oH down).
    *         +---+        - Touch area is from (tX0, tY0) to (tX1, tY1).
    *         |   |        - Range is tY1 (= oLoPct) to tY0 (= oHiPct).
    *         |   |        - If oLoPct is negative and oHiPct is positive, a virtual
    *         +---+ yPos0    zero yPos0 is used to draw a bi-directional bar.
    *         |---|          Otherwise the bar fills bottom-top.
    *         |---|
    *         +---+ yt
    *         |   |
    *         |   |
    *         +---+ tX1, tY1  oH
    *
    * Arguments:
    *    pObj       Must point to a SLIDERBAR_V object.
    *    pct        Percentage Fill for the sliderbar (required to determine up
    *               or down slider if LoPct is negative and HiPct is positive).
    *               Must always be provided as the % click on the bar
    *               (max range -128 to + 127).
    *    enXtouch   If this is non-zero, xTouch is used as the reference instead of
    *               pct computed.
    *               - TOUCHPOS_REF Use actual xTouch position to determine where
    *                 to relocate the button object (fastest, only possible by
    *                 touch operation).
    *               - TOUCHPOS_CALC (default): compute a virtual touch position
    *                 (slower, can be used to put the object at a specific position).
    *
    * Sample code:
    *    SCR_DATA* p = &sd[?];            // ? points to an existing SLIDERBAR_V object.
    *    int16_t   pct = 0;
    *    sliderbar_h_fill_pct (b, 75, 1); // Fills the sliderbar to 75%.
    ***************************************************************************/
    void SliderBarVPct(SCR_DATA *pObj, int16_t pct, TOUCH_REFERENCE enYtouch);


    /***************************************************************************
    * DrawScreen()
    *
    * Draws a screen. The first screen after power on must be called in the setup()
    * Arduino function.  From such 'base'screen, all others can be 'launched'.
    *
    * Arguments:
    *    pNewScreen  Pointer to the new screen vector.
    *    drawMode    Normal draw (only write if screen has changed or forced
    *                with/without clearing it first).
    *
    * Returns:
    *    Returns 0 if successful, positive for warning, negative for error.
    ***************************************************************************/
    int16_t DrawScreen(scr_vec_t *pNewScreen, SCR_UPDATE drawMode = SCR_DRAW);


    /***************************************************************************
    * ScanScreen()
    *
    * Scan the touchscreen coordinates and lookup in the actual SCR_DATA vector
    * if the X, Y touch coordinates are valid in a specific object.  If a valid touch
    * area is found and a function pointer is found, it will call this function.
    *
    * Note: if 0,0 is not perfectly calibrated, the Touch return from map function may
    *       be negative (hex value 0xFFxx).  Easist to get the software correct is
    *       print the p.x and p.y values for the 4 corners after calibration and map
    *       accordingly.
    *
    * Note: For BUTTON_SLH, BUTTON_SLV, SLIDERBAR_H, SLIDERBAR_V
    *       additional screen changes are applied.
    *
    * Note: There are slight differences for every object as SLIDERBAR_H has the
    *       full range whereas BUTTON_SLV has only tX0 to (tX1 - tW).
    ***************************************************************************/
    void ScanScreen();


    uint16_t GetTouchIndex()    const { return m_TouchIndex; }
    uint16_t GetBgColor()       const { return m_BgColor; }
    scr_vec_t *GetScreenPtr()         { return m_pCurrentScreen; }

private:
    static const uint16_t CHECKBOX_WALLS = 3;   // Width of checkbox walls.


    /***************************************************************************
    * DrawScreenObjects()
    *
    * Draws all of the screen objects from the selected screen data.
    *
    * Arguments:
    *   pScr - Pointer to the vector containing the objects of the selected screen.
    ***************************************************************************/
    void DrawScreenObjects(scr_vec_t* pScr);


    Adafruit_ILI9341 &m_Tft;            // Display reference.
    GFXcanvas16      &m_Canvas;         // Canvas used for screen display.
    TsKeypad         &m_Ts;             // Touch screen reference.
    uint16_t          m_BgColor;        // Actual Background color.
    int16_t           m_XTouch;         // Fast access to current touch X value.
    int16_t           m_YTouch;         // Fast access to current touch Y value.
    int16_t           m_TouchIndex;     // Fast access to SCR_DATA array index
                                        // pressed in the actual Screen.
    scr_vec_t        *m_pCurrentScreen; // Pointer to the currently active screen.
    int32_t           m_StartTime;      // Time at which the current screen started.
                                        // Used for timeout checking.
    int32_t           m_ScreenTimeout;  // Screen timeout in milliseconds.
    void  (*m_TimeoutCallback)(int16_t);// Screen timeout callback function.
}; // End GuiDriver.


#endif // GUI_DRIVER_H.
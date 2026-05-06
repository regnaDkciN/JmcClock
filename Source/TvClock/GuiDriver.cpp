/******************************************************************************
* GuiDriver.h
*
* Contains methods that implement GUI screens for the TV Clock.
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

#include <arduino.h>                // For standard Arduino stuff.
#include <Adafruit_GFX.h>           // For graphics (TFT and canvas) classes.
#include <Adafruit_ILI9341.h>       // For Adafruit 2.8" TFT display.
#include <TouchScreen.h>            // For touchscreen interface.
#include "GuiDriver.h"              // For our class declaration.


/*******************************************************************************
* Constructor
*
* Arguments:
*   rTft    - Reference to the TFT display being used.
*   rCanvas - Reference to the canvas used with the tft display.
*   rTs     - Reference to the touchscreen being used.
*******************************************************************************/
GuiDriver::GuiDriver(Adafruit_ILI9341 &rTft, GFXcanvas16 &rCanvas, TsKeypad &rTs) :
    m_Tft(rTft), m_Canvas(rCanvas), m_Ts(rTs), m_BgColor(ILI9341_BLACK),
    m_XTouch(0), m_YTouch(0), m_TouchIndex(0), m_pCurrentScreen(NULL),
    m_StartTime(millis()), m_ScreenTimeout(0), m_TimeoutCallback(NULL)
{ }


/*******************************************************************************
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
*   pObj     - Pointer to the BUTTON_SLH object.   Must be a BUTTON_SLH object.
*   pct      - Percentage Fill for the sliderbar (required to determine left
*              or right slider if LoPct is negative and HiPct is positive).
*              Must always be provided as the % click on the bar
*              (max range -128 to + 127).
*   enXtouch - TOUCHPOS_REF uses actual xTouch position to determine where to
*              relocate the button object (fastest, only possible by touch
*              operation).
*              TOUCHPOS_CALC (default): compute a virtual touch position
*              (slower, can be used to put the object at a specific position).
*
* Note:
*    If the BUTTON_SLH is not correctly configured, it may not be drawn on
*    the screen.
*******************************************************************************/
void GuiDriver::ButtonSLHPct(SCR_DATA* pObj, int16_t pct, TOUCH_REFERENCE enXtouch)
{
    // Make sure there's something to do.  If not just return.
    if ((pObj == NULL) || (pObj->m_Obj != BUTTON_SLH))
    {
        return;
    }

    // Cache our data.
    int16_t xt = m_XTouch;
    int16_t xL = pObj->m_TX1 - pObj->m_TX0;
    int16_t x0 = pObj->m_TX0;
    int16_t y0 = pObj->m_TY0;
    uint16_t oH = pObj->m_OH;
    uint16_t oW = pObj->m_OW;
    uint16_t hoH = pObj->m_OH / 2;
    uint16_t oC = pObj->m_OC;
    const char *pTxt = pObj->m_pTxt;

    // Compute the virtual touch position is so specified.
    if (enXtouch == TOUCHPOS_CALC)
    {
        int16_t pctPos = pct - pObj->m_LoPct;   // Diff pct to lowest possible %.
        int32_t p32 = xL * pctPos;
        pctPos = pObj->m_HiPct - pObj->m_LoPct; // Maximum possible range.
        p32 /= pctPos;
        xt = x0 + (int16_t)(p32);
        if (xt < x0)
        {
            xt = x0;
        }
    }
    // Draw the empty slider to the canvas.
    // Erase above the line.
    m_Canvas.fillRect(x0, y0, xL, hoH, m_BgColor);
    // Erase below the line.
    m_Canvas.fillRect(x0, y0 + hoH + 1, xL, hoH - 1, m_BgColor);
    // Draw the line.
    m_Canvas.drawFastHLine(x0, y0 + hoH, xL, oC);

    // The object X-positon cannot exceed (m_TX1 - width).
    if (xt > pObj->m_TX1 - oW)
    {
        xt = pObj->m_TX1 - oW;
    }
    // Draw a new position object.
    m_Canvas.fillRect(xt, y0, oW, oH, oC);

    // Display any associated text.
    if (pTxt && *pTxt)
    {
        int16_t x1 = 0;
        int16_t y1 = 0;
        uint16_t w = 0;
        uint16_t h = 0;
        uint16_t fS = pObj->m_FS;

        m_Canvas.setFont(pObj->m_pFont);
        m_Canvas.setTextSize(fS);
        m_Canvas.getTextBounds(pTxt, 0, 0, &x1, &y1, &w, &h);

        // We account for the extra space at the bottom and end of the final
        // character by reducing the width and height by the size multiplier
        // of the font.
        w -= fS;
        h -= fS;

        // Put the text out to the canvas.
        m_Canvas.setTextColor(pObj->m_LC);
        m_Canvas.setCursor(xt - x1 + (oW - w) / 2, y0 - y1 + (oH - h) / 2);
        m_Canvas.print(pTxt);
    }
} // End ButtonSLHPct().


/*******************************************************************************
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
*******************************************************************************/
void GuiDriver::ButtonSLVPct(SCR_DATA* pObj, int16_t pct, TOUCH_REFERENCE enYtouch)
{
    // Make sure there's something to do.  If not just return.
    if ((pObj == NULL) || (pObj->m_Obj != BUTTON_SLV))
    {
        return;
    }

    // Cache our data.
    int16_t yt = m_YTouch;
    int16_t yL = pObj->m_TY1 - pObj->m_TY0;
    int16_t tx0 = pObj->m_TX0;
    int16_t ty0 = pObj->m_TY0;
    int16_t ty1 = pObj->m_TY1;
    uint16_t oH = pObj->m_OH;
    uint16_t oW = pObj->m_OW;
    uint16_t hoW = oW / 2;
    uint16_t oC = pObj->m_OC;
    const char *pTxt = pObj->m_pTxt;

    // Compute the virtual touch position is so specified.
    if (enYtouch == TOUCHPOS_CALC)
    {
        int16_t pctPos = pct - pObj->m_LoPct;   // Diff pct to lowest possible %.
        int32_t pct = yL * pctPos;
        pctPos = pObj->m_HiPct - pObj->m_LoPct; // Maximum possible range.
        pct /= pctPos;
        yt = ty1 - pct;
        if (yt < ty0)
        {
            yt = ty0;
        }
    }
    // Draw the empty slider to the canvas.
    // Erase left of the line.
    m_Canvas.fillRect (tx0, ty0, hoW, yL, m_BgColor);
    // Erase right from the line.
    m_Canvas.fillRect (tx0 + hoW + 1, ty0, hoW - 1, yL, m_BgColor);
    // Draw the line.
    m_Canvas.drawFastVLine(tx0 + hoW, ty0, yL, oC);

    // The object Y-positon cannot exceed (m_TX1 - width).
    if (yt > (ty1 - oH))
    {
        yt = ty1 - oH;
    }
    // Draw a new position object.
    m_Canvas.fillRect (tx0, yt, oW, oH, oC);

    // Display any associated text.
    if (pTxt && *pTxt)
    {
        int16_t x1 = 0;
        int16_t y1 = 0;
        uint16_t w = 0;
        uint16_t h = 0;
        uint16_t fS = pObj->m_FS;

        m_Canvas.setFont(pObj->m_pFont);
        m_Canvas.setTextSize(fS);
        m_Canvas.getTextBounds(pTxt, 0, 0, &x1, &y1, &w, &h);

        // We account for the extra space at the bottom and end of the final
        // character by reducing the width and height by the size multiplier
        // of the font.
        w -= fS;
        h -= fS;

        // Put the text out to the canvas.
        m_Canvas.setTextColor(pObj->m_LC);
        m_Canvas.setCursor(tx0 - x1 + (oW - w) / 2, yt - y1 + (oH - h) / 2);
        m_Canvas.print(pTxt);
    }
} // End ButtonSLVPct().


/*******************************************************************************
* UpdateCheckbox()
*
* Displays the CHECKBOX object on the screen.
*
* Arguments:
*   pSd - Pointer to the CHECKBOX object.  Must be a CHECKBOX object.
*******************************************************************************/
void GuiDriver::UpdateCheckbox(SCR_DATA* pSd)
{
    // Make sure there's something to do.  If not just return.
    if ((pSd == NULL) || (pSd->m_Obj != CHECKBOX))
    {
      return;
    }

    // Cache our data.
    uint16_t oW = pSd->m_OW;
    uint16_t oH = pSd->m_OH;

    // If the box is empty, simply display an empty box.
    if (pSd->m_Val == 0)
    {
      m_Canvas.fillRect (pSd->m_TX0 + CHECKBOX_WALLS, pSd->m_TY0 + CHECKBOX_WALLS,
                         oW - (CHECKBOX_WALLS << 1), oH - (CHECKBOX_WALLS << 1),
                         m_BgColor);
    }
    else
    {
        // Box is not empty.  Use oE to determine how to fill it.
        switch (pSd->m_OE)
        {
        case 1: // X-fill.
            {
            uint16_t epx = pSd->m_OX0 + oW;
            uint16_t epy = pSd->m_OY0 + pSd->m_OH;
            m_Canvas.drawLine (pSd->m_OX0, pSd->m_OY0, epx, epy, pSd->m_OC);
            m_Canvas.drawLine (pSd->m_OX0, epy, epx, pSd->m_OY0, pSd->m_OC);
            }
            break;
        default: // Filled rectangle.
            m_Canvas.fillRect (pSd->m_OX0, pSd->m_OY0, oW, oH, pSd->m_OC);
            break;
        }
    }
} // End UpdateCheckbox().


/*******************************************************************************
* UpdateRadioButton()
*
* Displays the RADIO_BUTTON object on the screen.
*
* Arguments:
*   pSd - Pointer to the RADIO_BUTTON object.  Must be a RADIO_BUTTON object.
*******************************************************************************/
void GuiDriver::UpdateRadioButton(SCR_DATA* pSd)
{
    // Make sure there's something to do.  If not just return.
    if ((pSd == NULL) || (pSd->m_Obj != RADIO_BUTTON))
    {
      return;
    }

    // Cache our data.
    uint16_t oW = pSd->m_OW;
    uint16_t oX0 = pSd->m_OX0;
    uint16_t oY0 = pSd->m_OY0;
    uint16_t hh = oY0 + (oW / 2);
    uint16_t oC = pSd->m_OC;

    // Make sure to clear the area where the button will be.
    m_Canvas.drawCircle (oX0 + (oW / 2), hh, oW / 2, m_BgColor);

    // Display the empty or filled circle based on val.
    if (pSd->m_Val == 0)
    {
        m_Canvas.drawCircle (oX0 + (oW / 2), hh, oW / 2, oC);
    }
    else
    {
        m_Canvas.fillCircle (oX0 + (oW / 2), hh, oW / 2, oC);
    }
} // End UpdateRadioButton().


/*******************************************************************************
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
*    SCR_DATA* p = &sd[?];  // ? points to an existing SLIDERBAR_H object.
*    int16_t  pct = 0;
*    SliderBarHPct (p, 75); // Fills the sliderbar to 75%.
*******************************************************************************/
void GuiDriver::SliderBarHPct(SCR_DATA *pObj, int16_t pct, TOUCH_REFERENCE enXtouch)
{
    // Make sure there's something to do.  If not just return.
    if (!pObj || (pObj->m_Obj != SLIDERBAR_H))
    {
        return;
    }

    // Calculate the slide bar position.
    int16_t xt = m_XTouch;
    int16_t valRange = pObj->m_HiPct - pObj->m_LoPct;
    if (enXtouch == TOUCHPOS_CALC)
    {
        // pct = minimal: leftmost position (prevent division by zero).
        if (pct == pObj->m_LoPct)
        {
            xt = pObj->m_TX0;
        }
        else
        {
            xt = pct - pObj->m_LoPct;           // Compute the absolute percentage vs.
                                                // the sLoPct (does not fit in 8bit signed).
            uint16_t pct = xt * pObj->m_OW;     // Scale percentage on sliderbar as if touched.
            xt = pObj->m_HiPct - pObj->m_LoPct; // The total value range.
            pct /= xt;                          // The relative touch position on the slider.
            xt = pct + pObj->m_TX0;             // Absolte X touch on the screen.
        }
    }
    uint16_t xPos0 = pObj->m_OW;    // Compute the Bar Pixel Width * 100
                                    // (to avoid losses on next divisions).
    xPos0 *= 100;
    xPos0 /= valRange;              // Compute the Bar Pixel Width * 100
                                    // (to avoid losses on next divisions).
    xPos0 *= -pObj->m_LoPct;        // Deduct Lower value.
    xPos0 +=  50;                   // Round by 0.5 pix.
    xPos0 /= 100;                   // Scale down 100 to have pixelcount.
    if (xPos0 > pObj->m_OW)
    {
        xPos0 = 0;    // Overflow correction.
    }
    int16_t xw = 0;
    if (pct < 0)
    {
        xw = xt - pObj->m_TX0 - 1;      // Touch position - XLeftmost.
        if (xw > 0)
        {
            // Left of inactive bar (when startpoint negative).
            m_Canvas.fillRect (pObj->m_TX0 + 1, pObj->m_TY0 + 1, xw, pObj->m_OH - 2, m_BgColor);
        }
        xw = xPos0 - xt + pObj->m_TX0;
        if (xw > 0)
        {
            m_Canvas.fillRect(xt, pObj->m_TY0, xw, pObj->m_OH, pObj->m_OC);  // Active Bar.
        }
        // There is touch area remaining to the right of the touch point and
        // Rightmost value > 0.
        if ((xt < pObj->m_TX1) && (pObj->m_HiPct > 0))
        {
            xw = xPos0 - 1;   // Width of erased bar at the right of the filled bar.
            if (xw > 0)
            {
                m_Canvas.fillRect(xPos0 + pObj->m_TX0, pObj->m_TY0 + 1, xw, pObj->m_OH - 2, m_BgColor);
            }
        }
    }
    else
    {
        // The zero point is to the right of the touch point.
        if (xt > (pObj->m_TX0 + xPos0))
        {
            // Positive percentage: wipe the area left from the zero point.
            xw = xPos0 - 1;
            if (xw > 0)
            {
                m_Canvas.fillRect(pObj->m_TX0 + 1, pObj->m_TY0 + 1, xw, pObj->m_OH - 2, m_BgColor);
            }
        }
        // Positive %; calculate the length of the positive bar range.
        xw = xt - xPos0 - pObj->m_TX0;
        if (xw > 0)
        {
            m_Canvas.fillRect(pObj->m_TX0 + xPos0, pObj->m_TY0, xw, pObj->m_OH, pObj->m_OC);
        }
        xw = pObj->m_OW - xt + pObj->m_TX0 - 1;
        if (xw > 0)
        {
            // Positive %: the remainder area from xTouch to the maximal value.
            m_Canvas.fillRect(xt, pObj->m_TY0 + 1, xw, pObj->m_OH - 2, m_BgColor);
        }
    }
} // End SliderBarHPct().


/*******************************************************************************
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
*    pct        Percentage fill for the sliderbar (required to determine up
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
*    SCR_DATA* p = &sd[?];      // ? points to an existing SLIDERBAR_V object.
*    int16_t   pct = 0;
*    SliderBarVPct (p, 75, 1);  // Fills the sliderbar to 75%.
*******************************************************************************/
void GuiDriver::SliderBarVPct(SCR_DATA *pObj, int16_t pct, TOUCH_REFERENCE enYtouch)
{
    // Make sure there's something to do.  If not just return.
    if (!pObj || (pObj->m_Obj != SLIDERBAR_V))
    {
        return;
    }

    // Calculate the slide bar position.
    int16_t yt = m_YTouch;
    int16_t valRange = pObj->m_HiPct - pObj->m_LoPct;
    if (enYtouch == TOUCHPOS_CALC)
    {
        // pct = minimal: bottom position (prevent division by zero).
        if (pct == pObj->m_LoPct)
        {
            yt = pObj->m_TY1;
        }
        else
        {
            yt = pct - pObj->m_LoPct;           // Compute the absolute percentage vs.
                                                // the sLoPct (does not fit in 8bit signed).
            uint16_t pct = yt * pObj->m_OH;     // Scale percentage on sliderbar as if touched.
            yt = pObj->m_HiPct - pObj->m_LoPct; // The total value range.
            pct /= yt;                          // The relative touch position on the slider.
            yt = pObj->m_TY1 - pct;             // Absolte Y touch on the screen
                                                // (computed from pct).
        }
    }

    uint16_t yPos0 = (pObj->m_OH * 100) / valRange; // Compute the Bar Pixel Height * 100
                                                    // (to avoid losses on next divisions).
    yPos0 *= -pObj->m_LoPct;                        // Deduct Lower value.
    yPos0 +=  50;                                   // Round by 0.5 pix.
    yPos0 /= 100;                                   // Scale down 100: pixelcount above
                                                    // tY1 in SLIDERBAR_V object.
    int16_t yh;
    if (pct < 0)                                    // Negative Percentage.
    {
        yh = pObj->m_TY1 - yt - 2;                  // Height from touch point to bottom.
        if (yh > 0)
        {
            // Erase from touch point to Y1.
            m_Canvas.fillRect(pObj->m_TX0 + 1, yt + 1, pObj->m_OW - 2, yh, m_BgColor);
        }
        yh = yt - yPos0 - pObj->m_TY0;
        if (yh > 0)
        {
            // Draw the active part of the bar from yPos0 to touch point.
            m_Canvas.fillRect(pObj->m_TX0, pObj->m_TY0 + yPos0, pObj->m_OW, yh, pObj->m_OC);
        }
        // For bipolar SLIDERBAR clear the positive half since pct is negative.
        yh = yPos0 - 2;
        if (yh > 0)
        {
            m_Canvas.fillRect(pObj->m_TX0 + 1, pObj->m_TY0 + 1, pObj->m_OW - 2, yh, m_BgColor);
        }
    }
    else   // Positive Percentage.
    {
        if (yPos0 == 0)
        {
            m_Canvas.fillRect(pObj->m_TX0 + 1, pObj->m_TY0 + 1, pObj->m_OW - 2, yt - pObj->m_TY0, m_BgColor);
            m_Canvas.fillRect(pObj->m_TX0, yt, pObj->m_OW, pObj->m_TY1 - yt, pObj->m_OC);
        }
        else
        {
            // Zero somehwere in SLIDERBAR_V: clear negative part.
            m_Canvas.fillRect(pObj->m_TX0 + 1, pObj->m_TY0 + yPos0 + 1,pObj->m_OW - 2, yPos0 - 2, m_BgColor);
            yh = yPos0 - yt + pObj->m_OY0;
            if (yPos0 == 0)
            {
                yh = abs(yh);   // yPos0 is at the bottom -> only use height.
            }
            if (yh > 0)
            {
                // Fill bar from touch to zero point.
                m_Canvas.fillRect(pObj->m_TX0, yt, pObj->m_OW, yh, pObj->m_OC);
            }
            yh = yt - pObj->m_TX0;
            if (yh > 2)
            {
                m_Canvas.fillRect(pObj->m_TX0 + 1, pObj->m_TY0, pObj->m_OW - 2, yh, m_BgColor);
            }
        }
    }
} // End SliderBarVPct().


/*******************************************************************************
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
*******************************************************************************/
int16_t GuiDriver::DrawScreen(scr_vec_t *pNewScreen, SCR_UPDATE drawMode)
{
    // Only draw the screen if the desired screen is not the current screen or
    // if directed to via SCR_DRAW.
    if ((pNewScreen == m_pCurrentScreen) && (drawMode == SCR_DRAW))
    {
      return 1;
    }

    // Reset our timeout start time when we switch screens.
    if (pNewScreen != m_pCurrentScreen)
    {
        m_StartTime = millis();
        m_ScreenTimeout = 0;
        m_TimeoutCallback = NULL;
    }

    // Make sure the new screen exists.
    if (pNewScreen == NULL)
    {
        m_pCurrentScreen = NULL;
        return -1;
    }

    // Make sure the new screen has at least 1 object.
    scr_iter_t sd = pNewScreen->begin();
    if (pNewScreen->size())
    {
        // Handle the BACKGROUND object specially.
        m_BgColor = sd->m_OC;
        if ((sd->m_Obj == BACKGROUND) && sd->m_Enable)
        {
            // First object is BACKGROUND and it is enabled.  Does it have a callback?
            if (sd->m_pClick)
            {
                // Perform callback.
                sd->m_pClick(0);
            }
            else
            {
                // No callback, just fill the background with the background color.
                m_Canvas.fillScreen(m_BgColor);
            }
        }
        // First object is not BACKGROUND.  If it's a new screen or SCR_REDRAW_W_CLR
        // then fill the background with the background color.
        else if ((pNewScreen != m_pCurrentScreen) || (drawMode == SCR_REDRAW_W_CLR))
        {
            m_Canvas.fillScreen(m_BgColor);
        }

        // Now draw the rest of the screen objects.
        DrawScreenObjects(pNewScreen);
        m_pCurrentScreen = pNewScreen;
        m_Tft.drawRGBBitmap(0, 0, m_Canvas.getBuffer(),
                            m_Canvas.width(), m_Canvas.height());
        return 0;
    }
    return -2;
} // End DrawScreen().


/*******************************************************************************
* DrawScreenObjects()
*
* Draws all of the screen objects from the selected screen data.
*
* Arguments:
*   pScr - Pointer to the vector containing the objects of the selected screen.
*******************************************************************************/
void GuiDriver::DrawScreenObjects(scr_vec_t* pScr)
{
    // Exit if there is nothing to do.
    if (pScr == NULL)
    {
        return;
    }

    // Loop through each screen object.
    for (scr_iter_t p = pScr->begin(); p != pScr->end(); ++p)
    {
        // Skip this entry if it is not enabled.
        if (!p->m_Enable)
        {
            continue;
        }

        // Cache the object's data for easier use later.
        uint16_t oC = p->m_OC;
        uint16_t oX0 = p->m_OX0;
        uint16_t oW = p->m_OW;
        uint16_t oY0 = p->m_OY0;
        uint16_t oH = p->m_OH;

        uint16_t lx0 = p->m_LX0;
        uint16_t ly0 = p->m_LY0;
        uint16_t oE = p->m_OE;

        const char* pTxt = p->m_pTxt;

        uint16_t fS = p->m_FS;
        uint16_t tX0 = p->m_TX0;
        uint16_t tX1 = p->m_TX1;
        uint16_t tY0 = p->m_TY0;
        uint16_t tY1 = p->m_TY1;

        int16_t oHiPct = p->m_HiPct;
        int16_t oLoPct = p->m_LoPct;

        int16_t x1 = 0;
        int16_t y1 = 0;
        uint16_t w = 0;
        uint16_t h = 0;

        // Is text used by this object?
        if (pTxt && *pTxt)
        {
            // Text is used.  Initialize the font that will be used to display it.
            if (!p->m_pFont)
            {
                m_Canvas.setFont();
            }
            else
            {
                m_Canvas.setFont(p->m_pFont);
            }
            m_Canvas.setTextSize(fS);
            m_Canvas.setTextWrap(false);

            // Calculate the space that will be occupied by the text.
            m_Canvas.getTextBounds(pTxt, 0, 0, &x1, &y1, &w, &h);

            // Account for the extra space at the bottom and end of the characters
            // by reducing the width and height by the size multiplier of the font.
            w -= fS;
            h -= fS;

            if (lx0)
            {
                lx0 -= x1;
            }
            if (ly0)
            {
                ly0 -= y1;
            }
        }

        // Handle each object type individually.
        switch (p->m_Obj)
        {
        case BUTTON_SLH:
            if ((oW >  3) && (oH > 9))
            {
                ButtonSLHPct(&*p, p->m_Val, TOUCHPOS_CALC);
            }
            break;
        case BUTTON_SLV:
            if ((oW > 9) && (oH >  3))
            {
                ButtonSLVPct(&*p, p->m_Val, TOUCHPOS_CALC);
            }
            break;
        case BUTTON_RECT:
            if ((oW > 5) && (oH > 9))
            {
                m_Canvas.fillRect (oX0, oY0, oW, oH, oC);
                if (!lx0)
                {
                    lx0 = oX0 - x1 + (oW - w) / 2;
                }
                if (!ly0)
                {
                    ly0 = oY0 - y1 + (oH - h) / 2;
                }
                if ((oE) && (oE < (oH / 2)) && (oE < (oW / 2)))
                {
                    // Make all colors more dark: divide all by 2; clear color shifts.
                    // 0b0111.1011.1110.1111 (from R to G and G to B).
                    uint16_t t = (oC / 2) & 0x7BEF;
                    m_Canvas.fillRect (oX0 + oW - oE, oY0, oE, oH, t);
                    m_Canvas.fillRect (oX0, oY0 + oH - oE, oW, oE, t);
                }
            }
            break;
        case BUTTON_RECT_RND:
            if ((oW > 9) && (oH > 9))
            {
                m_Canvas.fillRoundRect (oX0, oY0, oW, oH, oE, oC);
                if (!lx0)
                {
                    lx0 = oX0 - x1 + (oW - w) / 2;
                }
                if (!ly0)
                {
                    ly0 = oY0 - y1 + (oH - h) / 2;
                }
            }
            break;
        case BUTTON_ROUND:
            if (oW > 9)
            {   // Does not use oH (Shape Y Height).
                uint16_t hh = oY0 + (oW / 2);
                m_Canvas.fillCircle (oX0 + (oW / 2), hh, oW / 2, oC); // Width to radius.
                if (!ly0)
                {
                    ly0 = oY0 - y1 + (oW - h) / 2;
                }
                if (!lx0)
                {
                    lx0 = oX0 - x1 + (oW - w) / 2;
                }
            }
            break;
        case RADIO_BUTTON:
            if ((oW > 9) && (tX1 > (tX0 + 5)) && (tY1 > (tY0 + 5)))
            {   // Does not use oH (Shape Y Height).
                uint16_t hh = oY0 + (oW / 2);
                m_Canvas.drawCircle (oX0 + (oW / 2), hh, oW / 2, oC); // Width to radius.
                UpdateRadioButton(&*p);
                if (!lx0)
                {
                    lx0 = oX0 + oW - x1 + (tX1 - tX0 - oW - w) / 2;
                }
                if (!ly0)
                {
                    ly0 = oY0 - y1 + (oW - h) / 2;
                }
            }
            break;
        case CHECKBOX:
            if ((tX1 > (tX0 + 5)) && (tY1 > (tY0 + 5)))
            {
                uint16_t i = CHECKBOX_WALLS;     // Make the marker a bit wider.
                do
                {
                    m_Canvas.drawRect (oX0 + i, oY0 + i, oW - i * 2, oH - i * 2, oC);
                    --i;
                } while (i);
                UpdateCheckbox(&*p);
                if (!lx0)
                {
                    lx0 = oX0 + oW - x1 + (tX1 - tX0 - oW - w) / 2;
                }
                if (!ly0)
                {
                    ly0 = oY0 - y1 + (oH - h) / 2;
                }
            }
            break;
        case INPUT_DATA:
            break;
        case LINE:
            m_Canvas.drawLine(oX0, oY0, oW, oH, oC);
            break;
        case LINE_H:
            m_Canvas.drawFastHLine(oX0, oY0, oW, oC);
            break;
        case LINE_V:
            m_Canvas.drawFastVLine(oX0, oY0, oH, oC);
            break;
        case RECTANGLE:
            if ((oW >= 5) && (oH >= 5))
            {
                m_Canvas.drawRect(oX0, oY0, oW, oH, oC);
                if (!lx0)
                {
                    lx0 = oX0 - x1 + (oW - w) / 2;
                }
                if (!ly0)
                {
                    ly0 = oY0 - y1 + (oH - h) / 2;
                }
            }
            break;
        case LABEL:
                if (!lx0)
                {
                    lx0 = oX0 - x1 + (oW - w) / 2;
                }
                if (!ly0)
                {
                    ly0 = oY0 - y1 + (oH - h) / 2;
                }
                break;
        case RECTANGLE_FILL:
            if ((oW >= 5) && (oH >= 5))
            {
                m_Canvas.fillRect(oX0, oY0, oW, oH, oC);
                if (!lx0)
                {
                    lx0 = oX0 - x1 + (oW - w) / 2;
                }
                if (!ly0)
                {
                    ly0 = oY0 - y1 + (oH - h) / 2;
                }
            }
            break;
        case SLIDERBAR_H:
            if ((oW >= 20) && (oH >= 5) && (oHiPct > oLoPct))
            {
                m_Canvas.drawRect(oX0, oY0, oW, oH, oC);
                if (oE)
                {
                    uint16_t divPos = 0;
                    uint16_t div = oX0;
                    uint16_t step = oW / oE;
                    // This can overflow the slider bar if not well divided.
                    while (divPos <= oE)
                    {
                        m_Canvas.drawFastVLine(div, oY0 - 10, 10, oC);
                        div += step;
                        ++divPos;
                    }
                }
                SliderBarHPct(&*p, p->m_Val, TOUCHPOS_CALC);
            }
            break;
        case SLIDERBAR_V:
            if ((oW >= 5) && (oH >= 20) && (oHiPct > oLoPct))
            {
                m_Canvas.drawRect(oX0, oY0, oW, oH, oC);
                if (oE)
                {
                    uint16_t divPos = 0;
                    uint16_t div = oY0;
                    uint16_t step = oH / oE;
                    // This can overflow the slider bar if not well divided.
                    while (divPos <= oE)
                    {
                        m_Canvas.drawFastHLine(oX0 - 10, div, 10, oC);
                        div += step;
                        ++divPos;
                    }
                }
                SliderBarVPct(&*p, p->m_Val, TOUCHPOS_CALC);
            }
            break;
        case TIMEOUT:
            m_ScreenTimeout = p->m_Val;
            m_TimeoutCallback = p->m_pClick;
            break;
        default:
            break;
        } // End switch.

        // Text @ Position 0,0 is not allowed and Text must contain something.
        if ((lx0 || ly0) && pTxt && *pTxt)
        {
            // Display the text.
            uint16_t lC = p->m_LC;
            m_Canvas.setCursor(lx0, ly0);
            m_Canvas.setTextColor(lC);
            m_Canvas.print(pTxt);
        }

#if defined (TOUCH_HIGHLIGHT)
    // For debugging/testing - draw a box around the touch area.
    m_Canvas.drawRect(tX0 - 1, tY0 - 1, tX1 - tX0 + 2, tY1 - tY0 + 2, TOUCH_HIGHLIGHT);
    m_Canvas.drawRect(tX0 - 2, tY0 - 2, tX1 - tX0 + 4, tY1 - tY0 + 4, TOUCH_HIGHLIGHT);
#endif
    }// End for().
} // End DrawScreenObjects().


/*******************************************************************************
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
*******************************************************************************/
void GuiDriver::ScanScreen()
{
    // Just return if there is nothing to do here.
    if (m_pCurrentScreen == NULL)
    {
        return;
    }

    // Point to the first screen object.
    scr_iter_t pObj = m_pCurrentScreen->begin();

    // Check for screen timeout.
    if (m_ScreenTimeout && m_TimeoutCallback &&
       ((millis() - m_StartTime) >= (uint32_t)m_ScreenTimeout))
    {
        m_TimeoutCallback(0);
    }

    // Refresh the screen if it is an enabled BACKGROUND object and has a
    // background update (onClick) function.  This allows us to continuously
    // update dynamic screens.
    if (m_pCurrentScreen->size() && (pObj->m_Obj == BACKGROUND) &&
        pObj->m_Enable && pObj->m_OE && pObj->m_pClick)
    {
        DrawScreen(m_pCurrentScreen, SCR_REDRAW);
    }

    // Just return if no touch yet.
    TSPoint p;
    if (!m_Ts.GetTouch(p))
    {
        return;
    }

    // Reset our timeout any time we get a touch, whether valid or not.
    m_StartTime = millis();

    // Got a touch - handle it.
    m_XTouch = p.x;
    m_YTouch = p.y;

    // Loop through each screen object to see if the touch was intended for it.
    m_TouchIndex = 0;
    for (pObj = m_pCurrentScreen->begin(); pObj != m_pCurrentScreen->end(); ++pObj)
    {
        // Skip this entry if it is not enabled.
        if (!pObj->m_Enable)
        {
            ++m_TouchIndex;
            continue;
        }

        // Is the touch associated with this object?
        if ((m_XTouch >= pObj->m_TX0)  &&     // Touch X Left.
            (m_XTouch <= pObj->m_TX1)  &&     // Touch X Right.
            (m_YTouch >= pObj->m_TY0)  &&     // Touch Y Top.
            (m_YTouch <= pObj->m_TY1))        // Touch Y Bottom.
        {
            // Yes - touch is for this object.  Now handle it.
            bool allowExec = false;
            int16_t pct = 0;
            switch (pObj->m_Obj)
            {
            case BUTTON_SLH:
            {
                ButtonSLHPct(&*pObj, 0, TOUCHPOS_REF);
                uint16_t xPos0 = m_XTouch - pObj->m_TX0; // Relative position.
                xPos0 *= 100;                            // *100 to reduce int-division losses.
                xPos0 += 50;                             // Rounding.
                xPos0 /= (pObj->m_TX1 - pObj->m_TX0 -pObj->m_OW);
                int16_t valRange = xPos0 * (pObj->m_HiPct - pObj->m_LoPct);
                valRange /= 100;
                valRange += pObj->m_LoPct;
                pct = valRange;
                if (pct < pObj->m_LoPct)
                {
                    pct = pObj->m_LoPct;
                }
                if (pct > pObj->m_HiPct)
                {
                    pct = pObj->m_HiPct;
                }
                pObj->m_Val = pct;
                allowExec = true;
            }
            break;
            case BUTTON_SLV:
            {
                ButtonSLVPct(&*pObj, 0, TOUCHPOS_REF);
                uint16_t yPos0 = pObj->m_TY1 - m_YTouch; // Relative position.
                yPos0 *= 100;                            // *100 to reduce int-division losses.
                yPos0 += 50;                             // Rounding.
                yPos0 /= (pObj->m_TY1 - pObj->m_TY0 - pObj->m_OH);
                int16_t valRange = yPos0 * (pObj->m_HiPct - pObj->m_LoPct);
                valRange /= 100;
                valRange += pObj->m_LoPct;
                pct = valRange;
                if (pct < pObj->m_LoPct)
                {
                    pct = pObj->m_LoPct;
                }
                if (pct > pObj->m_HiPct)
                {
                    pct = pObj->m_HiPct;
                }
                allowExec = true;
                pObj->m_Val = pct;
            }
            break;
            case SLIDERBAR_H:
            {
                uint16_t xPos0 = m_XTouch - pObj->m_TX0; // Relative position.
                xPos0 *= 100;                            // *100 to reduce int-division losses.
                xPos0 += 50;                             // Rounding.
                xPos0 /= pObj->m_OW;
                int16_t valRange = xPos0 * (pObj->m_HiPct - pObj->m_LoPct);
                valRange /= 100;
                valRange += pObj->m_LoPct;
                pct = valRange;
                if (pct < pObj->m_LoPct)
                {
                    pct = pObj->m_LoPct;
                }
                if (pct > pObj->m_HiPct)
                {
                    pct = pObj->m_HiPct;
                }
                SliderBarHPct(&*pObj, pct, TOUCHPOS_REF);
                allowExec = true;
                pObj->m_Val = pct;
            }
            break;
            case SLIDERBAR_V:
            {
                uint16_t yPos0 = pObj->m_TY1 - m_YTouch; // Relative position.
                yPos0 *= 100;                            // *100 to reduce int-division losses.
                yPos0 /= pObj->m_OH;
                int16_t valRange = yPos0 * (pObj->m_HiPct - pObj->m_LoPct);
                valRange /= 100;
                valRange += pObj->m_LoPct;
                pct = valRange;
                if (pct < pObj->m_LoPct)
                {
                    pct = pObj->m_LoPct;
                }
                if (pct > pObj->m_HiPct)
                {
                    pct = pObj->m_HiPct;
                }
                SliderBarVPct(&*pObj, pct, TOUCHPOS_REF);
                allowExec = true;
                pObj->m_Val = pct;
            }
            break;
            default:
                allowExec = true;
                m_Ts.WaitKeyRelease(750);
                break;
            } // End switch.

            // Call the object's callback if one exists.
            if (allowExec && pObj->m_pClick)
            {
                pObj->m_pClick(m_TouchIndex);
                break;
            }
        } // End if().

        ++m_TouchIndex;
    } // End for().
} // End ScanScreen().

/*******************************************************************************
* Backlight.h
*
* Declares the Backlight class.  This class controls the backlight of the
* Adafruit 2.8" TFT LCD display breakout with resistive touchscreen.
*    https://www.adafruit.com/product/1770
*    https://learn.adafruit.com/adafruit-2-8-and-3-2-color-tft-touchscreen-breakout-v2/adafruit-gfx-library
* It should work with alnost any backlit display.
*
* This class can optionally use an LDR to automatically adjust backlight brightness.
*
* History:
*   24-APR-2026 JMC
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

#if !defined BACKLIGHT_H
#define BACKLIGHT_H


#include <arduino.h>     // For standard Arduino stuff.


/*******************************************************************************
* Backlight Class
*
* This class manages the display backlight including the saving to NVS and
* restoring from NVS of the backlight brightness settings.
*******************************************************************************/
class Backlight
{
public:
    /***************************************************************************
    * Constructor
    *
    * Initialize the Backlight object..
    *
    * Agruments:
    *   bPin   The processor pin used to control the backlight brightness.
    *   lPin   The processor pin used to read the LDR value.  -1 if no LDR.
    ***************************************************************************/
    Backlight(int16_t bPin, int16_t lPin);


    /***************************************************************************
    * SaveNvs()
    *
    * Saves brightness data to NVS.
    *
    * Agruments:
    *   pBuf   This is a pointer to the NVS buffer where our NVS data will be
    *          stored.
    ***************************************************************************/
    void SaveNvs(uint8_t *pBuf);


    /***************************************************************************
    * RestoreNvs()
    *
    * Restores brightness data from NVS.
    *
    * Agruments:
    *   pBuf   This is a pointer to the NVS buffer containing our restored NVS
    *          values.
    ***************************************************************************/
    void RestoreNvs(uint8_t *pBuf);


    /***************************************************************************
    * SetBrightness()
    *
    * Sets the backlight brightness.
    *
    * If an LDR is used, then the brightness value can range from MIN_BRIGHTNESS
    * to MAX_BRIGHTNESS  since it is actually an offset to be added to the
    * brightnessw value determined by the LDR.
    *
    * If an LDR is not used, then the brightness value can range from 0 (off)
    * to MAX_BRIGHTNESS (full brightness).
    ***************************************************************************/
    void SetBrightness(int16_t b);


    /***************************************************************************
    * UseLdr()
    *
    * Sets up to either use or not use the LDR for automatic control of the
    * backlight..
    *
    * If an LDR is used, then the brightness value can range from MIN_BRIGHTNESS
    * to MAX_BRIGHTNESS  since it is actually an offset to be added to the
    * brightnessw value determined by the LDR.
    *
    * If an LDR is not used, then the brightness value can range from 0 (off)
    * to MAX_BRIGHTNESS (full brightness).
    *
    * Arguments:
    *   use   This determines if the LDR will be used or not.
    ***************************************************************************/
    void UseLdr(bool use);


    /***************************************************************************
    * AdjustBrightness()
    *
    * Adjusts the backlight brightness.
    *
    * If no LDR is used, then the current m_Brightness value is passed directly
    * to the backlight (0 for off, MAX_BRIGHTNESS for maximum brightness).
    *
    * If an LDR is used, then the brightness value depends on the amount of light
    * seen by the LDR.  In this case, since the LDR analog input circuitry can
    * get some noise, the LDR value is passed through a low pass filter to
    * smoothe it out.  Then the m_Brightness value is added in as an offset.
    ***************************************************************************/
    void AdjustBrightness() const;


    // Simple methods.
    size_t  GetNvsSize()       const { return sizeof(m_Brightness) +
                                              sizeof(m_LdrUsed); }
    int16_t GetBrightness()    const { return m_Brightness; }
    int16_t GetMinBrightness() const { return m_MinBrightness; }
    int16_t GetMaxBrightness() const { return MAX_BRIGHTNESS; }
    int16_t GetRange()         const { return MAX_BRIGHTNESS - m_MinBrightness; }
    bool    IsLdrPresent()     const { return m_LdrPin != -1; }
    bool    IsLdrUsed()        const { return m_LdrUsed; }


private:
    // Unimplimented methods.
    Backlight();
    Backlight(Backlight &r);
    Backlight &operator=(Backlight &r);

    // Maximum and minimum brightness values.
    static const int     ANALOG_BITS    = 10;  // Number of analog write bits.
    static const int16_t MAX_BRIGHTNESS = (1 << ANALOG_BITS) - 1;
    static const int16_t MIN_BRIGHTNESS = -MAX_BRIGHTNESS;

    int16_t m_BacklightPin;     // Pin used for backlight control..
    int16_t m_LdrPin;           // Pin connected to LDR (-1 if not used).
    int16_t m_MinBrightness;    // Minimum brightness based on LDR use.
    int16_t m_Brightness;       // The brightness if no LDR is used. or
                                // the offset from LDR value, if LDR used.
    bool    m_LdrUsed;          // True if LDR is taken advantage of.
}; // End class Backlight.


#endif // BACKLIGHT_H.

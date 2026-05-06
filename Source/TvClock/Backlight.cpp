/*******************************************************************************
* Backlight.cpp
*
* Implements the Backlight class.  This class  This class controls the backlight
* of the Adafruit 2.8" TFT LCD display breakout with resistive touchscreen.
*    https://www.adafruit.com/product/1770
*    https://learn.adafruit.com/adafruit-2-8-and-3-2-color-tft-touchscreen-breakout-v2/adafruit-gfx-library
* It should work with alnost any backlit display.
*
* This class can optionally use an LDR to automatically adjust backlight
* brightness.
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

#include "Backlight.h"       // Our class.


/*******************************************************************************
* Constructor
*
* Initialize the Backlight object..
*
* Agruments:
*   bPin   The processor pin used to control the backlight brightness.
*   lPin   The processor pin used to read the LDR value.  -1 if no LDR.
*******************************************************************************/
Backlight::Backlight(int16_t bPin, int16_t lPin) :
    m_BacklightPin(bPin), m_LdrPin(lPin), m_LdrUsed(m_LdrPin != -1)
{
    m_MinBrightness = IsLdrUsed() ? MIN_BRIGHTNESS : 0;
    m_Brightness = (lPin == -1) ? MAX_BRIGHTNESS : 0;

    // Setup the TFT backlight hardware.
    pinMode(bPin, OUTPUT);
    digitalWrite(bPin, HIGH); // Start with the backlight on.
    analogWriteFreq(10000);
    analogWriteResolution(ANALOG_BITS);

    // Setup the analog resolution for reading the LDR.
    pinMode(lPin, INPUT);
    analogReadResolution(ANALOG_BITS);
} // End constructor.


/*******************************************************************************
* SaveNvs()
*
* Saves brightness data to NVS.
*
* Agruments:
*   pBuf   This is a pointer to the NVS buffer where our NVS data will be
*          stored.
*******************************************************************************/
void Backlight::SaveNvs(uint8_t *pBuf)
{
    memcpy(pBuf, &m_Brightness, sizeof(m_Brightness));
    memcpy(pBuf + sizeof(m_Brightness), &m_LdrUsed, sizeof(m_LdrUsed));
} // End SaveNvs().


/*******************************************************************************
* RestoreNvs()
*
* Restores brightness data from NVS.
*
* Agruments:
*   pBuf   This is a pointer to the NVS buffer containing our restored NVS
*          values.
*******************************************************************************/
void Backlight::RestoreNvs(uint8_t *pBuf)
{
    memcpy(&m_Brightness, pBuf, sizeof(m_Brightness));
    memcpy(&m_LdrUsed, pBuf + sizeof(m_Brightness), sizeof(m_LdrUsed));
} // End RestoreNvs().


/*******************************************************************************
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
*******************************************************************************/
void Backlight::SetBrightness(int16_t b)
{
    // Constrain the new brightness value.
    m_Brightness = constrain(b, m_MinBrightness, MAX_BRIGHTNESS);
} // End SetBrightness().


/*******************************************************************************
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
*******************************************************************************/
void Backlight::UseLdr(bool use)
{
    m_LdrUsed = use & IsLdrPresent();
    m_MinBrightness = m_LdrUsed ? MIN_BRIGHTNESS : 0;
} // End UseLdr().


/*******************************************************************************
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
*******************************************************************************/
void Backlight::AdjustBrightness() const
{
    int16_t analogValue = m_Brightness;
    // Are we using an LDR?
    if (IsLdrUsed())
    {
        // Yes, smoothe the LDR input via a low pass filter.
        // The 'z' constant determines how long it takes for a change in LDR
        // value to be seen.  Smaller values take longer.  This filter helps
        // to keep the screen from distracting brightness changes due to
        // analog noise in the LDR circuit.
        // A good value of 'z' for our application is 0.1.
        const float_t z = .01;
        // The average starts at 0.
        static float_t avg = 0.;
        // Read the LDR value.
        int16_t ldr = analogRead(m_LdrPin);
        float_t ldrf = (float_t)ldr;
        // Calculate the filterd LDR value and map it to the corresponding
        // backlight value.
        avg = (z * ldrf) + (1.0 - z) * avg;
        int16_t mapped = map((int)avg, 5, 950, MAX_BRIGHTNESS, 1);
        mapped += m_Brightness;
        analogValue = constrain(mapped, 1, MAX_BRIGHTNESS);
    }
    // Write the filtered value to the backlight.
    analogWrite(m_BacklightPin, analogValue);
} // End AdjustBrightness();

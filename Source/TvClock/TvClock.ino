/*******************************************************************************
* TvClock.ino
*
* This file contains the main Arduino setup() and loop() functions to run the
* TV Clock.  The TV Clock has the following functionality:
*   - Configurable display with options for displaying:
*       o 12 or 24 hour display
*       o AM/PM
*       o Timezone
*       o Day of week
*       o Running seconds
*       o Date
*       o Temperature in degrees C or  degrees F
*       o Color cycling of displayed data
*       o Periodic changing of the main time font
*   - A large variety of main time fonts to choose from.
*   - Automatic brightness adjustment based on ambient light.
*   - Can connect to an NTP server to get accurate time from the internet.
*   - A battery backed Real Time Clock (RTC) that keeps accurate time even when
*     power is lost for extended periods or when the internet connection is lost.
*   - Timezone selection which contains most of the world's timezones.
*   - Settings are saved in non-volatile storage which remembers saved settings
*     across power cycles.
*   - A rudimentary web interface for viewing and changing clock options.
*
* The following hardware is supported:
*   - Raspberry Pi Pico 2W processor.
*   - Adafruit 2.8" TFT LCD display breakout with resistive touchscreen.
*      https://www.adafruit.com/product/1770
*      https://learn.adafruit.com/adafruit-2-8-and-3-2-color-tft-touchscreen-breakout-v2/adafruit-gfx-library
*      uses the default RP2350 SPI0 pins as follows:
*           MOSI (SPI0 TX)  GPIO 19
*           CLK  (SPI0 SCK) GPIO 18
*           CS   (SPI0 CSn) GPIO 17
*           MISO (SPI0 RX)  GPIO 16
*           RST             GPIO 9
*           DC              GPIO 8
*           BACK LIGHT      GPIO 7
*   - Touch screen pins
*     https://learn.adafruit.com/adafruit-2-8-and-3-2-color-tft-touchscreen-breakout-v2/resistive-touchscreen
*           X+ GPIO 10
*           X- A0
*           Y+ A1
*           Y- GPIO 11
*   - DS3231 Real Time Clock (RTC) module. https://www.adafruit.com/product/3013
*           SDA  I2C0 SDA (GPIO 4)
*           SCL  I2C0 SCL (GPIO 5)
*   - LDR with 10K resistor uses analog pin A2.
*
* A future enhancement may include the DFRobot SEN0539-EN voice input module:
*   https://www.dfrobot.com/product-2665.html
*   https://wiki.dfrobot.com/SKU_SEN0539-EN_Gravity_Voice_Recognition_Module_I2C_UART#target_9
*   https://www.hackster.io/maheshyadav216/getting-started-with-dfrobot-voice-recognition-sensor-e29488
*   https://www.dfrobot.com/forum/topic/354197
*   May need 10K pullups.
*
* History:
*   04-HUN-2026 JMV
*      Changed TFT rotation from 1 to 3 to match case setup. Minor commennt fixes.
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

// Uncomment the following line to enable debugging.
// #define DEBUGGING 1

#include "TvClock.h"        // Includes several useful header files.
#include "GuiScreens.h"     // For initial screen display.
#include "ClockWiFi.h"      // For WiFi object.
#include "ClockNvs.h"       // For NVS object.
#include "ClockWelcome.h"   // For displaying our welcome screen.
#include "ClockWebServer.h" // For web server interface.


// Adafruit 2.8" TFT display breakout board pin assignments.
#define TFT_CS        17
#define TFT_RST        9
#define TFT_DC         8
#define TFT_BACKLIGHT  7 // Display backlight pin

// Touchscreen pin assignments.
#define YP A1  // Must be an analog pin.
#define XM A0  // Must be an analog pin.
#define YM 11  // Can be any digital pin.
#define XP 10  // Can be any digital pin.

// LDR assignment.
#define LDR_PIN       A2

// For 2.8" TFT with ILI9341 use:
const int16_t SCREEN_WIDTH = 320;
const int16_t SCREEN_HEIGHT = 240;

// Create our globally used objects.
// TFT display.
Adafruit_ILI9341 gTft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
// Graphics canvas.
GFXcanvas16 gCanvas(SCREEN_WIDTH, SCREEN_HEIGHT);
// Touchscreen keypad.
TsKeypad &gTs = TsKeypad::Instance(gTft, gCanvas, XP, YP, XM, YM, false);
// GUI screen collection.
GuiDriver gGui(gTft, gCanvas, gTs);
// Time (main) display.
TimeDisplay gTd(gTft, gCanvas, gTs, false);
// Display backlight.
Backlight gBacklight(TFT_BACKLIGHT, LDR_PIN);


// The following allow debugging by setting a specific date and time for display.
#if defined DEBUGGING
uint16_t month = 1;
uint16_t day = 1;
uint16_t year = 2025 - 1900;
uint16_t hour = 1;
uint16_t minute = 1;
uint16_t second = 1;
uint16_t dst = 1;
#endif


#if defined DEBUGGING

/*******************************************************************************
* HandleRemoteCommands()
*
* This function is mainly used for debugging.  It allows for entry of commands
* via the serial port.
*******************************************************************************/
void HandleRemoteCommands()
{
    // Setup a buffer to hold any received commands.
    const uint_fast16_t BUFLEN = 100;
    char buf[BUFLEN];
    char *endptr;   // Needed for strtoul() and strtod().

    // See if we got any commands.
    if (Serial.available() > 0)
    {
        // Read the command and remove any trailing junk from the input buffer.
        int_fast16_t numBytesRead = Serial.readBytesUntil('\n', buf, BUFLEN - 1);
        while (isspace(buf[numBytesRead - 1]) && (numBytesRead > 0))
        {
            buf[--numBytesRead] = '\0';

        }
        // Handle the remote command.
        switch (toupper(buf[0]))
        {
            case 'M': // Set/display month.
                if (numBytesRead > 1)
                {
                    month = strtoul(buf + 1, &endptr, 10);
                    month = constrain(month, 0, 11);
                }
                Serial.printf("%d\n", month);
                break;
            case 'D': // Set/display day.
                if (numBytesRead > 1)
                {
                    day = strtoul(buf + 1, &endptr, 10);
                    day = constrain(day, 1, 31);
                }
                Serial.printf("%d\n", day);
                break;
            case 'Y': // Set/display year.
                if (numBytesRead > 1)
                {
                    year = strtoul(buf + 1, &endptr, 10);
                    year = constrain(year, 2000, 2030) - 1900;
                }
                Serial.printf("%d\n", year);
                break;
            case 'H': // Set/display hour.
                if (numBytesRead > 1)
                {
                    hour = strtoul(buf + 1, &endptr, 10);
                    hour = constrain(hour, 0, 23);
                }
                Serial.printf("%d\n", hour);
                break;
            case '"': // Set/display minute.
                if (numBytesRead > 1)
                {
                    minute = strtoul(buf + 1, &endptr, 10);
                    minute = constrain(minute, 0, 59);
                }
                Serial.printf("%d\n", minute);
                break;
            case 'S': // Set/display second.
                if (numBytesRead > 1)
                {
                    second = strtoul(buf + 1, &endptr, 10);
                    second = constrain(second, 0, 59);
                }
                Serial.printf("%d\n", second);
                break;
            case 'Z': // Set/display dst.
                if (numBytesRead > 1)
                {
                    dst = strtoul(buf + 1, &endptr, 10);
                    dst = constrain(dst, 0, 12 * 60);
                }
                Serial.printf("%d\n", second);
                break;
            case '+': // Increment brightness.
                {
                    uint16_t brite = gBacklight.GetBrightness();
                    if (numBytesRead > 1)
                    {
                        brite += strtoul(buf + 1, &endptr, 10);
                    }
                    else
                    {
                        brite += 10;
                    }
                    gBacklight.SetBrightness(brite);
                    Serial.printf("%d\n", gBacklight.GetBrightness());
                    break;
                }
            case '-': // Decrement brightness.
                {
                    uint16_t brite = gBacklight.GetBrightness();
                    if (numBytesRead > 1)
                    {
                        brite -= strtoul(buf + 1, &endptr, 10);
                    }
                    else
                    {
                        brite -= 10;
                    }
                    gBacklight.SetBrightness(brite);
                    Serial.printf("%d\n", gBacklight.GetBrightness());
                    break;
                }
            case '=': // Set brightness to specific value.
                {
                    if (numBytesRead > 1)
                    {
                        gBacklight.SetBrightness(strtoul(buf + 1, &endptr, 10));
                    }
                    Serial.printf("%d\n", gBacklight.GetBrightness());
                    break;
                }
            // Default - Not anything we know about.  Just ignore it.
            default:
                break;
        }
        second = (second + 1) % 60;
    }
} // End HandleRemoteCommands().

#endif // DEBUGGING



/*******************************************************************************
* setup()
*
* This is the standard Arduino setup() funciton.  It initializes all I/O and
* all subsystems.
*******************************************************************************/
void setup()
{
    // We always enable the serial port since many subsystems want to print
    // some data to the serial port.
    Serial.begin(115200);

    // Settling time for the serial port.
    delay(250);

    // SPI speed defaults to SPI_DEFAULT_FREQ defined in the library, you can
    // override it here.  Note that allowable speed depends on chip and quality
    // of wiring, if too fast, a black screen may result.
    gTft.setSPISpeed(40000000);

    // Initialise the display.
    gTft.begin();
    gTft.setRotation(3);            // Set to first landscape display.
    gCanvas.setFont();              // Set default font.
    gCanvas.setTextWrap(false);     // We don't want the deisplay to wrap.
    gTft.fillScreen(ILI9341_BLUE);  // Fill the screen with a background color.

    // Display a welcome screen.
    ClockWelcome::ShowWelcome();

    // Initialize our timezone.
    ClockTz::Init();

    // Initialize our wiFi connection.
    ClockWiFi::Init();

    // Restore our settable options.
    // If restore fails, then attempt to save our current settings.
    ClockNvs &nvs = ClockNvs::Instance();
    Serial.printf("Restoring settings...\n");
    if (!nvs.Restore())
    {
        Serial.printf("Restore failed...Saving current settings.\n");
        if (!nvs.Save())
        {
            Serial.printf("Save failed!\n");
        }
        else
        {
            Serial.printf("Save succeeded.\n");
        }
        if (!ClockWiFi::NtpConnected())
        {
            ClockWiFi::Connect();
        }

        ClockRtc &rtc = ClockRtc::Instance();
        rtc.SetClock();
    }
    else
    {
        Serial.printf("Restore succeeded.\n");
    }

    // Initialize our screen subsystem by displaying the main screen.
    InitScreens();

    // Start the server if the connection was successful.
    InitClockWebServer();

} // End setup().


/*******************************************************************************
* loop()
*
* This is the standard Arduino loop() funciton.  It displays the currently
* selected screen, and attempts to keep connections to the NTP server active.
 ******************************************************************************/
void loop()
{
    // Place any code here that should run only on the first iteration
    // after power-up.
    static bool first = true;
    if (first)
    {
        first = false;
    }

#if defined DEBUGGING
    // Handle debug commands if DEBUGGING is enabled.
    HandleRemoteCommands();
#endif

    // Update the time from NTP every hour, or every 5 minutes if not connected yet.
    static long lastTime = millis();
    long thisTime = millis();
    long deltaTime = thisTime - lastTime;
    ClockRtc &rtc = ClockRtc::Instance();
    if (!rtc.UsingNetworkTime() && (deltaTime > 5 * 60000))
    {
        ClockWiFi::Connect();
        lastTime = thisTime;
    }
    if (deltaTime > 60000 * 60)
    {
        rtc.SetClock();
        lastTime = thisTime;
    }

    // Update the web server.
    HandleClockWebServer();

    // Update the GUI screen.
    gGui.ScanScreen();

    //Update the display backlight brightness.
    gBacklight.AdjustBrightness();

    // No need to be in too much of a hurry.
    delay(10);
} // End loop().

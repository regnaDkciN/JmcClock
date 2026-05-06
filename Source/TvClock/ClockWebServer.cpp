/*******************************************************************************
* ClockWebServer.cpp
*
* Implements the server side web interface for the clock.
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

#include <WiFi.h>           // For WiFi library.
#include <WebServer.h>      // For web server library.
#include <LEAmDNS.h>        // For DNS support.
#include <ArduinoJson.h>    // For JSON support.
#include "TimeDisplay.h"    // For time and temperature data, etc.
#include "ClockRtc.h"       // For time and temperature data, etc.
#include "TvClock.h"        // For gTd.
#include "ClockWiFi.h"      // For WiFiConnected().
#include "ClockNvs.h"       // For NVS save/restore.

// ClockWebServer.h contains some declarations that are used by several 
// subsystems.  It also contains our web page strings that only this subsystem
// should see.  The following is defined before including ClockWebServer.h so 
// that we can see the web page strings.  Other subsystems should not define
// DEFINE_CLOCK_WEB_PAGES.
#define DEFINE_CLOCK_WEB_PAGES
#include "ClockWebServer.h"

// Create out locally global objects and constants.
static WebServer gServer(80);
static const char *gNetworkServerName = "JmcClock";
static const size_t gNumMainFontsPerTx = 4;
static const size_t gNumSecFontsPerTx = 2;


/*******************************************************************************
********************************************************************************
* MAIN PAGE RELATED FUNCTIONS.
********************************************************************************
*******************************************************************************/

/*******************************************************************************
* ShowMainPage()
*
* Sends the main (root) page to the client.
*******************************************************************************/
static void ShowMainPage()
{
    gServer.send(200, "text/html", gRootPage);
} // End ShowMainPage().


/*******************************************************************************
* SendMainPageData()
*
* Called when the client requests a refresh of the main page data.  Updates all
* of the relevant fields of the main page based on current values.
*******************************************************************************/
static void SendMainPageData()
{
    String webPage;
    JsonDocument doc;

    // TEMPERATURE
    ClockRtc &rtc = ClockRtc::Instance();
    float_t degrees = gTd.IsShowingDegreesF() ? rtc.GetTempF() : rtc.GetTempC();
    if (isnan(degrees))
    {
        doc["TEMPERATURE"]  = "-";
    }
    else
    {
        doc["TEMPERATURE"]  = degrees;
    }
    doc["TEMPERATURE_UNITS"] = gTd.IsShowingDegreesF() ? "F" : "C";

    // UP TIME
    doc["UPTIME"] = millis();

    // IP ADDRESS
    IPAddress ip = WiFi.localIP();
    String ipString = String(ip[0]) + String(".") +
                      String(ip[1]) + String(".") +
                      String(ip[2]) + String(".") +
                      String(ip[3]);
    doc["IP_ADDRESS"] = ipString;

    // WEB ID
    String http = "http://";
    String id = gNetworkServerName;
    doc["WEB_ID"] = http + id;

    // SIGNAL STRENGTH
    doc["SIGNAL_STRENGTH"] = WiFi.RSSI();

    serializeJson(doc, webPage);
    gServer.send(200, "text/html", webPage);
} // End SendMainPageData().



/*******************************************************************************
********************************************************************************
* TIME OPTIONS PAGE RELATED FUNCTIONS.
********************************************************************************
*******************************************************************************/

/*******************************************************************************
* ShowTimeOptsPage()
*
* Sends the time options page to the client.
*******************************************************************************/
static void ShowTimeOptsPage()
{
    gServer.send(200, "text/html", gTimeScreenOptsPage);
} // End ShowTimeOptsPage().


/*******************************************************************************
* SendTimeOptsData()
*
* Called when the client requests a refresh of the time options page data.
* Updates all of the relevant fields of the time options page based on current
* values.
*******************************************************************************/
static void SendTimeOptsData()
{
    String webPage;
    JsonDocument doc;

    // WEB ID
    String http = "http://";
    String id = gNetworkServerName;
    doc["WEB_ID"] = http + id;

    // CURRENT TIME OPTIONS
    doc["FMT12"] = gTd.IsShowing12Hour();
    doc["AMPM"]  = gTd.IsShowingAmPm();
    doc["SEC"]   = gTd.IsShowingSeconds();
    doc["DATE"]  = gTd.IsShowingDate();
    doc["DOW"]   = gTd.IsShowingWkDay();
    doc["TZ"]    = gTd.IsShowingTz();
    doc["TEMP"]  = gTd.IsShowingTemp();
    doc["DEGREES_F"] = gTd.IsShowingDegreesF();
    doc["LDR"]   = gBacklight.IsLdrPresent();
    doc["AUTOBRITE"] = gBacklight.IsLdrUsed();
    if (gBacklight.IsLdrUsed())
    {
        doc["BRIGHTNESS"] = (gBacklight.GetBrightness() * 200) / gBacklight.GetRange();
    }
    else
    {
        doc["BRIGHTNESS"] = (gBacklight.GetBrightness() * 100) / gBacklight.GetRange();
    }

    serializeJson(doc, webPage);
    gServer.send(200, "text/html", webPage);
} // End SendTimeOptsData().


/*******************************************************************************
* UpdateTimeOptsData()
*
* Called when the client has (possibly) changed one or more of the time options.
* Saves the (possibly) new settings for immediate use.
*
* Note: New values are saved for immediate use, but are not saved to NVS.  If
*       values are meant to be used permanently, an NVS save must be performed.
*******************************************************************************/
static void UpdateTimeOptsData()
{
    uint16_t response = 200;    // Assume OK response.
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, gServer.arg("plain"));
    if (error)
    {
        Serial.print("deserializeJson() failed with code ");
        Serial.println(error.c_str());
        response = 400; // BAD REQUEST response.
    }
    else
    {
        // Update the time options.
        gTd.Show12Hour((int)doc["fmt12"]);
        gTd.ShowAmPm((int)doc["amPm"]);
        gTd.ShowSeconds((int)doc["sec"]);
        gTd.ShowDate((int)doc["date"]);
        gTd.ShowWkDay((int)doc["dow"]);
        gTd.ShowTz((int)doc["tz"]);
        gTd.ShowTemp((int)doc["temp"]);
        gTd.ShowDegreesF((int)doc["degrees"]);
        gBacklight.UseLdr((int)doc["auto"]);
        
        // Update the brightness.
        int brightness = (int)doc["brite"];
        brightness *= gBacklight.GetRange();
        brightness /= (gBacklight.IsLdrUsed() ? 200 : 100);
        gBacklight.SetBrightness(brightness);
    }
    // Send a response to the client.
    gServer.send(response, "text/html");
} // End UpdateTimeOptsData().



/*******************************************************************************
********************************************************************************
* COLOR SELECTION PAGE RELATED FUNCTIONS.
********************************************************************************
*******************************************************************************/

/*******************************************************************************
* ShowColorsPage()
*
* Sends the screen color selection page to the client.
*******************************************************************************/
static void ShowColorsPage()
{
    gServer.send(200, "text/html", gClockColorsPage);
} // End ShowColorsPage().


/*******************************************************************************
* SendColorsData()
*
* Called when the client requests a refresh of the color options page data.
* Updates all of the relevant fields of the color options page based on current
* values.
*******************************************************************************/
static void SendColorsData()
{
    String webPage;
    JsonDocument doc;

    // WEB ID
    String http = "http://";
    String id = gNetworkServerName;
    doc["WEB_ID"] = http + id;

    // CURRENT COLOR DATA
    doc["CYCLE"] = gTd.IsColorCycling();
    doc["PERIOD"] = (int32_t)gTd.GetColorCyclePeriod();
    uint16_t priColor = 0;
    uint16_t secColor = 0;
    uint16_t bgColor  = 0;
    gTd.GetColors(priColor, secColor, bgColor);
    char buf[10];
    snprintf(buf, sizeof(buf), "#%06x", (unsigned)ClockHelper::Rgb162Rgb24(priColor));
    doc["PRI_COLOR"] = buf;
    snprintf(buf, sizeof(buf), "#%06x", (unsigned)ClockHelper::Rgb162Rgb24(secColor));
    doc["SEC_COLOR"] = buf;
    snprintf(buf, sizeof(buf), "#%06x", (unsigned)ClockHelper::Rgb162Rgb24(bgColor));
    doc["BG_COLOR"]  = buf;

    serializeJson(doc, webPage);
    gServer.send(200, "text/html", webPage);
} // End SendColorsData().


/*******************************************************************************
* UpdateColorsData()
*
* Called when the client has (possibly) changed one or more of the colors.
* Saves the (possibly) new settings for immediate use.
*
* Note: New values are saved for immediate use, but are not saved to NVS.  If
*       values are meant to be used permanently, an NVS save must be performed.
*******************************************************************************/
static void UpdateColorsData()
{
    uint16_t response = 200;    // Assume OK response.
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, gServer.arg("plain"));
    if (error)
    {
        Serial.print("deserializeJson() failed with code ");
        Serial.println(error.c_str());
        response = 400; // BAD REQUEST response.
    }
    else
    {
        gTd.CycleColors((int)doc["cycle"]);
        gTd.SetColorCyclePeriod((float)((int)doc["period"]));

        char *endptr;
        uint32_t color = strtoul((const char *)doc["bgColor"] + 1, &endptr, 16);
        rgb16_t bgColor = ClockHelper::Rgb242Rgb16(color);

        color = strtoul((const char *)doc["priColor"] + 1, &endptr, 16);
        rgb16_t priColor = ClockHelper::Rgb242Rgb16(color);

        color = strtoul((const char *)doc["secColor"] + 1, &endptr, 16);
        rgb16_t secColor = ClockHelper::Rgb242Rgb16(color);

        gTd.SetColors(priColor, secColor, bgColor);
    }
    // Send a response to the client.
    gServer.send(response, "text/html");
} // End UpdateColorsData().



/*******************************************************************************
********************************************************************************
* FONT SELECTION PAGE RELATED FUNCTIONS.
********************************************************************************
*******************************************************************************/

/*******************************************************************************
* ShowFontsPage()
*
* Sends the font selection page to the client.
*******************************************************************************/
static void ShowFontsPage()
{
    gServer.send(200, "text/html", gFontsPage);
} // End ShowFontsPage().


/*******************************************************************************
* SendFontsData()
*
* Called when the client requests a refresh of the font selection page data.
* Updates all of the relevant fields of the font selection page based on current
* values.
*******************************************************************************/
static void SendFontsData()
{
    String webPage;
    JsonDocument doc;

    // WEB ID
    String http = "http://";
    String id = gNetworkServerName;
    doc["WEB_ID"] = http + id;
    
    // CURRENT FONT DATA
    doc["NUM_MAIN_FONTS_PER_TX"] = gNumMainFontsPerTx;
    doc["NUM_SEC_FONTS_PER_TX"] = gNumSecFontsPerTx;
    doc["NUM_MAIN_FONTS"] = TimeMainFonts.NumFonts();
    doc["NUM_SEC_FONTS"] = TimeSecondaryFonts.NumFonts();
    serializeJson(doc, webPage);
    gServer.send(200, "text/html", webPage);
} // End SendFontsData().


/*******************************************************************************
* SendMainFont()
*
* Called when the client requests the main font list.  The client requests fonts
* starting with a specific index.  This function sends up to gNumMainFontsPerTx
* number of main fonts starting with the requested font.
*******************************************************************************/
static void SendMainFont()
{
    String webPage;

    uint16_t response = 200;    // Assume OK response.
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, gServer.arg("plain"));
    if (error)
    {
        Serial.print("deserializeJson() failed with code ");
        Serial.println(error.c_str());
        response = 400; // BAD REQUEST response.
    }
    else
    {
        // Start with the requested font.
        font_vec_t *pFonts = &PrimaryFonts;
        size_t startIndex = (int)doc["index"];
        // Clear the JSON doc string so it can be reused.
        doc.clear();
        // Start an array of font data.
        JsonArray table = doc["MFTABLE"].to<JsonArray>();
        
        // Send fonts data until the end of the font list, or until 
        // gNumMainFontsPerTx fonts have been sent.
        for (size_t i = 0; (i < gNumMainFontsPerTx) &&
                           (i + startIndex < TimeMainFonts.NumFonts()); i++)
        {
            size_t curIndex = i + startIndex;
            JsonObject obj = table.add<JsonObject>();
            obj["INDEX"]    = curIndex;
            obj["FONTNAME"] = (*pFonts)[curIndex].m_pName;
            obj["CHECKED"]  = (*pFonts)[curIndex].m_Active ? " checked " : " ";
            obj["ICON"]     = (*pFonts)[curIndex].m_Icon;
        }
    }

    serializeJson(doc, webPage);
    gServer.send(response, "text/html", webPage);
} // End SendMainFont().


/*******************************************************************************
* SendSecFont()
*
* Called when the client requests the secondary font list.  The client requests
* fonts starting with a specific index.  This function sends up to 
* gNumSecFontsPerTx number of secondary fonts starting with the requested font.
*******************************************************************************/
static void SendSecFont()
{
    String webPage;

    uint16_t response = 200;    // Assume OK response.
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, gServer.arg("plain"));
    if (error)
    {
        Serial.print("deserializeJson() failed with code ");
        Serial.println(error.c_str());
        response = 400; // BAD REQUEST response.
    }
    else
    {
        // Start with the requested font.
        font_vec_t *pFonts = &SecondaryFonts;
        size_t startIndex = (int)doc["index"];
        // Clear the JSON doc string so it can be reused.
        doc.clear();
        // Start an array of font data.
        JsonArray table = doc["SFTABLE"].to<JsonArray>();
        
        // Send fonts data until the end of the font list, or until 
        // gNumSecFontsPerTx fonts have been sent.
        for (size_t i = 0; (i < gNumSecFontsPerTx) &&
                           (i + startIndex < TimeSecondaryFonts.NumFonts()); i++)
        {
            size_t curIndex = i + startIndex;
            JsonObject obj = table.add<JsonObject>();
            obj["INDEX"]    = curIndex;
            obj["FONTNAME"] = (*pFonts)[curIndex].m_pName;
            obj["CHECKED"]  = (*pFonts)[curIndex].m_Active ? " checked " : " ";
            obj["ICON"]     = (*pFonts)[curIndex].m_Icon;
        }
    }

    serializeJson(doc, webPage);
    gServer.send(response, "text/html", webPage);
} // End SendMainFont().


/*******************************************************************************
* UpdateFontsData()
*
* Called when the client has (possibly) changed one or more of the font
* selections.  Saves the (possibly) new settings for immediate use.
*
* Note: New values are saved for immediate use, but are not saved to NVS.  If
*       values are meant to be used permanently, an NVS save must be performed.
*******************************************************************************/
static void UpdateFontsData()
{
    uint16_t response = 200;    // Assume OK response.
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, gServer.arg("plain"));
    if (error)
    {
        Serial.print("deserializeJson() failed with code ");
        Serial.println(error.c_str());
        response = 400; // BAD REQUEST response.
    }
    else
    {
        // Update secondary font.
        TimeSecondaryFonts.Begin();
        TimeMinorFonts.Begin();

        // Mark all secondary fonts inactive.
        for ( ; !TimeSecondaryFonts.IsEnd(); ++TimeSecondaryFonts, ++TimeMinorFonts)
        {
            TimeSecondaryFonts.SetActive(false);
            TimeMinorFonts.SetActive(false);
        }

        // Set the font that we received to be active.
        int newSecFont = (int)doc["secFont"];
        TimeSecondaryFonts[newSecFont].SetActive(true);
        TimeMinorFonts[newSecFont].SetActive(true);

        // Update main fonts.
        int len = min((int)doc["mainFonts"].size(), (int)TimeMainFonts.NumFonts());
        int16_t currentFont = TimeMainFonts.Index();
        TimeMainFonts.Begin();
        for (int i = 0; i < len; i++)
        {
            TimeMainFonts.SetActive((int)doc["mainFonts"][i]);
            ++TimeMainFonts;
        }
        TimeMainFonts.SetIndex(currentFont);
    }
    // Send a response to the client.
    gServer.send(response, "text/html");
} // End UpdateFontsData().



/*******************************************************************************
********************************************************************************
* TIME ZONE SELECTION PAGE RELATED FUNCTIONS.
********************************************************************************
*******************************************************************************/

/*******************************************************************************
* ShowTzPage()
*
* Sends the time zone selection page to the client.
*******************************************************************************/
static void ShowTzPage()
{
    gServer.send(200, "text/html", gTimezonePage);
} // End ShowTzPage().


/*******************************************************************************
* SendTzData()
*
* Called when the client requests a refresh of the time zone selection page data.
* Updates all of the relevant fields of the time zone selection page based on
* current values.
*******************************************************************************/
static void SendTzData()
{
    String webPage;
    JsonDocument doc;

    // WEB ID
    String http = "http://";
    String id = gNetworkServerName;
    doc["WEB_ID"] = http + id;
    
    // TIME ZONE DATA
    doc["BY_CITY"] = (ClockTz::CurTzSortType == ClockTz::SORT_ALPHA);
    doc["CUR_TZ"] = ClockTz::ActiveTzId;

    serializeJson(doc, webPage);
    gServer.send(200, "text/html", webPage);
} // End SendTzData().


/*******************************************************************************
* SendTzTable()
*
* Called when the client requests the time zone list.  Sends the currently
* active sort type, and the related time zone table.
*******************************************************************************/
static void SendTzTable()
{
    String webPage;
    JsonDocument doc;

    // SORT TYPE
    doc["CUR_TZ"] = ClockTz::ActiveTzId;
    doc["BY_CITY"] = (ClockTz::CurTzSortType == ClockTz::SORT_ALPHA);

    // TIME ZONE TABLE
    JsonArray table = doc["TZTABLE"].to<JsonArray>();
    for (auto &entry : ClockTz::TzTable)
    {
        JsonObject obj = table.add<JsonObject>();
        obj["ID"] = entry.Id();
        obj["LOC"] = entry.Location();
        obj["OFST"] = entry.Offset();
    }

    serializeJson(doc, webPage);
    gServer.send(200, "text/html", webPage);
} // End SendTzTable().


/*******************************************************************************
* UpdateTzSettings()
*
* Called when the client has (possibly) changed one or more of the time zone
* selections.  Saves the (possibly) new settings for immediate use.
*
* Note: New values are saved for immediate use, but are not saved to NVS.  If
*       values are meant to be used permanently, an NVS save must be performed.
*******************************************************************************/
static void UpdateTzSettings()
{
    uint16_t response = 200;    // Assume OK response.
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, gServer.arg("plain"));
    if (error)
    {
        Serial.print("deserializeJson() failed with code ");
        Serial.println(error.c_str());
        response = 400; // BAD REQUEST response.
    }
    else
    {
        ClockTz::TzSortType newSort =
            ((int)doc["sort"] == 1 ? ClockTz::SORT_ALPHA : ClockTz::SORT_NUMERIC);
        if (newSort != ClockTz::CurTzSortType)
        {
            newSort == ClockTz::SORT_ALPHA ? ClockTz::TzSortByLocation() :
                                             ClockTz::TzSortByOffset();
        }
        ClockTz::SetTz((int)doc["active"]);
    }
    // Send a response to the client.
    gServer.send(response, "text/html");
} // End UpdateTzSettings().



/*******************************************************************************
********************************************************************************
* WIFI PAGE RELATED FUNCTIONS.
********************************************************************************
*******************************************************************************/

/*******************************************************************************
* ShowWifiPage()
*
* Sends the WiFi (NTP server) selection page to the client.
*******************************************************************************/
static void ShowWifiPage()
{
    gServer.send(200, "text/html", gWifiPage);
} // End ShowWifiPage().


/*******************************************************************************
* SendWifiData()
*
* Called when the client requests a refresh of the WiFi page data.
* Updates all of the relevant fields of the WiFi (NTP server) page based on
* current values.
*******************************************************************************/
static void SendWifiData()
{
    String webPage;
    JsonDocument doc;

    // WEB ID
    String http = "http://";
    String id = gNetworkServerName;
    doc["WEB_ID"] = http + id;

    // NTP SERVERS
    doc["NTP1"] = ClockWiFi::GetNtpServer1();
    doc["NTP2"] = ClockWiFi::GetNtpServer2();

    serializeJson(doc, webPage);
    gServer.send(200, "text/html", webPage);
} // End SendWifiData().


/*******************************************************************************
* UpdateWifiData()
*
* Called when the client has (possibly) changed one or more of the NTP server
* selections.  Saves the (possibly) new settings for immediate use.
*
* Note: New values are saved for immediate use, but are not saved to NVS.  If
*       values are meant to be used permanently, an NVS save must be performed.
*******************************************************************************/
static void UpdateWifiData()
{
    uint16_t response = 200;    // Assume OK response.
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, gServer.arg("plain"));
    if (error)
    {
        Serial.print("deserializeJson() failed with code ");
        Serial.println(error.c_str());
        response = 400; // BAD REQUEST response.
    }
    else
    {
        ClockWiFi::SetNtpServer1(((String)doc["ntp1"]).c_str());
        ClockWiFi::SetNtpServer2(((String)doc["ntp2"]).c_str());
    }
    // Send a response to the client.
    gServer.send(response, "text/html");
} // End UpdateWifiData().



/*******************************************************************************
********************************************************************************
* SAVE/RESTORE (NVS) PAGE RELATED FUNCTIONS.
********************************************************************************
*******************************************************************************/

/*******************************************************************************
* ShowNvsPage()
*
* Sends the save/restore page to the client.
*******************************************************************************/
static void ShowNvsPage()
{
    gServer.send(200, "text/html", gNvsPage);
} // End ShowNvsPage().


/*******************************************************************************
* SendNvsData()
*
* Called when the client requests a refresh of the NSV page data.
*******************************************************************************/
static void SendNvsData()
{
    String webPage;
    JsonDocument doc;

    // WEB ID
    String http = "http://";
    String id = gNetworkServerName;
    doc["WEB_ID"] = http + id;
    serializeJson(doc, webPage);
    gServer.send(200, "text/html", webPage);
} // End SendNvsData().


/*******************************************************************************
* SaveNvs()
*
* Called when the client requests a save to nvs.  Performs the save, and returns
* the result to the client.
*******************************************************************************/
static void SaveNvs()
{
    ClockNvs &nvs = ClockNvs::Instance();
    String webPage;
    JsonDocument doc;
    doc["SAVE_RESULT"] = nvs.Save();
    serializeJson(doc, webPage);
    gServer.send(200, "text/html", webPage);
} // End SaveNvs().


/*******************************************************************************
* RestoreNvs()
*
* Called when the client requests a restore from nvs.  Performs the restore
* but does not return a result since a re-connection will be involved if
* successful.
*******************************************************************************/
static void RestoreNvs()
{
    ClockNvs &nvs = ClockNvs::Instance();
    String webPage;
    JsonDocument doc;
    serializeJson(doc, webPage);
    gServer.send(200, "text/html", webPage);
    delay(1000);
    nvs.Restore();

} // End RestoreNvs().


/*******************************************************************************
* RestartClock()
*
* Called when the client requests a clock restart.  Performs the restart
* but does not return a result since a re-connection will be involved if
* successful.
*******************************************************************************/
static void RestartClock()
{
    String webPage;
    JsonDocument doc;
    serializeJson(doc, webPage);
    gServer.send(200, "text/html", webPage);

    // Software reset the system.  !! Won't return from this. !!
    delay(1000);
    #define AIRCR_Register (*((volatile uint32_t*)(PPB_BASE + 0x0ED0C)))
    AIRCR_Register = 0x5FA0004;
} // End RestartClock().


/*******************************************************************************
* FactoryReset()
*
* Called when the client requests a factory reset.  Performs the reset
* but does not return a result since a re-connection will be involved if
* successful.
*******************************************************************************/
static void FactoryReset()
{
    ClockNvs &nvs = ClockNvs::Instance();
    String webPage;
    JsonDocument doc;
    serializeJson(doc, webPage);
    gServer.send(200, "text/html", webPage);

    // Factory reset will reset the system.  !! Won't return from this. !!
    delay(1000);
    nvs.FactoryReset();
} // End FactoryReset().



/*******************************************************************************
* HandleNotFound()
*
* Called when the client tries to fetch a page that we don't support.
* Returns a 404 message to the client with info regarding the request.
*******************************************************************************/
static void HandleNotFound()
{
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += gServer.uri();
    message += "\nMethod: ";
    message += (gServer.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += gServer.args();
    message += "\n";

    for (int i = 0; i < gServer.args(); i++)
    {
        message += " " + gServer.argName(i) + ": " + gServer.arg(i) + "\n";
    }

    gServer.send(404, "text/plain", message);
} // End HandleNotFound().


/*******************************************************************************
* InitClockWebServer()
*
* Called at startup to initialize our web server.  Sets up our network server
* name, registers all known message types for handling, then starts the
* web server.  This function is exposed globally.
*******************************************************************************/
void InitClockWebServer()
{
    if (ClockWiFi::WiFiConnected())
    {
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());

        if (MDNS.begin(gNetworkServerName))
        {
            Serial.println("MDNS responder started");
        }

        gServer.on("/", ShowMainPage);
        gServer.on("/getMainPageData", SendMainPageData);

        gServer.on("/timeOpts", ShowTimeOptsPage);
        gServer.on("/getTimeOptsData", SendTimeOptsData);
        gServer.on("/updateTimeOptsData", UpdateTimeOptsData);

        gServer.on("/colors", ShowColorsPage);
        gServer.on("/getColors", SendColorsData);
        gServer.on("/updateColorsData", UpdateColorsData);

        gServer.on("/fonts", ShowFontsPage);
        gServer.on("/getFontsData", SendFontsData);
        gServer.on("/updateFontsData", UpdateFontsData);
        gServer.on("/getMainFont", SendMainFont);
        gServer.on("/getSecFont", SendSecFont);

        gServer.on("/timezone", ShowTzPage);
        gServer.on("/getTzTable", SendTzTable);
        gServer.on("/getTzSettings", SendTzData);
        gServer.on("/updateTzSettings", UpdateTzSettings);

        gServer.on("/wifi", ShowWifiPage);
        gServer.on("/getWifi", SendWifiData);
        gServer.on("/updateWifiData", UpdateWifiData);

        gServer.on("/nvs", ShowNvsPage);
        gServer.on("/getNvs", SendNvsData);
        gServer.on("/doSave", SaveNvs);
        gServer.on("/doRestore", RestoreNvs);
        gServer.on("/doRestart", RestartClock);
        gServer.on("/doFactoryReset", FactoryReset);

        gServer.onNotFound(HandleNotFound);

        gServer.begin();
        Serial.println("HTTP gServer started");
    }
} // End InitClockWebServer().


/*******************************************************************************
* InitClockWebServer()
*
* Called repeatedly in the Arduino loop() to service the web interface.
* This function is exposed globally.
*******************************************************************************/
void HandleClockWebServer()
{
    if (ClockWiFi::WiFiConnected())
    {
        gServer.handleClient();
        MDNS.update();
    }
} // End HandleClockWebServer().

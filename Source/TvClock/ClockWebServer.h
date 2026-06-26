/*******************************************************************************
* ClockWebServer.h
*
* Implements strings containing the web screens available for the clock.
*
* NOTE: These web pages were generated manually by a novice web page designer.
*       There are absolutely better ways to do this stuff.  Please don't use
*       any of this code as a basis for anything useful.
*
* History:
*   26-JUN-2026 JMC
*      - Removed unused JS lines.
*      - Changed Fonts page timeouts from 25ms to 250ms.
*   09-JUN-2026 JMC
*      Optimized current font display.
*   09-JUN-2026 JMC
*      - Added missing font cycling period setting.
*      - Added next font and previous font selection buttons.
*      - Added current font update.
*      - Made border use consistent.
*      - Minor comment fixes.
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

#if !defined  CLOCK_WEB_SERVER_H
#define CLOCK_WEB_SERVER_H


// Declare extern functions for others to use.
extern void InitClockWebServer();
extern void HandleClockWebServer();


// Only create the web page strings if someoune REALLY wants them.
#if defined DEFINE_CLOCK_WEB_PAGES


/*******************************************************************************
* HOME PAGE
*******************************************************************************/
const char gRootPage[] = R"=====(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <title>JMC Clock Settings</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet">
    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/js/bootstrap.bundle.min.js"></script>
  </head>

  <body id="idBody" hidden>

    <div class="p-1 bg-primary text-white text-center">
      <h1>JMC Clock Settings</h1>
      <p id="idWebId">JmcClock</p>
    </div>

    <nav class="navbar nav-tabs navbar-expand-sm border-0">
      <div class="container-fluid mt3">
        <ul class="nav nav-tabs">
          <li class="nav-item">
            <a class="nav-link" id="headMain" href="/">Main</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headTime" href="/timeOpts">Time Screen Options</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headColors" href="/colors">Colors</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headFonts" href="/fonts">Fonts</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headTz" href="/timezone">Time Zone</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headWifi" href="/wifi">WiFi</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headSave" href="/nvs">Save/Restore</a>
          </li>
        </ul>
      </div>
    </nav>

    <!-- UP TIME -->
    <div class="row">
      <div class="container col-md-3 p-3 my-1 bg-dark text-white rounded-4">
        <h7>Up Time:</h7>
        <div class="fs-4 text-center" id="idUptime"></div>
      </div>

      <!-- TEMPERATURE -->
      <div class="container col-md-2 p-3 my-1 bg-dark text-white rounded-4">
        <h7>Temperature:</h7>
        <div class="fs-4 text-center" id="idTemperature"></div>
      </div>

      <!-- IP ADDRESS -->
      <div class="container col-md-3 p-3 my-1 bg-dark text-white rounded-4">
        <h7>IP Address:</h7>
        <div class="fs-4 text-center" id="idIpAddress"></div>
      </div>

      <!-- SIGNAL STRENGTH -->
      <div class="container col-md-2 p-3 my-1 bg-dark text-white rounded-4">
        <h7>Signal Strength:</h7>
        <div class="progress my-3" role="progressbar" style="height:25px; background-color: gray">
          <div class="progress-bar text-bg-dark overflow-visible bg-success active" id="idSignalStrength" style="width:0%;>"</div>
        </div>
      </div>
    </div>
  </body>

  <script>
    // Global variables.
    let msgInProcess = false;       // Used for serializing messsage requests.

    // Initialize globals on page load.
    window.onload = (event) => {
      msgInProcess = false;
      document.getElementById("headMain").classList.add("active");
    }

    // Start the main page.  It will continue on its own.
    (function triggerMainPage() {
      loadDoc("/getMainPageData", updateMainPageData);

      // Use setTimeout rather than setInterval since we don't want run
      // requests to queue up due to long executions of some links.
      setTimeout(triggerMainPage, 1000);
    })();

    // Load a selected url followed by calling a specified function.
    function loadDoc(url, cFunction) {
      if (msgInProcess) {
        setTimeout(loadDoc, 100, url, cFunction);
      }
      else {
        msgInProcess = true;
        let xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4) {
            if (this.status == 200) {
              cFunction(this);
            }
            msgInProcess = false;
          }
        };
        xhttp.open("GET", url, true);
        xhttp.timeout = 2000;
        xhttp.send();
      }
    }

    // Update the main page display.
    function updateMainPageData(xhttp) {
      let json = JSON.parse(xhttp.responseText);

      // WEB ID
      document.getElementById("idWebId").innerText = json.WEB_ID;

      // TEMPERATURE - "\xb0" represents the degrees symbol.
      let tempUnits = " \xb0" + json.TEMPERATURE_UNITS.trim();
      let value = parseFloat(json.TEMPERATURE);
      if (value != "-") {
        if (json.TEMPERATURE_UNITS.trim() == "F") {
          value = value.toFixed(0) + tempUnits;
        }
        else {
          value = value.toFixed(1) + tempUnits;
        }
      }
      document.getElementById("idTemperature").innerText = value;

      // UP TIME
      let seconds = parseInt(json.UPTIME / 1000);
      let minutes = parseInt((seconds / 60) % 60).toString().padStart(2, "0");
      let hours = parseInt(seconds / 3600);
      seconds = (seconds % 60).toString().padStart(2, "0");;
      document.getElementById("idUptime").innerText = hours + ":" + minutes + ":" + seconds;

      // IP ADDRESS
      document.getElementById("idIpAddress").innerText = json.IP_ADDRESS;

      // SIGNAL STRENGTH
      value = json.SIGNAL_STRENGTH;
      let sc = document.getElementById("idSignalStrength");
      let ss = sc.style;
      // Expect strength to be between -20 dBm and -100 dBm
      let signalPercent = Math.min(Math.max(value * 1.25 + 125, 0), 100);
      ss.width = signalPercent + "%";
      // Note: "\xa0" is equivalent to &nbsp.
      sc.innerText = value + "\xa0dBm";
      // document.getElementById("idSignalStrength").innerText = signalPercent + "%";
      sc.classList.remove('bg-success', 'bg-warning', 'bg-danger');

      if (value >= -60) {
        sc.classList.add("bg-success");
      }
      else if (value >= -80) {
        sc.classList.add("bg-warning");
      }
      else {
        sc.classList.add("bg-danger");
      }
      document.getElementById("idBody").hidden = false;
      console.log(json);
    }

  </script>
</html>
)=====";  // End gRootPage[].


/*******************************************************************************
* TIME OPTIONS SCREEN
*******************************************************************************/
const char gTimeScreenOptsPage[] = R"=====(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <title>JMC Clock Settings</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet">
    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/js/bootstrap.bundle.min.js"></script>
  </head>

  <style>
    #idBrite {height: 16px; border-radius: 5px;}
    #idBrite::-webkit-slider-runnable-track { background: transparent;}
  </style>

  <body id="idBody" hidden>

    <div class="p-1 bg-primary text-white text-center">
      <h1>JMC Clock Settings</h1>
      <p id="idWebId">JmcClock</p>
    </div>

    <nav class="navbar nav-tabs navbar-expand-sm border-0">
      <div class="container-fluid mt3">
        <ul class="nav nav-tabs">
          <li class="nav-item">
            <a class="nav-link" id="headMain" href="/">Main</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headTime" href="/timeOpts">Time Screen Options</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headColors" href="/colors">Colors</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headFonts" href="/fonts">Fonts</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headTz" href="/timezone">Time Zone</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headWifi" href="/wifi">WiFi</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headSave" href="/nvs">Save/Restore</a>
          </li>
        </ul>
      </div>
    </nav>

    <div class="container">
      <div class="col-sm-12">
        <fieldset><legend>Time Screen Options</legend></fieldset>
        <div class="container-flex col-sm-6 p-1 my-2 border" onclick="putOptionsData()"><h6>Hour Format</h6>
          <div class="form-check">
            <input type="radio" class="form-check-input" name="timeFmt" id="idFmt12" checked>12 Hour Format
            <label class="form-check-label" for="idFmt12"></label>
          </div>
          <div class="form-check">
            <input type="radio" class="form-check-input" name="timeFmt" id="idFmt24">24 Hour Format
            <label class="form-check-label" for="idFmt24"></label>
          </div>
        </div>
        <div class="container col-sm-6 p-1"></div>

        <div class="container-flex col-sm-6 p-1 my-2 border" onclick="putOptionsData()"><h6>Displayed Items</h6>
          <div class="form-check form-switch">
            <input class="form-check-input" type="checkbox" id="idAmPm">
            <label class="form-check-label" for="idAmPm">Show AM/PM</label>
          </div>
          <div class="form-check form-switch">
            <input class="form-check-input" type="checkbox" id="idSeconds">
            <label class="form-check-label" for="idSeconds">Show Seconds</label>
          </div>
          <div class="form-check form-switch">
            <input class="form-check-input" type="checkbox" id="idDate">
            <label class="form-check-label" for="idDate">Show Date</label>
          </div>
          <div class="form-check form-switch">
            <input class="form-check-input" type="checkbox" id="idDow">
            <label class="form-check-label" for="idDow">Show Day of Week</label>
          </div>
          <div class="form-check form-switch">
            <input class="form-check-input" type="checkbox" id="idTz">
            <label class="form-check-label" for="idTz">Show Time Zone</label>
          </div>
        </div>
        <div class="container col-sm-6 p-1"></div>

        <div class="container-flex col-sm-6 p-1 my-2 border" onclick="putOptionsData()"><h6>Temperature Format</h6>
          <div class="form-check">
            <input type="radio" class="form-check-input" name="tempFmt" id="idTempF" checked>Show Fahrenheit Degrees (F)
            <label class="form-check-label" for="idTempF"></label>
          </div>
          <div class="form-check">
            <input type="radio" class="form-check-input" name="tempFmt" id="idTempC">Show Celsius Degrees (C)
            <label class="form-check-label" for="idTempC"></label>
          </div>
          <div class="form-check">
            <input type="radio" class="form-check-input" name="tempFmt" id="idNoTemp">Don't Show Temperature
            <label class="form-check-label" for="idNoTemp"></label>
          </div>
        </div>
        <div class="container col-sm-6 p-1"></div>

        <div class="container-flex col-sm-6 p-1 my-2 border"><h6>Brightness</h6>
          <div class="form-check form-switch" onclick="updateBrite()" id="idHideAuto">
            <input class="form-check-input" type="checkbox" id="idAutoBrite">
            <label class="form-check-label" for="idAutoBrite">Auto Brightness</label>
          </div>
          <div class="flex-container mt-4 w-50">
            <label for="idBrite" class="form-label" id="idBriteLbl">Auto Brightness Offset</label>
            <input type="range" class="form-range" min="-100" max="100" value="0" id="idBrite">
          </div>
        </div>
      </div>
    </div>
  </body>

  <script>
    // Global variables.
    let msgInProcess = false;       // Used for serializing messsage requests.
    const briteSlider = document.getElementById('idBrite');
    let firstPass = true;
    let hasLdr = false;

    // Initialize globals on page load.
    window.onload = (event) => {
      msgInProcess = false;
      document.getElementById("headTime").classList.add("active");
      briteSlider.addEventListener('input', updateBrite);
    }

    // Start the main page.  It will continue on its own.
    (function triggerMainPage() {
      loadDoc("/getTimeOptsData", updateTimeOptsData);
      setTimeout(triggerMainPage, 5000);
    })();

    // Load a selected url followed by calling a specified function.
    function loadDoc(url, cFunction) {
      if (msgInProcess) {
        setTimeout(loadDoc, 100, url, cFunction);
      }
      else {
        msgInProcess = true;
        let xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4) {
            if (this.status == 200) {
              cFunction(this);
            }
            msgInProcess = false;
          }
        };
        xhttp.open("GET", url, true);
        xhttp.timeout = 2000;
        xhttp.send();
      }
    }

    // Send form data to the server.
    function putFormData(url, data, callback) {
      let xhttp = new XMLHttpRequest();
      xhttp.open("POST", url, true);
      xhttp.setRequestHeader('Content-Type', 'application/json');
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4) {
          if (this.status == 200) {
            console.log(this.responseText);
          }
          if (callback) {
            callback(this);
          }
          msgInProcess = false;
        }
      };
      xhttp.send(JSON.stringify(data));
    }

    function putOptionsData() {
      if (msgInProcess) {
        setTimeout(putOptionsData, 25);
      }
      else {
        msgInProcess = true;

        let fmt12Val   = document.getElementById("idFmt12").checked;
        let sc = document.getElementById("idAmPm");
        sc.disabled = !fmt12Val;
        let amPmVal    = document.getElementById("idAmPm").checked;
        let secVal     = document.getElementById("idSeconds").checked;
        let dateVal    = document.getElementById("idDate").checked;
        let dowVal     = document.getElementById("idDow").checked;
        let tzVal      = document.getElementById("idTz").checked;
        let tempVal    = !document.getElementById("idNoTemp").checked;
        let degreesVal = document.getElementById("idTempF").checked;
        let autoVal    = document.getElementById("idAutoBrite").checked;
        let briteVal   = briteSlider.value;

        let optionsData = {
          fmt12: fmt12Val,
          amPm:  amPmVal,
          sec:   secVal,
          date:  dateVal,
          dow:   dowVal,
          tz:    tzVal,
          temp:  tempVal,
          degrees: degreesVal,
          brite: briteVal,
          auto: autoVal
        };

        putFormData("/updateTimeOptsData", optionsData, null);
      }
    }

    // Update the clock settings page display.
    function updateTimeOptsData(xhttp) {
      let json = JSON.parse(xhttp.responseText);

      // WEB ID
      document.getElementById("idWebId").innerText = json.WEB_ID;

      // Options buttons.
      document.getElementById("idFmt12").checked = json.FMT12;
      document.getElementById("idFmt24").checked = !json.FMT12;
      document.getElementById("idAmPm").disabled = !json. FMT12;
      document.getElementById("idAmPm").checked = json.AMPM;
      document.getElementById("idSeconds").checked = json.SEC;
      document.getElementById("idDate").checked = json.DATE;
      document.getElementById("idDow").checked = json.DOW;
      document.getElementById("idTz").checked = json.TZ;
      if (!json.TEMP) {
        document.getElementById("idNoTemp").checked = true;
      }
      else if (json.DEGREES_F) {
        document.getElementById("idTempF").checked = true;
      }
      else {
        document.getElementById("idTempC").checked = true;
      }
      document.getElementById("idAutoBrite").checked = json.AUTOBRITE;
      hasLdr = json.LDR;

      if (firstPass) { // Only update the brightness on entry.
        briteSlider.value = json.BRIGHTNESS;
        firstPass = false;
      }

      updateBrite();

      document.getElementById("idBody").hidden = false;
      console.log(json);
    }

    function updateBrite() {
      document.getElementById("idHideAuto").style.display = hasLdr ? "block" : "none";
      let useAuto = document.getElementById("idAutoBrite").checked;
      document.getElementById("idBriteLbl").innerHTML = (useAuto ? "Auto Brightness Offset" : "Brightness");
      let min = useAuto ? -100 : 0;
      briteSlider.min = min;
      const max = briteSlider.max || 100;
      const value = briteSlider.value;
      const center = useAuto ? ((max - min) / 2 + parseInt(min)) : 0; // Change to 0 for single ended

      // Calculate percentages for the gradient
      const centerPercent = useAuto ? 50 : 0;   // Change to 0 for single ended
      const currentPercent = ((value - min) / (max - min)) * 100;
      let gradient;
      if (value >= center) {
        // Fill to the right
        gradient = `linear-gradient(to right, #ddd 0%, #ddd ${centerPercent}%, #0d6efd ${centerPercent}%, #0d6efd ${currentPercent}%, #ddd ${currentPercent}%)`;
        } else {
        // Fill to the left
        gradient = `linear-gradient(to right, #ddd 0%, #ddd ${currentPercent}%, #0d6efd ${currentPercent}%, #0d6efd ${centerPercent}%, #ddd ${centerPercent}%)`;
      }

      briteSlider.style.background = gradient;

      putOptionsData();
    }
  </script>
</html>
)=====";  // End gTimeScreenOptsPage[].



/*******************************************************************************
* COLOR SELECTION SCREEN
*******************************************************************************/
const char gClockColorsPage[] = R"=====(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <title>JMC Clock Settings</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet">
    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/js/bootstrap.bundle.min.js"></script>
  </head>
  <body id="idBody" hidden>

    <div class="p-1 bg-primary text-white text-center">
      <h1>JMC Clock Settings</h1>
      <p id="idWebId">JmcClock</p>
    </div>

    <nav class="navbar nav-tabs navbar-expand-sm border-0">
      <div class="container-fluid mt3">
        <ul class="nav nav-tabs">
          <li class="nav-item">
            <a class="nav-link" id="headMain" href="/">Main</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headTime" href="/timeOpts">Time Screen Options</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headColors" href="/colors">Colors</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headFonts" href="/fonts">Fonts</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headTz" href="/timezone">Time Zone</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headWifi" href="/wifi">WiFi</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headSave" href="/nvs">Save/Restore</a>
          </li>
        </ul>
      </div>
    </nav>

    <div class="container">
      <div class="col-sm-12" onclick="putColorsData()">
        <fieldset><legend>Color Options</legend></fieldset>
        <div class="container-flex col-sm-6 p-1 border">
          <div class="form-check">
            <input type="radio" class="form-check-input" name="cycleColor" id="idCycled" checked>Cycle Colors
            <label class="form-check-label" for="idCycled"></label>
          </div>
          <div class="form-check">
            <input type="radio" class="form-check-input" name="cycleColor" id="idFixed">Fixed Colors
            <label class="form-check-label" for="idFixed"></label>
          </div>
        </div>
        <div class="container col-sm-12 p-1 border" id="idPeriodLabel">
          <label for="idPeriod" class="form-label my-3">Cycle Period - Time to complete one color cycle.</label>
          <input type="range" class="form-range w-60" min="10" max="3590" step="10" value="360" id="idPeriod">
          <output for="idPeriod" id="idPeriodValue" aria-hidden="true"></output>
        </div>
      </div>


      <!--
        https://www.geeksforgeeks.org/bootstrap/bootstrap-5-range-steps/
        https://getbootstrap.com/docs/5.3/forms/range/
      -->
      <div class="container mt-4" id="idScreenProto">
        <legend>Screen Colors Check:</legend>
        <div class="container rounded border my-5" style="background-color:rgb(0, 0, 255);" id="idScrBgColor">
          <div class="col-xs-12" style="height:50px;"></div>
          <p style="color: rgb(255, 0, 0);font-size: 32px;text-align:center;font-family: 'Times New Roman';" class="my-3" id="idScrPriColor">PRIMARY TEXT COLOR</p>
          <p style="color: rgb(0, 255, 0);font-size: 20px;text-align:center;font-family: 'Times New Roman';" id="idScrSecColor">SECONDARY TEXT COLOR</p>
          <div class="col-xs-12" style="height:90px;"></div>
        </div>
      </div>

      <div class="container mt-4">
        <legend>Select Screen Colors:</legend>
        <div class="row g-3">
          <datalist id="idColors">
            <option value="#ff0000"></option>
            <option value="#00ff00"></option>
            <option value="#0000ff"></option>
            <option value="#660000"></option>
            <option value="#006600"></option>
            <option value="#000066"></option>
            <option value="#cc0000"></option>
            <option value="#00cc00"></option>
            <option value="#0034af"></option>
            <option value="#ffff00"></option>
            <option value="#00ffff"></option>
            <option value="#ff00ff"></option>
            <option value="#000000"></option>
            <option value="#ffffff"></option>
            <option value="#808080"></option>
          </datalist>

          <!-- Background Color -->
          <div class="col-md-4">
            <label for="idBgColor" class="form-label">Background</label>
            <input type="color" class="form-control form-control-color w-100" id="idBgColor" value="#0d6efd" list="idColors">
          </div>

          <!-- Primary Color -->
          <div class="col-md-4" id="idPriLabel">
            <label for="idPriColor" class="form-label">Primary Text Color</label>
            <input type="color" class="form-control form-control-color w-100" id="idPriColor" value="#198754" list="idColors">
          </div>

          <!-- Secondary Color -->
          <div class="col-md-4" id="idSecLabel">
            <label for="idSecColor" class="form-label">Secondary Text Color</label>
            <input type="color" class="form-control form-control-color w-100" id="idSecColor" value="#ffc107" list="idColors">
          </div>
        </div>
      </div>
    </div>
  </body>

  <script>
    // Global variables.
    let msgInProcess = false;       // Used for serializing messsage requests.
    const periodInput = document.getElementById('idPeriod');
    const periodOutput = document.getElementById('idPeriodValue');
    const bgInput = document.getElementById('idBgColor');
    const priInput = document.getElementById('idPriColor');
    const secInput = document.getElementById('idSecColor');

    // Initialize globals on page load.

    window.onload = (event) => {
      msgInProcess = false;
      document.getElementById("headColors").classList.add("active");
      // Set initial value
      updatePeriod();

      periodInput.addEventListener('input', function() {
        updatePeriod();
        putColorsData();
      });

      bgInput.addEventListener('input', putColorsData);
      priInput.addEventListener('input', putColorsData);
      secInput.addEventListener('input', putColorsData);
    }

    // Start the main page.  It will continue on its own.
    (function triggerMainPage() {
      loadDoc("/getColors", updateColorsData);
      setTimeout(triggerMainPage, 1000);
    })();

    // Load a selected url followed by calling a specified function.
    function loadDoc(url, cFunction) {
      if (msgInProcess) {
        setTimeout(loadDoc, 100, url, cFunction);
      }
      else {
        msgInProcess = true;
        let xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4) {
            if (this.status == 200) {
              cFunction(this);
            }
            msgInProcess = false;
          }
        };
        xhttp.open("GET", url, true);
        xhttp.timeout = 2000;
        xhttp.send();
      }
    }

    // Send form data to the server.
    function putFormData(url, data, callback) {
      let xhttp = new XMLHttpRequest();
      xhttp.open("POST", url, true);
      xhttp.setRequestHeader('Content-Type', 'application/json');
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4) {
          if (this.status == 200) {
            console.log(this.responseText);
          }
          if (callback) {
            callback(this);
          }
          msgInProcess = false;
        }
      };
      xhttp.send(JSON.stringify(data));
    }

    function updateScreen() {
      let cycleColorVal = document.getElementById("idCycled").checked;
      document.getElementById("idPriLabel").hidden = cycleColorVal;
      document.getElementById("idSecLabel").hidden = cycleColorVal;
      document.getElementById("idPeriodLabel").hidden = !cycleColorVal;
      // document.getElementById("idScreenProto").hidden = cycleColorVal;

      let bgColorVal = document.getElementById("idBgColor").value;
      let priColorVal = document.getElementById("idPriColor").value;
      let secColorVal = document.getElementById("idSecColor").value;

      document.getElementById("idScrBgColor").style.backgroundColor = bgColorVal;
      document.getElementById("idScrPriColor").style.color = priColorVal;
      document.getElementById("idScrSecColor").style.color = secColorVal;

      updatePeriod();
    }

    function putColorsData() {
      if (msgInProcess) {
        setTimeout(putColorsData, 25);
      }
      else {
        msgInProcess = true;

        updateScreen();

        let cycleColorVal = document.getElementById("idCycled").checked;
        let cyclePeriodVal = document.getElementById("idPeriod").value;
        let bgColorVal = document.getElementById("idBgColor").value;
        let priColorVal = document.getElementById("idPriColor").value;
        let secColorVal = document.getElementById("idSecColor").value;

        let colorsData = {
          cycle: cycleColorVal,
          period: cyclePeriodVal,
          bgColor: bgColorVal,
          priColor: priColorVal,
          secColor: secColorVal
        };

        putFormData("/updateColorsData", colorsData, null);
      }
    }

    function updatePeriod() {
      let seconds = parseInt(periodInput.value);
      let minutes = parseInt((seconds / 60) % 60).toString().padStart(2, "0");
      seconds = (seconds % 60).toString().padStart(2, "0");
      periodOutput.textContent = "(MM:SS): " + minutes + ":" + seconds;
    }


    function updateColorsData(xhttp) {
      let json = JSON.parse(xhttp.responseText);

      // WEB ID
      document.getElementById("idWebId").innerText = json.WEB_ID;

      // Options buttons.
      document.getElementById("idCycled").checked = json.CYCLE;
      document.getElementById("idFixed").checked = !json.CYCLE;
      document.getElementById("idPeriod").value = json.PERIOD;
      document.getElementById("idPriColor").value = json. PRI_COLOR;
      document.getElementById("idSecColor").value = json.SEC_COLOR;
      document.getElementById("idBgColor").value = json.BG_COLOR;

      updateScreen();
      document.getElementById("idBody").hidden = false;
      console.log(json);
    }

  </script>
</html>
)=====";  // End gClockColorsPage[].


/*******************************************************************************
* TIME ZONE SELECTION SCREEN
*******************************************************************************/
const char gTimezonePage[] = R"=====(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <title>JMC Clock Settings</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet">
    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/js/bootstrap.bundle.min.js"></script>
  </head>
  <body id="idBody" hidden>

    <div class="p-1 bg-primary text-white text-center">
      <h1>JMC Clock Settings</h1>
      <p id="idWebId">JmcClock</p>
    </div>

    <nav class="navbar nav-tabs navbar-expand-sm border-0">
      <div class="container-fluid mt3">
        <ul class="nav nav-tabs">
          <li class="nav-item">
            <a class="nav-link" id="headMain" href="/">Main</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headTime" href="/timeOpts">Time Screen Options</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headColors" href="/colors">Colors</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headFonts" href="/fonts">Fonts</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headTz" href="/timezone">Time Zone</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headWifi" href="/wifi">WiFi</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headSave" href="/nvs">Save/Restore</a>
          </li>
        </ul>
      </div>
    </nav>

    <div class="container">
      <div class="col-sm-12">
        <fieldset><legend>Time Zone Options</legend></fieldset>
        <div class="container-flex col-sm-8 p-1 my-3" onchange="SortTzData()">
          <h5>Select Sort Type</h5>
          <div class="container-flex col-sm-6 p-1 border">
            <div class="form-check">
              <input type="radio" class="form-check-input" name="sort" id="idByCity" checked>Sort by City
              <label class="form-check-label" for="idByCity"></label>
            </div>
            <div class="form-check">
              <input type="radio" class="form-check-input" name="sort" id="idByOffset">Sort by Offset
              <label class="form-check-label" for="idByOffser"></label>
            </div>
          </div>
        </div>
      </div>

      <div class="container col-sm-6 p-1"></div>

      <div class="container-flex col-sm-8 p-1 my-3">
        <h5>Select Location</h5>
        <div class="container-flex col-sm-6" onchange="putTzData()">
          <select class="form-select" id="idTzTable"></select>
        </div>
      </div>
    </div>

  </body>

  <script>
    // Global variables.
    let msgInProcess = false;       // Used for serializing messsage requests.
    const tzTable = [];

    // Initialize globals on page load.
    window.onload = (event) => {
      msgInProcess = false;
      document.getElementById("headTz").classList.add("active");
      loadDoc("/getTzTable", updateTzTable);
    }

    // Start the main page.  It will continue on its own.
    (function triggerMainPage() {
      loadDoc("/getTzSettings", updateTzSettings);
      setTimeout(triggerMainPage, 5000);
    })();

    // Load a selected url followed by calling a specified function.
    function loadDoc(url, cFunction) {
      if (msgInProcess) {
        setTimeout(loadDoc, 100, url, cFunction);
      }
      else {
        msgInProcess = true;
        let xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4) {
            if (this.status == 200) {
              cFunction(this);
            }
            msgInProcess = false;
          }
        };
        xhttp.open("GET", url, true);
        xhttp.timeout = 2000;
        xhttp.send();
      }
    }

    // Send form data to the server.
    function putFormData(url, data, callback) {
      let xhttp = new XMLHttpRequest();
      xhttp.open("POST", url, true);
      xhttp.setRequestHeader('Content-Type', 'application/json');
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4) {
          if (this.status == 200) {
            console.log(this.responseText);
          }
          if (callback) {
            callback(this);
          }
          msgInProcess = false;
        }
      };
      xhttp.send(JSON.stringify(data));
    }

    function putTzData() {
      if (msgInProcess) {
        setTimeout(putTzData, 25);
      }
      else {
        msgInProcess = true;
        let sortType = document.getElementById("idByCity").checked;
        let activeTz = document.getElementById("idTzTable").value;

        let tzData = {
          sort : sortType,
          active : activeTz
        };

        putFormData("/updateTzSettings", tzData, null);
      }
    }

    function TzOfst2Str(ofst) {
      let hours = parseInt(ofst, 10);
      let seconds = Math.abs(parseInt((ofst - hours) * 60, 10));
      let sign = Math.sign(ofst) == 1 ? "+" : "-";
      return "GMT " + sign + " " + String(Math.abs(hours)).padStart(2, '0')  + ":" + String(seconds).padStart(2, '0');
    }

    function SortTzData() {
      putTzData();

      let activeTz = document.getElementById("idTzTable").value;

      if (document.getElementById("idByCity").checked) {
        tzTable.sort((a, b) => a.loc.localeCompare(b.loc));
      }
      else {
        tzTable.sort((a, b) => a.ofst - b.ofst);
      }

      // Remove all options from TZ table.
      document.getElementById("idTzTable").innerHTML = '';

      let i = 0;
      let elements = "";
      for (i = 0; i < tzTable.length; i++) {
        elements += "<option value='" + tzTable[i].id + "'>" + TzOfst2Str(tzTable[i].ofst) + " ===> " + tzTable[i].loc + "</option>";
      }
      document.getElementById("idTzTable").innerHTML = elements;
      document.getElementById("idTzTable").value = activeTz;
    }

    function updateTzTable(xhttp) {
      let json = JSON.parse(xhttp.responseText);

      // Remove all options from TZ table.
      document.getElementById("idTzTable").innerHTML = '';

      let i = 0;
      let id = "";
      let loc = "";
      let ofst = 0.0;
      let elements = "";

      for (i = 0; i < json.TZTABLE.length; i++)  {
        id = json.TZTABLE[i].ID;
        loc = json.TZTABLE[i].LOC;
        ofst = json.TZTABLE[i].OFST;
        tzTable.push({id: id, loc: loc, ofst: ofst});
        elements += "<option value='" + id + "'>" + TzOfst2Str(ofst) + " ===> " + loc + "</option>";
      }
      document.getElementById("idTzTable").innerHTML = elements;

      // Set initially selected option.
      document.getElementById("idTzTable").value = json.CUR_TZ;
      document.getElementById("idByCity").checked = json.BY_CITY;
      document.getElementById("idByOffset").checked = !json.BY_CITY;

      // Options buttons.
      console.log(json);
    }

    // Update the timezone page display.
    function updateTzSettings(xhttp) {
      let json = JSON.parse(xhttp.responseText);

      // WEB ID
      document.getElementById("idWebId").innerText = json.WEB_ID;
      document.getElementById("idTzTable").value = json.CUR_TZ;

      if (document.getElementById("idByCity").checked != json.BY_CITY) {
        document.getElementById("idByCity").checked = json.BY_CITY;
        document.getElementById("idByOffset").checked = !json.BY_CITY;
        SortTzData();
      }

      document.getElementById("idBody").hidden = false;

      // Options buttons.
      console.log(json);
    }

  </script>

</html>
)=====";  // End gTimezonePage[].



/*******************************************************************************
* FONT SELECTION SCREEN
*******************************************************************************/
const char gFontsPage[] = R"=====(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <title>JMC Clock Settings</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet">
    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/js/bootstrap.bundle.min.js"></script>

    <style>
      .checkbox-container {
      display: flex;
      gap: 2px; /* Space between items */
      flex-wrap: wrap; /* Allows wrapping on small screens */
      }
    </style>
  </head>
  <body id="idBody" hidden>

    <div class="p-1 bg-primary text-white text-center">
      <h1>JMC Clock Settings</h1>
      <p id="idWebId">JmcClock</p>
    </div>

    <nav class="navbar nav-tabs navbar-expand-sm border-0">
      <div class="container-fluid mt3">
        <ul class="nav nav-tabs">
          <li class="nav-item">
            <a class="nav-link" id="headMain" href="/">Main</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headTime" href="/timeOpts">Time Screen Options</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headColors" href="/colors">Colors</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headFonts" href="/fonts">Fonts</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headTz" href="/timezone">Time Zone</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headWifi" href="/wifi">WiFi</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headSave" href="/nvs">Save/Restore</a>
          </li>
        </ul>
      </div>
    </nav>

    <div class="container">
      <div class="col-sm-12">
        <fieldset><legend>Font Options</legend></fieldset>
        <div class="container col-sm-6 p-1"></div>
      </div>

      <div class="container-flex col-sm-6 p-1 my-3 border">
        <h5 id="idCurrentFont"></h5>
        <button class="btn btn-primary" onclick="prevFont()">Previous Font</button>
        <button class="btn btn-primary" onclick="nextFont()">Next Font</button>
      </div>

      <div class="container-flex col-sm-12 p-1 my-3 border">
        <h5>Select Main Fonts <span style="font-size:14px;"> (Select At Least One)</span></h5>
        <button class="btn btn-primary" onclick="setAll()">Select All</button>
        <button class="btn btn-primary" onclick="clearAll()">Clear All (But One)</button>
        <div class="checkbox-container container-flex col-sm-12 my-4 mx-1" id="idMainFonts" onclick="putFontData()"></div>
      </div>

      <div class="container-flex col-sm-6 p-1 my-3 border">
        <h5>Select Secondary Fonts<span style="font-size:14px;"> (Select Only One)</span></h5>
        <div class="container-flex col-sm-12 my-4 mx-1" id="idSecFonts" onclick="putFontData()"></div>
      </div>

      <div class="container-flex col-sm-6 p-1 my-3 border">
        <h5>Select Main Font Cycle Period</h5>
        <div class="container-flex col-sm-6" onchange="putFontPeriod()">
          <select class="form-select" id="idFontPeriod">
            <option value='0'>Never</option>
            <option value='1'>Every Second</option>
            <option value='2'>Every Minute</option>
            <option value='3'>Every Hour</option>
            <option value='4'>Every Day</option>
            <option value='5'>Every Week</option>
            <option value='6'>Every Month</option>
            <option value='7'>Every Year</option>
          </select>
        </div>
      </div>
    </div>

  </body>

  <script>
    // Global variables.
    let msgInProcess = false;       // Used for serializing messsage requests.
    let mainFontsLength = 0;
    let mainFontsPerTx = 0;
    let secFontsLength = 0;
    let secFontsPerTx = 0;
    let curFontIndex = 999;

    // Start the main page.  It will continue on its own.
    (function triggerMainPage() {
      msgInProcess = false;
      document.getElementById("headFonts").classList.add("active");
      loadDoc("/getFontsData", updateFontSettings);
      createMainFonts();
      createSecFonts();
      setInterval(refreshCurrentFont, 1000);
    })();

    // Load a selected url followed by calling a specified function.
    async function loadDoc(url, cFunction) {
      if (msgInProcess) {
        setTimeout(loadDoc, 100, url, cFunction);
      }
      else {
        msgInProcess = true;
        let xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4) {
            if (this.status == 200) {
              cFunction(this);
            }
            msgInProcess = false;
          }
        };
        xhttp.open("GET", url, true);
        xhttp.timeout = 2000;
        xhttp.send();
      }
    }

    // Send form data to the server.
    function putFormData(url, data, callback) {
      let xhttp = new XMLHttpRequest();
      xhttp.open("POST", url, true);
      xhttp.setRequestHeader('Content-Type', 'application/json');
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4) {
          if (this.status == 200) {
            console.log(this.responseText);
          }
          if (callback) {
            callback(this);
          }
          msgInProcess = false;
        }
      };
      xhttp.send(JSON.stringify(data));
    }

    async function postData(url, data, callback, rCount) {
      if (++rCount > 3) {
        return;
      }
      try {
        const response = await fetch(url, {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json' // Inform the server that you are sending JSON
          },
          body: JSON.stringify(data) // Convert the JS object to a JSON string
        });
        if (!response.ok) {
          throw new Error(`HTTP error! status: ${response.status}`);
        }

        const result = await response.json(); // Parse the JSON response from the server
        callback(result);
        } catch (error) {
        postData(url, data, callback, rCount);
        console.error('Error:', error);
      }
    }

    function putFontData() {
      if (msgInProcess) {
        setTimeout(putFontData, 250);
      }
      else {
        msgInProcess = true;

        const sfButtons = document.querySelectorAll('input[name="secFont"]');
        let checkedSf;
        for (const sf of sfButtons) {
          if (sf.checked) {
            checkedSf = sf.id.slice(1);
            break
          }
        }

        const mfButtons = document.querySelectorAll('input[type="checkbox"]');
        const checkedMf = [];
        for (const mf of mfButtons) {
          checkedMf[mf.id] = mf.checked ? 1 : 0;
        }

        let fontData = {
          secFont: checkedSf,
          mainFonts : checkedMf
        };

        putFormData("/updateFontsData", fontData, null);
      }
    }

    function createMainFontEntry(json ) {
      let s = "";
      for (let i = 0; (i < mainFontsPerTx); i++) {
        let name = json.MFTABLE[i].FONT_NAME;
        let checked = json.MFTABLE[i].CHECKED;
        let icon = json.MFTABLE[i].ICON;
        let index = json.MFTABLE[i].FONT_INDEX;

        s += '<div class="form-check col-sm-3">'
        s += '<input class="form-check-input" ' + checked + ' type="checkbox" id="' + index + '">';
        s += '<label class="form-check-label" id="L' + index + '"' + 'for="' + index + '"><img src="data:image/png;base64,';
        s += icon + '"/>&nbsp ' + name + '</label></div>';

        if (index >= (mainFontsLength - 1)) {
          break;
        }
      }
      document.getElementById("idMainFonts").innerHTML += s;
    }


    function createSecFontEntry(json ) {
      let s = "";
      for (let i = 0; (i < secFontsPerTx); i++) {
        let name = json.SFTABLE[i].FONT_NAME;
        let checked = json.SFTABLE[i].CHECKED;
        let icon = json.SFTABLE[i].ICON;
        let index = json.SFTABLE[i].FONT_INDEX;

        s += '<div class="form-check col-sm-6">'
        s += '<input class="form-check-input" type="radio" name="secFont" id="s' + index + '" ' + checked + '">';
        s += '<label class="form-checl-label" for="secFont">';
        s += '<img src="data:image/png;base64,' + icon + '"/>&nbsp ' + name + '</label></div>';

        if (index >= (secFontsLength - 1)) {
          break;
        }
      }
      document.getElementById("idSecFonts").innerHTML += s;
    }


    function createMainFonts(xhttp) {
      if (mainFontsLength == 0) {
        setTimeout(createMainFonts, 250);
      }
      // Remove all options from font table.
      document.getElementById("idMainFonts").innerHTML = '';
      for (index = 0; index < mainFontsLength; index += mainFontsPerTx)  {
        let data = {index: index };
        postData("/getMainFont", data, createMainFontEntry, 0);
      }
    }

    function createSecFonts(xhttp) {
      if (secFontsLength == 0) {
        setTimeout(createSecFonts, 250);
      }
      // Remove all options from font table.
      document.getElementById("idSecFonts").innerHTML = '';
      for (index = 0; index < secFontsLength; index += secFontsPerTx)  {
        let data = {index: index };
        postData("/getSecFont", data, createSecFontEntry, 0);
      }
    }

    // Update the clock settings page display.
    function updateFontSettings(xhttp) {
      let json = JSON.parse(xhttp.responseText);

      // WEB ID
      document.getElementById("idWebId").innerText = json.WEB_ID;
      document.getElementById("idFontPeriod").value = json.FONT_PERIOD;
      document.getElementById("idCurrentFont").innerHTML =
        'Current Main Font: <span class="text-danger">' + json.FONT_NAME + '</span>';
      mainFontsPerTx = json.NUM_MAIN_FONTS_PER_TX;
      mainFontsLength = json.NUM_MAIN_FONTS;
      secFontsPerTx = json.NUM_SEC_FONTS_PER_TX;
      // This (secFontsLength) must be set last.
      secFontsLength = json.NUM_SEC_FONTS;
      document.getElementById("idBody").hidden = false;

      // Options buttons.
      console.log(json);
    }

    // Check all main font boxes.
    function setAll() {
      const boxes = document.querySelectorAll('input[type="checkbox"]');

      boxes.forEach((checkbox) => {
        checkbox.checked = true;
      });
      putFontData();
    }

    // Clear all main font boxes except the first one.
    function clearAll() {
      const boxes = document.querySelectorAll('input[type="checkbox"]');

      boxes.forEach((checkbox) => {
        checkbox.checked = false;
        if (checkbox.id == "0") {
          checkbox.checked = true;
        }
      });
      putFontData();
    }

    // Update/refresh the current font.
    function refreshCurrentFont() {
      loadDoc("/refreshCurrentFont", updateCurrentFont);
    }

    // Bump to the previous active main font.
    function prevFont() {
      loadDoc("/setPrevFont", updateCurrentFont);
    }

    // Bump to the next active main font.
    function nextFont() {
      loadDoc("/setNextFont", updateCurrentFont);
    }

    // Show the currently displayed font in red.
    function SetCurrentFont(newIndex) {
      if (curFontIndex != newIndex) {
        curFontIndex = newIndex;
        const labels = document.getElementsByClassName('form-check-label');
        for (const l of labels) {
          if (l.id == "L" + curFontIndex) {
            l.style.color = "red";
          }
          else {
            l.style.color = "black";
            }
        }
      }
    }

    // Update the current font.
    function updateCurrentFont(xhttp) {
      let json = JSON.parse(xhttp.responseText);

      // WEB ID
      document.getElementById("idCurrentFont").innerHTML =
        'Current Main Font: <span class="text-danger">' + json.FONT_NAME + '</span>';
      let newFontIndex = json.FONT_INDEX;
      SetCurrentFont(newFontIndex);

      // Options buttons.
      console.log(json);
    }

    function putFontPeriod() {
      if (msgInProcess) {
        setTimeout(putFontPeriod, 250);
      }
      else {
        msgInProcess = true;

        let period = document.getElementById("idFontPeriod").value;

        let optionsData = {
          fontPeriod : period
        };

        putFormData("/updateFontPeriod", optionsData, null);
      }
    }

  </script>

</html>
)=====";  // End gFontsPage[].



/*******************************************************************************
* WIFI NTP SERVER SELECTION SCREEN
*******************************************************************************/
const char gWifiPage[] = R"=====(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <title>JMC Clock Settings</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet">
    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/js/bootstrap.bundle.min.js"></script>
  </head>

  <body id="idBody" hidden>

    <div class="p-1 bg-primary text-white text-center">
      <h1>JMC Clock Settings</h1>
      <p id="idWebId">JmcClock</p>
    </div>

    <nav class="navbar nav-tabs navbar-expand-sm border-0">
      <div class="container-fluid mt3">
        <ul class="nav nav-tabs">
          <li class="nav-item">
            <a class="nav-link" id="headMain" href="/">Main</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headTime" href="/timeOpts">Time Screen Options</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headColors" href="/colors">Colors</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headFonts" href="/fonts">Fonts</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headTz" href="/timezone">Time Zone</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headWifi" href="/wifi">WiFi</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headSave" href="/nvs">Save/Restore</a>
          </li>
        </ul>
      </div>
    </nav>

    <div class="container">
      <div class="col-sm-12" onclick="putWifiData()">
        <fieldset>
          <legend>Wifi Options</legend>
        </fieldset>
        <form>
          <div class="mb-3 mt-3">
            <label for="idNtp1">NTP Server 1:</label>
            <input type="text" class="form-control" id="idNtp1" placeholder="pool.ntp.org">
          </div>
          <div class="mb-3">
            <label for="idNtp2">NTP Server 2:</label>
            <input type="text" class="form-control" id="idNtp2" placeholder="time.nist.gov">
          </div>
          <button class="btn btn-primary" onclick="putWifiData()">Use</button>
        </form>
      </div>
    </div>

  </body>

  <script>
    // Global variables.
    let msgInProcess = false;       // Used for serializing messsage requests.

    // Initialize globals on page load.
    window.onload = (event) => {
      msgInProcess = false;
      document.getElementById("headWifi").classList.add("active");
    }

    // Start the main page.  It will continue on its own.
    (function triggerMainPage() {
      loadDoc("/getWifi", updateWifiData);
    })();

    // Load a selected url followed by calling a specified function.
    function loadDoc(url, cFunction) {
      if (msgInProcess) {
        setTimeout(loadDoc, 100, url, cFunction);
      }
      else {
        msgInProcess = true;
        let xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4) {
            if (this.status == 200) {
              cFunction(this);
            }
            msgInProcess = false;
          }
        };
        xhttp.open("GET", url, true);
        xhttp.timeout = 2000;
        xhttp.send();
      }
    }

    // Send form data to the server.
    function putFormData(url, data, callback) {
      let xhttp = new XMLHttpRequest();
      xhttp.open("POST", url, true);
      xhttp.setRequestHeader('Content-Type', 'application/json');
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4) {
          if (this.status == 200) {
            console.log(this.responseText);
          }
          if (callback) {
            callback(this);
          }
          msgInProcess = false;
        }
      };
      xhttp.send(JSON.stringify(data));
    }

    function putWifiData() {
      if (msgInProcess) {
        setTimeout(putWifiData, 25);
      }
      else {
        msgInProcess = true;

        let n1 = document.getElementById("idNtp1").value;
        let n2 = document.getElementById("idNtp2").value;

        let wifiData = {
          ntp1: n1,
          ntp2: n2
        };

        putFormData("/updateWifiData", wifiData, null);
        return false;
      }
    }

    // Update the main page display.
    function updateWifiData(xhttp) {
      let json = JSON.parse(xhttp.responseText);

      // WEB ID
      document.getElementById("idWebId").innerText = json.WEB_ID;

      document.getElementById("idNtp1").value = json.NTP1;
      document.getElementById("idNtp2").value = json.NTP2;

      document.getElementById("idBody").hidden = false;
      console.log(json);
    }

  </script>

</html>
)=====";  // End gWifiPage[].




/*******************************************************************************
* SAVE/RESTORE SCREEN
*******************************************************************************/
const char gNvsPage[] = R"=====(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <title>JMC Clock Settings</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet">
    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/js/bootstrap.bundle.min.js"></script>
  </head>

  <body id="idBody" hidden>

    <div class="p-1 bg-primary text-white text-center">
      <h1>JMC Clock Settings</h1>
      <p id="idWebId">JmcClock</p>
    </div>

    <nav class="navbar nav-tabs navbar-expand-sm border-0">
      <div class="container-fluid mt3">
        <ul class="nav nav-tabs">
          <li class="nav-item">
            <a class="nav-link" id="headMain" href="/">Main</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headTime" href="/timeOpts">Time Screen Options</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headColors" href="/colors">Colors</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headFonts" href="/fonts">Fonts</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headTz" href="/timezone">Time Zone</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headWifi" href="/wifi">WiFi</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" id="headSave" href="/nvs">Save/Restore</a>
          </li>
        </ul>
      </div>
    </nav>

    <div class="container">
      <div class="col-sm-12">
        <fieldset><legend>Save/Restore Options</legend></fieldset>

        <button type="button" class="btn btn-primary" onclick="doSave()">Save Current Settings</button>
        <button type="button" class="btn btn-primary" onclick="doRestore()"> Restore Saved Settings</button>
        <button type="button" class="btn btn-primary" onclick="doRestart()">Restart Clock</button>
        <button type="button" class="btn btn-danger" data-bs-toggle="modal" data-bs-target="#idFacReset">Factory Reset</button>
      </div>
    </div>

    <!-- The Modal -->
    <div class="modal" id="idFacReset">
      <div class="modal-dialog modal-dialog-centered">
        <div class="modal-content text-bg-warning">

          <!-- Modal Header -->
          <div class="modal-header">
            <h4 class="modal-title">!!! Factory Reset !!!</h4>
            <button type="button" class="btn-close" data-bs-dismiss="modal"></button>
          </div>

          <!-- Modal body -->
          <div class="modal-body">
            Warning: Factory reset will replace all current clock settings with the initial factory default values.
          </div>

          <!-- Modal footer -->
          <div class="modal-footer">
            <button type="button" class="btn btn-success" data-bs-dismiss="modal">Cancel</button>
            <button type="button" class="btn btn-danger" data-bs-dismiss="modal" onclick="doFactoryReset()">Do Factory Reset</button>
          </div>

        </div>
      </div>
    </div>

    <div class = "toast-container top-40 start-50 translate-middle">
      <div id="idToast" class="toast text-white">
        <div class="toast-header">
          <strong class="me-auto" id="idToastHeader"></strong>
          <button type="button" class="btn-close" data-bs-dismiss="toast"></button>
        </div>
        <div class="toast-body" id="idToastBody">
          <p id="idToastText"></p>
        </div>
      </div>
    </div>
  </div>

</body>

<script>
  // Global variables.
  let msgInProcess = false;       // Used for serializing messsage requests.

  // Initialize globals on page load.
  window.onload = (event) => {
    msgInProcess = false;
    document.getElementById("headSave").classList.add("active");
  }


  // Start the main page.  It will continue on its own.
  (function triggerMainPage() {
    loadDoc("/getNvs", updateNvsData);
  })();


  // Load a selected url followed by calling a specified function.
  function loadDoc(url, cFunction) {
    if (msgInProcess) {
      setTimeout(loadDoc, 100, url, cFunction);
    }
    else {
      msgInProcess = true;
      let xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4) {
          if (this.status == 200) {
            cFunction(this);
          }
          msgInProcess = false;
        }
      };
      xhttp.open("GET", url, true);
      xhttp.timeout = 2000;
      xhttp.send();
    }
  }

  // Send form data to the server.
  function putFormData(url, data, callback) {
    let xhttp = new XMLHttpRequest();
    xhttp.open("POST", url, true);
    xhttp.setRequestHeader('Content-Type', 'application/json');
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4) {
        if (this.status == 200) {
          console.log(this.responseText);
        }
        if (callback) {
          callback(this);
        }
        msgInProcess = false;
      }
    };
    xhttp.send(JSON.stringify(data));
  }

  // Update the main page display.
  function updateNvsData(xhttp) {
    let json = JSON.parse(xhttp.responseText);

    // WEB ID
    document.getElementById("idWebId").innerText = json.WEB_ID;

    document.getElementById("idBody").hidden = false;
    console.log(json);
  }

  function reloadMainPage(delay) {
    setTimeout(function() {
      window.location.href ="/";
    }, delay);
  }

  function ShowStatus(heading, bodytext, bgcolor) {
    const toastDiv = document.getElementById("idToast");
    const toastHdr = document.getElementById("idToastHeader");
    const toastBody = document.getElementById("idToastBody");
    const toastText = document.getElementById("idToastText");
    toastHdr.innerHTML = heading;
    toastDiv.classList.add(bgcolor);
    toastText.innerHTML = bodytext;
    const toastInst = bootstrap.Toast.getOrCreateInstance(toastDiv);
    toastInst.show();
  }

  function doSave() {
    loadDoc("/doSave", checkSaveStatus);
  }

  function checkSaveStatus(xhttp) {
    let json = JSON.parse(xhttp.responseText);
    const heading = "Save Status";
    if (json.SAVE_RESULT == true) {
      ShowStatus(heading, "Save succeeded.", "text-bg-success");
    }
    else {
      ShowStatus(heading, "Save failed.", "text-bg-danger");
    }
  }

  function doRestore() {
    loadDoc("/doRestore", checkRestoreStatus);
  }

  function checkRestoreStatus(xhttp) {
    reloadMainPage(2000);
  }

  function doRestart() {
    loadDoc("/doRestart", checkRestartStatus);
  }

  function checkRestartStatus(xhttp) {
    reloadMainPage(2000);
  }

  function doFactoryReset() {
    loadDoc("/doFactoryReset", checkFRStatus);
  }

  function checkFRStatus(xhttp) {
    reloadMainPage(2000);
  }

  </script>

</html>
)=====";  // End gNvsPage[].


#endif // DEFINE_CLOCK_WEB_PAGES

#endif // CLOCK_WEB_SERVER_H

const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<style>
.card{
    max-width: 700px;
     min-height: 150px;
     background: #808080;
     padding: 10px;
     box-sizing: border-box;
     color: #FFF;
     margin:10px;
     box-shadow: 0px 2px 18px -4px rgba(0,0,0,0.75);
}
</style>
<body>

<div class="card">
  <h4>ESP32 Retro Weather Station </h4><br>
  <h1>Last Value Update:<span id="TimeValue">0</span></h1><br>
  <h1>Temp inside:<span id="TempValue">0</span>&#176C</h1><br>
  <h1>Air pressure inside:<span id="PRESValue">0</span>hPa</h1><br>
  <h1>Temp BMP outside:<span id="TempBMValue">0</span>&#176C</h1><br>
  <h1>Temp DS outside:<span id="TempDSValue">0</span>&#176C</h1><br>
  <h1>Air pressure outside:<span id="PRESEValue">0</span>hPa</h1><br>
  <h1>Light Intensity:<span id="LUXValue">0</span>lux</h1><br>
  <h1>Battery Voltage:<span id="BATValue">0</span>V</h1><br>
</div>
<script>

setInterval(function() {
  // Call a function repetatively with 10 Second interval
  getData0();
  getData1();
  getData2();
  getData3();
  getData4();
  getData5();
  getData6();
  getData7();
}, 10000); //10Seconds update rate

function getData0() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("PRESValue").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "readPRES", true);
  xhttp.send();
}

function getData1() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("TempValue").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "readTMP", true);
  xhttp.send();
}

function getData2() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("TimeValue").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "readTime", true);
  xhttp.send();
}

function getData3() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("TempBMValue").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "readTMPBM", true);
  xhttp.send();
}

function getData4() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("TempDSValue").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "readTMPDS", true);
  xhttp.send();
}

function getData5() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("PRESEValue").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "readPRESE", true);
  xhttp.send();
}

function getData6() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("BATValue").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "readBAT", true);
  xhttp.send();
}

function getData7() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("LUXValue").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "readLUX", true);
  xhttp.send();
}
</script>
</body>
</html>
)=====";

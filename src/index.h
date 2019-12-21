#ifndef INDEX_H
#define INDEX_H

#ifndef ARDUINO_H
#define ARDUINO_H
#include <Arduino.h>
#endif

const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<style>
</style>
<body>
 
<h2>SAE Baja Marquette DAQ Server</h2>
<div>
    <form action="" method="post">
        <ul id="fileNames">
        </ul>
    </form>
</div>
<button type="button" onclick="loadXMLDoc()">Change Content</button>
<script>
 
function loadXMLDoc() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("fileNames").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "getNewFiles", true);
  xhttp.send();
}
</script>
</body>
</html>
)=====";

#endif
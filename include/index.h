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
<body onload="loadXMLDoc()">
 
<h2>SAE Baja Marquette DAQ Server</h2>
<div>
  <form method="POST">
    <label for="file_submit">Choose file:</label> 
      <select name="files_submit" id="file_chooser"> 
      </select>
    <button type="submit" formaction="/download"> Download </button>
    <button type="submit" formaction="/delete"> Delete </button>
  </form>
</div>
<button id="getFileBut" type="button" onclick="loadXMLDoc()">Update Files</button>
<div id="deleteAllDiv">
  <button id="deleteAllBut" type="button" onclick="deleteAll()"> Delete All </button>
</div>

<script>
 
function loadXMLDoc() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("file_chooser").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "getNewFiles", true);
  xhttp.send();
}

function deleteAll() {
  var xhttp = new XMLHttpRequest();
  document.getElementById("deleteAllBut").disabled = true;

  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("deleteAllDiv").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "deleteAllFiles", true);
  xhttp.send();
}

</script>
</body>
</html>
)=====";

#endif
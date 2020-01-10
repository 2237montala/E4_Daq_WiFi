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
<body onload="refreshFiles()">
 
<h2>SAE Baja Marquette DAQ Server</h2>
<div>
  <form method="POST">
    <label for="file_submit">Choose file:</label> 
      <select name="files_submit" id="file_chooser"> 
      </select>
    <button id="downloadBut" type="submit" formaction="/download"> Download </button>
    
  </form>
  <button id="deleteBut" onClick="deleteOneFile()" > Delete </button>
</div>
<div>
  <button id="getFileBut" type="button" onclick="refreshFiles()">Update Files</button>
</div>

<div id="deleteAllDiv">
  <button id="deleteAllBut" type="button" onclick="deleteAll()"> Delete All </button>
</div>

<script>
  function disableButtons() {
    document.getElementById("deleteAllBut").disabled = true;
    document.getElementById("deleteBut").disabled = true;
    document.getElementById("downloadBut").disabled = true;
    document.getElementById("getFileBut").disabled = true;
  }

  function enableButtons() {
    document.getElementById("deleteAllBut").disabled = false;
    document.getElementById("deleteBut").disabled = false;
    document.getElementById("downloadBut").disabled = false;
    document.getElementById("getFileBut").disabled = false;
  }

  function refreshFiles() {
    disableButtons();
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        document.getElementById("file_chooser").innerHTML =
        this.responseText;
        enableButtons();
      }
    };
    xhttp.open("GET", "getNewFiles", true);
    xhttp.send();
    
  }

  function deleteAll() {
    var xhttp = new XMLHttpRequest();
    disableButtons();

    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        document.getElementById("deleteAllDiv").innerHTML =
        this.responseText;
        enableButtons();
      }
    };
    xhttp.open("GET", "deleteAllFiles", true);
    xhttp.send();
    
  }

  function deleteOneFile() {
    var e = document.getElementById("file_chooser");
    var fileSelected = e.options[e.selectedIndex].value;

    var xhr = new XMLHttpRequest();
    xhr.open("POST", "/delete", true);
    xhr.setRequestHeader('Content-Type', 'application/json');
    xhr.send(JSON.stringify({ files_submit: fileSelected }));

    disableButtons();
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        enableButtons();
      }
    };
    xhttp.send();

  }

  function transferFile() {
    
  }



</script>
</body>
</html>
)=====";

#endif
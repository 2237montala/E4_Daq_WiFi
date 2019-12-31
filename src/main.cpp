#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "SerialCmds.h"
#include "index.h"
#include "main.h"

// Replsace with your network credentials
const char* ssid     = "MU DAQ Server";
const char* password = "123456789";

// Set web server port number to 80
WebServer server(80);

// Variable to store the HTTP request
String header;

//Array to hold file names
#define MAX_FILES 100
int currNumFiles = 0;
String fileName[100] = {""};
#define transferLED 5
#define transferLEDTwo 18



void handleRoot() {
 String s = MAIN_page; //Read HTML contents
 server.send(200, "text/html", s); //Send web page
 getNewFiles();
}

void getNewFiles() {
  String response;
  transferFileNames(); //Get files names from the host microcontroller
  if(currNumFiles == 0)
  {
      response += "<option value=\"No_File\"> No Files </option>";
  }
  else
  {
    for(int i = 0; i < currNumFiles; i++)
    {
      //value is the value returned when selected
      response += "<option value=\"" + fileName[i] + "\">" + fileName[i] + "</option>";
    }
  }
  server.send(200, "text/plane", response); //selection values
}

void sendFile(){
  String fileName = "";
  if (server.args() > 0 ) {
    for ( uint8_t i = 0; i < server.args(); i++ ) {
      if (server.argName(i) == "files_submit") {
         fileName = server.arg(i);
      }
    }
  }
  transferFileData(fileName);
}

bool getCMD(String& incomingCmd,int timeout) {
  uint32_t startTime = millis();
  while(millis() - startTime < timeout)
  {
    if(Serial2.available() > 0)
    {
      incomingCmd = Serial2.readStringUntil(EOL);
      return true;
    }
  }
  return false;
}

bool waitForACK(int timeout) {
  uint32_t startTime = millis();
  while((millis() - startTime < 5000))
  {
    if(Serial2.available()>0)
    {
      if(Serial2.readStringUntil(EOL).compareTo(ACK) == 0)
      {
        return true;
      }
    }
  }
  return false;
}

void sendCmd(String cmd,boolean addEOL=true,bool printCMD = true) {
  Serial2.print(cmd);
  if(printCMD)
    Serial.println(cmd);
  if(addEOL) {
    Serial2.print(EOL);
  }
}

void transferFileNames() {
  String cmd;

  sendCmd(FNAME); //Send string to signify request of file names
  waitForACK(5000);

  if(getCMD(cmd,1000),cmd.compareTo(RDY)==0)
  {
    Serial.println("RDY recieved");
    digitalWrite(transferLED,HIGH);
    sendCmd(ACK);
    int fileCount=0;
    if(getCMD(cmd,1000) && cmd.compareTo(FNAME) == 0)
    {
      //Keep reading until the host stops
      boolean moreFiles = true;
    
      while(moreFiles)
      {
        getCMD(cmd,1000);
        if(cmd.compareTo(END) == 0) {
          //Host stops transfer
          Serial.println("END");
          moreFiles = false;
        }
        else
        {
          //Add file name to an array to be referenced later
          fileName[fileCount] = cmd;
          fileCount++;
          Serial.println(cmd);
          sendCmd(ACK,true);
        }
      }
      currNumFiles = fileCount;
    }
  }
  else
  {
    Serial.println("Nothing connected");
  }
}

void transferFileData(String fileName) {
  sendCmd(FDATA,true); //Send command to start data transfer
  if(!waitForACK(5000)){
    Serial.println("No response");
    return;
  }

  sendCmd(fileName,true); //Send name of requested file
  String incomingCmd="";
  if(getCMD(incomingCmd,1000) && incomingCmd.compareTo(RDY) == 0)
  {
    //sender is ready to send file data
    sendCmd(ACK);

    //Get file size for HTTP header
    if(!getCMD(incomingCmd,1000))
    {
      Serial.println("File size not recived");
      return;
    };
    Serial.println((incomingCmd)); //Print file size in bytes
    sendCmd(ACK);

    //Send http header
    fileName.replace(".bin",".csv");
    server.sendHeader("Content-Length", incomingCmd); //Need to include length of file
    server.setContentLength(incomingCmd.toInt());
    server.sendHeader("Content-Disposition", "attachment; fileName="+fileName); //Tell website what kind of data it is
    server.send(200, "text/plain", "");

    //Send each line until EOF is reached
    while(getCMD(incomingCmd,1000) && incomingCmd.compareTo(END) != 0)
    {
      server.sendContent(incomingCmd);
      sendCmd(ACK,true,false);
    }
    Serial.println("Transfer Over\n");
  }
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200);

  pinMode(LED_BUILTIN,OUTPUT);
  pinMode(transferLED,OUTPUT);
  pinMode(transferLEDTwo,OUTPUT);
  digitalWrite(transferLED,LOW);
  digitalWrite(transferLEDTwo,LOW);
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Setting AP (Access Point)… ");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.mode(WIFI_AP); //Access Point mode
  WiFi.softAP(ssid, password);

  //Get ip address of new hot spot
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  //Sets what functions are called when the website is a certain pages
  server.on("/",handleRoot);
  server.on("/download",sendFile);
  server.on("/getNewFiles", getNewFiles);

  server.begin();
  Serial.println("Server started");

  //Try to connect with master microcontroller
  boolean connected = false;
  boolean ledState = true;
  uint32_t startTime = millis();
  while((millis() - startTime < 5000) && !connected) {
    //Send RDY command over serial
    sendCmd(RDY);
    delay(100);

    if(Serial2.available() > 0 && Serial2.readStringUntil(EOL).compareTo(ACK) == 0)
    {
      //Other micro acknowledged state
      Serial.println("Recieved something");
      sendCmd(ACK,true);
      if(waitForACK(1000))
      {
        connected = true;
        digitalWrite(LED_BUILTIN,HIGH);
        Serial.println("\nConnected to other microcontroller");
      }
    }
    else {
      if (ledState == LOW) {
        ledState = HIGH;  
      } else {
        ledState = LOW;
      }
      // set the LED with the ledState of the variable:
      digitalWrite(LED_BUILTIN, ledState);
    }
  }

  if(!connected)
  {
    Serial.println("Not connected");
    digitalWrite(LED_BUILTIN,LOW);
  }

  //Serial2.flush(); //Clear all commands incase extra were sent
  //Get file names from 
  if(connected)
  {
  }
}

void loop() {
  server.handleClient();
  delay(1);
}


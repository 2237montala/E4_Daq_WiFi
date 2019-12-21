#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
//#include <AsyncTCP.h>
//#include "ESPAsyncWebServer.h"
#include "SerialCmds.h"
#include "index.h"

// Replsace with your network credentials
const char* ssid     = "ESP32-Access-Point";
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
}

void getNewFiles() {
  String response;
  if(currNumFiles == 0)
  {
    response = "<li> No files </li>";
  }
  else
  {
    for(int i = 0; i < currNumFiles; i++)
    {
      response += "<li>" + fileName[i];
      response += "<input type=\"submit\" name=\"download\" value=\"download\"/>";
      response += "</li>";
    }
  }
  server.send(200, "text/plane", response); //Send ADC value only to client ajax request
}

void sendFile(){
  digitalWrite(transferLED,HIGH);
}

boolean getCMD(String& incomingCmd,int timeout) {
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

boolean waitForACK(int timeout) {
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

void sendCmd(String cmd,boolean addEOL=true) {
  Serial2.print(cmd);
  Serial.println(cmd);
  if(addEOL) {
    Serial2.print(EOL);
  }
}

void transferFileNames() {
  String cmd;
  bool status = getCMD(cmd,1000);
  digitalWrite(transferLEDTwo,HIGH);
  if(status)
  {
    Serial.println("Got command");
    digitalWrite(transferLEDTwo,LOW); 
    Serial.println(cmd);
    if(cmd.compareTo(RDY)==0)
    {
      Serial.println("RDY recieved");
      digitalWrite(transferLED,HIGH);
      sendCmd(ACK);
      status = getCMD(cmd,1000);
      int fileCount=0;
      if(status && cmd.compareTo(FNAME) == 0)
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
            fileName[fileCount] = cmd;
            fileCount++;
            Serial.println(cmd);
            sendCmd(ACK,true);
          }
        }
        currNumFiles = fileCount;
      }
    }
  }
  else
  {
    Serial.println("Nothing connected");
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
    delay(100);
    Serial.println("Transfering files names");
    transferFileNames();
    Serial.println(currNumFiles);
  }
}

void loop() {
  server.handleClient();
  delay(1);
}


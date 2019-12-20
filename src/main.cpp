#include <Arduino.h>
#include <WiFi.h>
#include "SerialCmds.h"

// Replace with your network credentials
const char* ssid     = "ESP32-Access-Point";
const char* password = "123456789";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

//Array to hold file names
#define MAX_FILES 100
int currNumFiles = 0;
String fileName[100] = {""};
#define transferLED 5
#define transferLEDTwo 18

boolean getCMD(String& incomingCmd,int timeout)
{
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

void sendCmd(String cmd,boolean addEOL=true)
{
  Serial2.print(cmd);
  Serial.println(cmd);
  if(addEOL) {
    Serial2.print(EOL);
  }
}

void transferFileNames()
{
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
  WiFi.softAP(ssid, password);

  //Get ip address of new hot spot
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  server.begin();

  //Try to connect with master microcontroller
  boolean connected = false;
  boolean ledState = true;
  uint32_t startTime = millis();
  while((millis() - startTime < 5000) && !connected)
  {
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
  delay(100);
  //Get file names from 
  if(connected)
  {
    Serial.println("Transfering files names");
    transferFileNames();
    Serial.println(currNumFiles);
  }
  
}

void loop() {
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>Marquette Baja DAQ Server</h1>");
            
            if(currNumFiles == 0)
            {
              client.println("<body><h1>No Files</h1>");
            }
            else
            {
              for(int i = 0; i < currNumFiles;i++)
              {
                client.print("<body><h1>");
                client.print(fileName[i]);
                client.println("</h1>");
              }
            }
            

            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}


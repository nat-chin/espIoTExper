/*WiFiAccessPoint.ino creates a WiFi access point and provides a web server on it.
  Steps:
  1. Connect to the access point "yourAp"
  2. Point your web browser to http://192.168.4.1/H to turn the LED on or http://192.168.4.1/L to turn it off
     OR
     Run raw TCP "GET /H" and "GET /L" on PuTTY terminal with 192.168.4.1 as IP address and 80 as port

  Created for arduino-esp32 on 04 July, 2018
  by Elochukwu Ifediora (fedy0) */

#include "Arduino.h"
#include <WiFi.h>
#include <HTTPClient.h>
// main library to connecting and configuring WiFi connection
// #include <WiFiClient.h>
// provide methods for creating TCP client 
// #include <WiFiAP.h>
// For configuring ESP32 as AP, which WiFi alone can't do this 

/*--------------- Actually we can just include WiFi.h or only the latter 3 + wifi server libs (to save flash memeory and SRAM)*/

/* Strange that WiFi.h already include both WiFiClient , and WiFiAP in its header file , so I expect that WiFi.h alone has all functionality*/

// Set these to your desired credentials.
const char *ssid = "yourAP";
const char *password = "yourPassword123";

WiFiServer server(80);
// instantiate WiFi server on port 80 , this port is TCP port on transport layer
// I think think this one is from WiFi.h but will only be defined after we include WiFiClient as well

// Brownout issue
#include "soc/soc.h"
#include "soc/rtc.h"

void setup() {
  // Set the control register , to disavle brownout detector.
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.println();
  Serial.println("Configuring access point...");

  // config esp32 as AP by the function inside the if condition , if connect fail , hold connection until success
  if (!WiFi.softAP(ssid, password)) {
    log_e("Soft AP creation failed.");
    while(1);
  }
  // Can remove the password parameter if you want the AP to be open.

  IPAddress myIP = WiFi.softAPIP();
  // I don't know about this C++ OOP notation , but it seems that myIP variable of type class IPAddress , but somehow output string with no problem on serial monitor
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();
  // Start a webserver hosted by ESP32 at TCP port 80 , now we can use our device to connect by using myIP ip address of SAP , hop through the network to port 80
  // the way it is coded is similar to Hardware Serial object

  Serial.println("Server started");
}

void loop() {
  WiFiClient client = server.available();   // listen for incoming clients
  // again this is a variable of type class , which I don't quite get it for now

  if (client) {                             // if you get a client
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // declare a string to hold incomming data from TCP client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // .available() for most of the library means if there's bytes to read from ... , in this case is TCP client,
        char c = client.read();             // esp32 webserver read a byte from client (This case should be Device connected to SAP , like my phone for example)
        Serial.write(c);                    // then , esp32 print that out to Serial COM port for out PC tp read
        if (c == '\n') {                    // if the byte char read at this moment is a newline character
          // meaning that 

          // if the current line is blank, meaning you got two newline characters in a row.
          // which indicates the end of the client HTTP request, so send a response: (Server side)
          if (currentLine.length() == 0) {
            // after detecting a blank line we will use client object (but initialze from server obj. wth? why not name other ways) 
            // to printout the following HTML code as a response to client to generate webpage on the webserver hosted by esp32

            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK) , and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("Click <a href=\"/H\">here</a> to turn ON the LED.<br>");
            client.print("Click <a href=\"/L\">here</a> to turn OFF the LED.<br>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while client connect loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
          digitalWrite(LED_BUILTIN, HIGH);               // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(LED_BUILTIN, LOW);                // GET /L turns the LED off
        }
        /* tcp client (client connecting to server via TCP port 80 (Transport layer))
        Ex. "GET /H" is HTTP get request , so the server side will accept client request, and response with the LED blink command 
        (The request doesn't have to be requesting data)
        (And the response doesn't needs to contain any payload to send back) it can just be a simple communication , trigger control action */

      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}

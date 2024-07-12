/*
 WiFi Web Server LED Blink

 A simple web server that lets you blink an LED via the web.
 This sketch will print the IP address of your WiFi Shield (once connected)
 to the Serial monitor. From there, you can open that address in a web browser
 to turn on and off the LED on pin 5.

 If the IP address of your shield is yourAddress:
 http://yourAddress/H turns the LED on
 http://yourAddress/L turns it off

 This example is written for a network using WPA2 encryption. For insecure
 WEP or WPA, change the Wifi.begin() call and use Wifi.setMinSecurity() accordingly.

 Circuit:
 * WiFi shield attached
 * LED attached to pin 5

 created for arduino 25 Nov 2012
 by Tom Igoe

ported for sparkfun esp32 
31.01.2017 by Jan Hendrik Berlin
 
 */
#include "Arduino.h"
#include <WiFi.h>
// Brownout issue
#include "soc/soc.h"
#include "soc/rtc.h"

// Differs from the usage of AP , this ssid and password set isn't for AP , but for our MCU to access WiFi router ,in which will act as a client to
const char* ssid     = "ton kong 2.4G";
const char* password = "022872225";

byte ledpin = LED_BUILTIN;
// Set ESP32 as WiFiserver or TCP server (Handle TCP manually, not HTTP specific web protocol)
WiFiServer server(80);

void setup() {
    // set control reg. to disable brownout detector
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    Serial.begin(115200);
    pinMode(ledpin, OUTPUT);      // set the LED pin mode

    delay(10);

    // We start by connecting to a WiFi network

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    // ESP32 begin receiving WiFi -> Transmitting password back
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    
    server.begin();

}

void loop(){
    // waiting for WiFiclient request (Any type of request). (The code below will run per request)
    WiFiClient client = server.available();   // listen for incoming clients

    if (client) {                             // if you get a client,
        Serial.println("New Client.");           // print a message out the serial port
        String currentLine = "";                // make a String to hold incoming data from the client
        while (client.connected()) {            // loop while the client's connected
        if (client.available()) {             // if there's bytes to read from the client,
            char c = client.read();             // read a byte, then
            Serial.write(c);                    // print it out the serial monitor
            if (c == '\n') {                    // if the byte is a newline character

            // if the current line is blank, you got two newline characters in a row.
            // that's the end of the client HTTP request, so send a response:
            if (currentLine.length() == 0) {
                // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                // and a content-type so the client knows what's coming, then a blank line:
                client.println("HTTP/1.1 200 OK");
                client.println("Content-type:text/html");
                client.println();

                // the content of the HTTP response follows the header:
                client.print("Click <a href=\"/H\">here</a> to turn the LED on pin 5 on.<br>");
                client.print("Click <a href=\"/L\">here</a> to turn the LED on pin 5 off.<br>");

                // The HTTP response ends with another blank line:
                client.println();
                // break out of the while loop:
                break;
            } else {    // if you got a newline, then clear currentLine:
                currentLine = "";
            }
            } else if (c != '\r') {  // if you got anything else but a carriage return character,
            currentLine += c;      // add it to the end of the currentLine
            }

            // Check to see if the client request was "GET /H" or "GET /L":
            if (currentLine.endsWith("GET /H")) {
            digitalWrite(ledpin, HIGH);               // GET /H turns the LED on
            }
            if (currentLine.endsWith("GET /L")) {
            digitalWrite(ledpin, LOW);                // GET /L turns the LED off
            }
        }
        }
        // close the connection:
        client.stop();
        Serial.println("Client Disconnected.");
    }
}

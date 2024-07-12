/*
  ESP32 mDNS responder sample

  This is an example of an HTTP server that is accessible
  via http://esp32.local URL thanks to mDNS responder.

  Instructions:
  - Update WiFi SSID and password as necessary.
  - Flash the sketch to the ESP32 board
  - Install host software:
    - For Linux, install Avahi (http://avahi.org/).
    - For Windows, install Bonjour (http://www.apple.com/support/bonjour/).
    - For Mac OSX and iOS support is built in through Bonjour already.
  - Point your browser to http://esp32.local, you should see a response.

    Connect ESP32 STA to LAN , Use ESP32 as WiFiServer or Webserver that has locol DNS (Acheived by multicast domain name MDNS)
    It also has Service advertisement in case of Device from different protocol configuration to connect it 
 */


#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiClient.h>
// Brownout issue
#include "soc/soc.h"
#include "soc/rtc.h"

const char* ssid = "............";
const char* password = "..............";

// TCP server at port 80 will respond to HTTP requests
WiFiServer server(80);
// No HTTP on top of it

void setup(void)
{  
    // Set the control register , to disavle brownout detector.
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    Serial.begin(115200);

    // Connect to WiFi network
    WiFi.begin(ssid, password);
    Serial.println("");

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Set up mDNS responder:
    // - first argument is the domain name, in this example
    //   the fully-qualified domain name is "esp32.local"
    // - second argument is the IP address to advertise
    //   we send our IP address on the WiFi network
    if (!MDNS.begin("esp32")) {
        Serial.println("Error setting up MDNS responder!");
        while(1) {
            delay(1000);
        }
    }
    // This will make ESP32 as a Server , has its domain name, and discoverable on the same network (LAN)
    // As multicast DNS isn't central DNS , and
    Serial.println("mDNS responder started");

    // Start TCP (HTTP) server
    server.begin();
    Serial.println("TCP server started");

    // Service advertice ment to other device that will discover
    // Suitable for multiple device connect in multiple port or transfer protocol like UDP , even non HTTP like FTP
    MDNS.addService("http", "tcp", 80);
}

void loop(void)
{
    // Check if a client has connected to esp32 wifiServer
    WiFiClient client = server.available();
    if (!client) {
        return;
    }
    Serial.println("");
    Serial.println("New client");

    // Wait for data from client to become available
    while(client.connected() && !client.available()){
        delay(1);
    }

    /* Low level client handling , manually code HTTP */

    // Read the first line of HTTP request
    String req = client.readStringUntil('\r');

    // First line of HTTP request looks like "GET /path HTTP/1.1"
    // Retrieve the "/path" part by finding the spaces
    int addr_start = req.indexOf(' ');
    int addr_end = req.indexOf(' ', addr_start + 1);
    if (addr_start == -1 || addr_end == -1) {
        Serial.print("Invalid request: ");
        Serial.println(req);
        return;
    }
    req = req.substring(addr_start + 1, addr_end);
    Serial.print("Request: ");
    Serial.println(req);

    // Send HTTP header and Content
    String s;
    if (req == "/")
    {
        IPAddress ip = WiFi.localIP();
        String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
        s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>Hello from ESP32 at ";
        s += ipStr;
        s += "</html>\r\n\r\n";
        Serial.println("Sending 200");
    }
    else
    {
        s = "HTTP/1.1 404 Not Found\r\n\r\n";
        Serial.println("Sending 404");
    }
    client.print(s);

    client.stop();
    Serial.println("Done with client");
}


/* Simple ESP32 Webserver, using TCP and HTTP , Client is from browser which will send HTTP req. using Local Domain name provided by
  mdns.h , two device can successfully communicated over TCP/IP stack
*/
#include "Arduino.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

// Brownout issue
#include "soc/soc.h"
#include "soc/rtc.h"

const char* ssid     = "ton kong 2.4G";
const char* password = "022872225";

// Set ESP32 as Webserver (Handle TCP automatically, HTTP specific web protocol) (Abstract low level detail away)
WebServer server(80);

const short led = LED_BUILTIN;

// Routine to handle root url ("/")
void handleRoot() {
  digitalWrite(led, !digitalRead(led));
  server.send(200, "text/plain", "hello from esp32!");
}

// Routine if the root url is not found
void handleNotFound() {
  // String Concat technique to display HTML content
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST"; // shorthand if
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  // digitalWrite(led, 0);
}

void setup(void) {
  // Diable brownout detector
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);

  // Set STA mode and begin wifi connection
  WiFi.mode(WIFI_STA);
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

  // Waiting to begin DNS server responder
  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
    // mDNS.h only give Domain name as localhost name esp.local
    // Accessible through LAN only
  }
  

  /*Set up route handler :*/
    // For this case / is root url route (after domain name)
    server.on("/", handleRoot);
    // For this case it it route after route , /inline , you can just type that after root
    server.on("/inline", []() {
      server.send(200, "text/plain", "this works as well");
    });
    // Not found URL (http://)
    server.onNotFound(handleNotFound);

  server.begin(); // Begin ESP32 as WebHTTPServer
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient(); // Process incoming http request from clientside , and respond accordingly
  // Abstract lower level detail on connecting to tcp port , polling for client , parsing text bla bla..
  
  delay(2); //allow the cpu to switch to other tasks
}

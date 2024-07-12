/*
  To upload through terminal you can use: curl -F "image=@firmware.bin" esp32-webupdate.local/update

  Webupdater , use ESP32 as HTTP server , use HTTPUpdateserver lib. as simple block of process behind Webpage that can update esp32
  so this is like all other esp32 webserver , but HTTPUpdateServer.h will handle the webpage
 */
#include "Arduino.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <HTTPUpdateServer.h>

// Brownout issue
#include "soc/soc.h"
#include "soc/rtc.h"

// If STASSID has not been defined yet , define it
#ifndef STASSID
#define STASSID "ton kong 2.4G"
#define STAPSK  "022872225"
#endif

const char* host = "esp32-webupdate";
const char* ssid = STASSID;
const char* password = STAPSK;

// init webserver at tcp port 80 -> Use HTTP on top of that
WebServer httpServer(80);
HTTPUpdateServer httpUpdater;

void setup(void) {
  // Set the control register , to disavle brownout detector.
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(115200);
  Serial.println();
  Serial.println("Booting Sketch...");
  // Set Mode as both STA and AP , but why?
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);

  // Connection check 
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    Serial.println("WiFi failed, retrying.");
  }

  MDNS.begin(host);
  if (MDNS.begin(host)) {
    Serial.println("mDNS responder started");
  }

  // Setup a  httpUpdater to provide HTML builtin to Update ESP32 through Webserver that is going to begin 
  httpUpdater.setup(&httpServer);
  httpServer.begin();
  // Begin ESP32 as WebHTTPServer 

  // Service Advertisement to LAN ( In case of multiple client on network to know which protocol to use when communicate to it)
  MDNS.addService("http", "tcp", 80);
  Serial.printf("HTTPUpdateServer ready! Open http://%s.local/update in your browser\n", host);
}

void loop(void) {
  httpServer.handleClient(); // Handle HTTP client automatically
}
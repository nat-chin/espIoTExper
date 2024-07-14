#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>

// File system uploader plugin that is compatible with AVR ,ESP ,RP2040 , let you accees Flash memory easier
#include "LittleFS.h"

// Stepper library to interface with various stepper motor driver chips
// The Driver board I am using is ULN2003AN , Stepper motor 28BYJ-48
#include <Stepper.h>


// Stepper Motor Settings 2048
const int stepsPerRevolution = 1000;  // change this to fit the number of steps per revolution
// How much set stepping speed
#define IN1 19
#define IN2 18
#define IN3 5
#define IN4 17
Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4);

// Replace with your network credentials
const char* ssid     = "ton kong 2.4G";
const char* password = "022872225";

// Create Async webserver on TCP port 80
AsyncWebServer server(80);

// HTTP Search for parameters in HTTP POST request
const char* PARAM_INPUT_1 = "direction";
const char* PARAM_INPUT_2 = "steps";

// Variables to save values from HTML form
String direction;
String steps;

// a flag to detect whether a new client request occurred
bool newRequest = false;

// Store HTML variable in flash memory (Raw String literal) 
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Stepper Motor</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
  <h1>Stepper Motor Control</h1>
    <form action="/" method="POST">
      <!-- Form submit and reload back to Root URL (Request root page from server) -->
      <input type="radio" name="direction" value="CW" checked>
      <label for="CW">Clockwise</label>
      <input type="radio" name="direction" value="CCW">
      <label for="CW">Counterclockwise</label><br><br><br>
      <label for="steps">Number of steps:</label>
      <input type="number" name="steps">
      
      <input type="submit" value="GO!">
    </form>
</body>
</html>
)rawliteral";

// The Client doesn't incorporate AJAX for Async client response

// Initialize LittleFS
void initLittleFS() {
  if (!LittleFS.begin(true)) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  else {
  Serial.println("LittleFS mounted successfully");
  }
}

// function to Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}


void setup() {
  Serial.begin(115200);
  initLittleFS();
  initWiFi();

  myStepper.setSpeed(50); // step per sec

  /*Set URL Route handler */
  // Handle Root "/"  GET request. (Initial request for webpage)
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", index_html);
  });
  
  // Handle Root "/" POST request (HTML form)
  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
    int params = request->params();
    // Stores HTTP parameters length

    // Loop for all parameter ,
    for(int i=0;i<params;i++){
      // get HTTP request parameter , check if it's POST request (Form) , then check the parameter name if it has direction & steps
      // Response back with HTTP 200 ok HTML , then set newrequest flag to true
      AsyncWebParameter* p = request->getParam(i);
      if(p->isPost()){

        // Check if HTTP POST input1 value is "direction" , save the value in ESP32 variable
        if (p->name() == PARAM_INPUT_1) {
          direction = p->value().c_str();
          Serial.print("Direction set to: ");
          Serial.println(direction);
        }
        // Check if HTTP POST input2 value is "steps" , save the value in ESP32 variable
        if (p->name() == PARAM_INPUT_2) {
          steps = p->value().c_str();
          Serial.print("Number of steps set to: ");
          Serial.println(steps);
        }
      }
    }
    request->send(200, "text/html", index_html);
    newRequest = true;
  });

  // mdns name for local device
  const char *host = "ESPMotor";
  MDNS.begin(host);
  if (MDNS.begin(host)) {
    Serial.println("mDNS responder started");
  }

  server.begin();
  
  // Service Advertisement to LAN ( In case of multiple client)
  MDNS.addService("http", "tcp", 80);
  Serial.printf("HTTPUpdateServer ready! Open http://%s.local/update in your browser\n", host);
}

void loop() {
  /* Asyncwebserver does not require to handle client in the loop , but the code is for driving stepper motor */
  // Check if there was a new request and move the stepper accordingly
  if (newRequest){
    if (direction == "CW"){
      // Spin the stepper clockwise direction
      myStepper.step(steps.toInt());
    }
    else{
      // Spin the stepper counterclockwise direction
      myStepper.step(-steps.toInt());
    }
    newRequest = false; // reset flag
  }
}
// Import required libraries
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>
#include <LittleFS.h>
// inside there's Asyncwebsocket server side library

// Brownout issue
#include "soc/soc.h"
#include "soc/rtc.h"

// Replace with your network credentials
const char* ssid     = "ton kong 2.4G";
const char* password = "022872225";

bool ledState = 0;
const int ledPin = 2; // ledbuiltin

// create AsyncWebServer object connecting to ASync TCP port 80
AsyncWebServer server(80);
// create Websocketserver object specified WS url that should match with that of  JS client side
AsyncWebSocket ws("/ws");
// So instead of making our server run on HTTP , we will use WS protocol not on top, but replace it

// Store variable inside flashmemory with SPI flash fil system
// Store the variable inside flashmemory with PROGMEM
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
</head>
<body>
  <div class="topnav">
    <h1>ESP WebSocket Server</h1>
  </div>
  <div class="content">
    <div class="card">
      <h2>Output - GPIO 2</h2>
      <p class="state">state: <span id="state">%STATE%</span></p>
      <p><button id="button" class="button">Toggle</button></p>
    </div>
  </div>
<script>
  var gateway = `ws://${window.location.hostname}/ws`;
  var websocket;
  window.addEventListener('load', onLoad);
  function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage; // <-- add this line
  }
  function onOpen(event) {
    console.log('Connection opened');
  }
  function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
  }
  function onMessage(event) {
    var state;
    if (event.data == "1"){
      state = "ON";
    }
    else{
      state = "OFF";
    }
    document.getElementById('state').innerHTML = state;
  }
  function onLoad(event) {
    initWebSocket();
    initButton();
  }
  function initButton() {
    document.getElementById('button').addEventListener('click', toggle);
  }
  function toggle(){
    websocket.send('toggle');
  }
</script>
</body>
</html>
)rawliteral";

// ws.textAll is a function to bidirectionally send WS message to all connected client
// Message format is in Frame
void notifyClients() {
  ws.textAll(String(ledState));
}

// WS message handler, will handle WS data frame from Client Side
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  // compress all the websocket message (not inside HTTP body anymore) into AWSframe object

  // Checking all sorts of attribute in AWSframe
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;

    // Toggle LED state as opposite and call notifyclient 
    if (strcmp((char*)data, "toggle") == 0) {
      ledState = !ledState;
      notifyClients();
    }
  }
}

// onEvent function , handles all event that the client can trigger with its own websocket object
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

// init websocket on server side
void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

// function to replace the html place holder before sending the response
String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    if (ledState){
      return "ON";
    }
    else{
      return "OFF";
    }
  }
  return String();
}

void setup(){
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  // Serial port for debugging purposes
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  if (!LittleFS.begin(true)) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  else {
  Serial.println("LittleFS mounted successfully");
  }

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  //  init websocket server
  initWebSocket();

  // For WS handshake response, HTTP request Handler for root URL /
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
    // request->send(LittleFS, "/websocketIO.html", "text/html",processor);
  });
  // note that HTTP req. from WS client is different than regular -> learn more https://www.jittagornp.me/blog/what-is-websocket/

  // Start asycn WS server
  server.begin();
}

void loop() {

  // in every runtime loop of esp32 , clean up disconnected client (is that meaning closing the socket , or it having single socket)
  // continously change LED state (state variable will be changed in processor)

  ws.cleanupClients();
  digitalWrite(ledPin, ledState);
}
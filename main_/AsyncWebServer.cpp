/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-async-web-server-espasyncwebserver-library/
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

// Import required libraries
#include <WiFi.h>
#include <AsyncTCP.h>
// Actually AsyncTCP is already included in the below
#include <ESPAsyncWebServer.h>

// Replace with your network credentials
const char* ssid     = "ton kong 2.4G";
const char* password = "022872225";

// HTTP parameter to search from AJAX request
const char* PARAM_INPUT_1 = "output";
const char* PARAM_INPUT_2 = "state";

// Create AsyncWebServer object on TCP port 80
AsyncWebServer server(80);

#define LED1 32
#define LED2 33

// Using PROGMEM to write HTML DOC to ESP32 Flashmemory
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 6px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 3px}
    input:checked+.slider {background-color: #b30000}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body>
  <h2>ESP Web Server</h2>
  %BUTTONPLACEHOLDER%
<script>function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/update?output="+element.id+"&state=1", true); }
  else { xhr.open("GET", "/update?output="+element.id+"&state=0", true); }
  xhr.send();
}
</script>
</body>
</html>
)rawliteral";


// Function that return a string text of output state of LED , read locally to sync the checkbox in webpage 
String outputState(int output){
  if(digitalRead(output)){
    return "checked";
  }
  else {
    return "";
  }
}

// The callback function to be called when any type of GET req. to Root URL occurred, 
// Before server response with HTML DOC, pass that DOC through this callback, and when it found %BUTTONPLACEHOLDER% ,do the following  
String processor(const String& var){
  
  // Replaces placeholder with CSS styled button section 
  if(var == "BUTTONPLACEHOLDER"){
    String buttons = "";
    buttons += "<h4>Output - GPIO 2</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"2\" " + outputState(LED_BUILTIN) + "><span class=\"slider\"></span></label>";
    buttons += "<h4>Output - GPIO 32</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"32\" " + outputState(LED1) + "><span class=\"slider\"></span></label>";
    buttons += "<h4>Output - GPIO 33</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"33\" " + outputState(LED2) + "><span class=\"slider\"></span></label>";
    // a little strange why I need to put \ like a delimeter or something
    // The id attribute of checkbox is LEDPIN number which will be add to HTTP parameter named output = <inputMessage1>
    // The state of LED will be add by JS XMLHttpRequest manually
    // HTTP GET format /route?PARAM1=<value1>&PARAM2=<value2> 
    return buttons;
  }
  return String();
}




void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(LED1, OUTPUT);
  digitalWrite(LED1, LOW);
  pinMode(LED2, OUTPUT);
  digitalWrite(LED2, LOW);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  /* Set URL Route Handler*/

  // Route handler for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){

    request->send_P(200, "text/html", index_html, processor);
  });
  // For Asyncwebserver to work we need to pass HTTP request object to Handler function
  // This request will initially comes from web browser when 1st request for webpage , the processor() will be called immediately
  // and replace place holder with button (that has JS eventlistener attached)

  // Event Onchange detect or call togglecheckbox() which will do the following
  // Send a XML format GET request to <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
  // After clicking the button, onchange event trigger JS listener to GET request to /update URL in the background
  // Sending data as HTTP GET req. paramenter = <inputMessage1> & state = <inputMessage2>
  // Handler Fetch the each parameter -> check value -> digitalWrite(output,state)
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage1;
    String inputMessage2;
    // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
    // HTTP parameter  is output and state
    if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2)) {
      inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      digitalWrite(inputMessage1.toInt(), inputMessage2.toInt());
    }
    else {
      inputMessage1 = "No message sent";
      inputMessage2 = "No message sent";
    }
    Serial.print("GPIO: ");
    Serial.print(inputMessage1);
    Serial.print(" - Set to: ");
    Serial.println(inputMessage2);
    request->send(200, "text/plain", "OK");
  });

  // Start Async server
  server.begin();
}

void loop() {
 // no need for Async server to handle client request in loop , it will handle Non-Block request
 // It won't need to wait for the process to be done

 // How can the state be saved
 // So by process
}
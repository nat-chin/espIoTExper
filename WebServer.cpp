#include <Arduino.h>
#include <WiFi.h> 
// To open WiFI gateway + to open webserver
#include <ESPmDNS.h>
// For Local domain name + service advertisement

#define LED32 32
#define LED33 33
// Orange wire = 32
// Blue wire = 33

const char* ssid     = "ton kong 2.4G";
const char* password = "022872225";

WiFiServer httpserver(80);

String header = ""; // to Store http request header

// initialize state varaible of output as off
String state32 = "off";
String state33 = "off";

#define hostname "ESPwebserv"
const char *host = hostname;

// Set the timing variable
unsigned long currentTime = 0;
unsigned long previousTime = 0; 
// Define timeout = 2000ms = 2s
const long timeoutTime = 2000;

void setup(){
  Serial.begin(115200); pinMode(LED32,OUTPUT); pinMode(LED33,OUTPUT);

  // Check wifi connection
  Serial.print("Connecting to ");
  Serial.print(ssid);
  WiFi.mode(WIFI_STA); WiFi.begin(ssid,password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  Serial.println("");
  Serial.println("Succesfully Connected to "); Serial.print(WiFi.getHostname());
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()); // Printout ipv4 (Assigned by router)
  
  // Begin hosting as wifiserver
  httpserver.begin();

  // set mDNS name
  if(MDNS.begin(hostname)){
    Serial.println("MDNS responder started");
  }
  // set Service advertisement as http server (in case of multiple client)
  MDNS.addService("http", "tcp", 80);
  Serial.printf("HTTPServer ready! Open at http://%s.local in your browser \n", host);
}

void loop(){
  // provide method for server side to handle client manually
  WiFiClient client = httpserver.available(); // Listening for Client Connection

  if(client){
    currentTime = millis(); // Start the time stamp
    previousTime = currentTime;
    Serial.println("New Client.");

    String currentLine = "";   // currentline to store the HTTP Req. Body       
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis(); // 
      if(client.available()){
        char c = client.read();    // read a byte, of character
        Serial.write(c);          // print it out the serial monitor
        header += c;             // Store the entire request in header

        /* HTTP req. always end with two newline \n 
          1st time encounter \n -> Reset currentline http req. body to read new line
          if \n is detected again in next MCU runtime -> Send http response
          else if \r is undetected instead meaning there're more HTTP req. body -> Add to currentline again
        */

        if (c == '\n') {                
          
          // Satisfying the below condition = two newline has been read
          if (currentLine.length() == 0) {
            // Send HTTP Response from server :

            client.println("HTTP/1.1 200 OK"); // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK) 
            client.println("Content-type:text/html"); // and a content-type so the client knows what's coming, 
            client.println("Connection: close"); // server intruct client to terminate TCP port connection
            client.println(); // then a blank line:
            
            // turns the GPIOs on and off , using .indexOf() to check for specific substring
            if (header.indexOf("GET /32/on") >= 0) {
              Serial.println("GPIO 32 on");
              state32 = "on";
              digitalWrite(LED32, HIGH);
            } else if (header.indexOf("GET /32/off") >= 0) {
              Serial.println("GPIO 32 off");
              state32 = "off";
              digitalWrite(LED32, LOW);
            } else if (header.indexOf("GET /33/on") >= 0) {
              Serial.println("GPIO 33 on");
              state33 = "on";
              digitalWrite(LED33, HIGH);
            } else if (header.indexOf("GET /33/off") >= 0) {
              Serial.println("GPIO 32 off");
              state33 = "off";
              digitalWrite(LED33, LOW);
            }
            /* Display HTML Webpage (Manually handle is this hedache)*/
            
            // HTML DOM <head>
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");

            // CSS styling on ON/OFF buttons 
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // HTML DOM <Body>
            client.println("<body><h1>ESP32 Web Server</h1>");
            
            // Display current state, and ON/OFF buttons for GPIO 32  
            client.println("<p>GPIO 32 - State " + state32 + "</p>");
            // If the output26State is off, it displays the ON button       
            if (state32=="off") {
              client.println("<p><a href=\"/32/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/32/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
            // Display current state, and ON/OFF buttons for GPIO 27  
            client.println("<p>GPIO 33 - State " + state33 + "</p>");
            // If the output27State is off, it displays the ON button       
            if (state33=="off") {
              client.println("<p><a href=\"/33/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/33/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else {
            currentLine = "";
          }
        // if the currentline is has not detected carriage return yet (indication that the next character is \n)
        } else if (c != '\r'){
          currentLine += c;
          // append byte read from client to currentline
        }    
      }

    }
    // After the connection between client and Server has stopped (failure or Response is done)
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected."); Serial.println("");

  }
}
/*
 *  WiFi Button for IFTTT
 *  
 *  Configuration:
 *  1. Connect to power
 *  2. Connect with laptop/mobile to Access Point ESP#######
 *  3. Open IP 192.168.1.4 in browser
 *  4. Select of enter WiFi credentials
 *  5. Enter IFTTT Auth Key
 *  6. Press Save
 *  
 *  Created by Sebastian Hodapp, http://www.sebastian-hodapp.de
 */

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <Ticker.h>

// ========== start configuration ==========
#define DEBUG 1  // Switch on/off debugging via serial console

char* auth_key = ""; // Preset IFTTT auth key, if needed
// ========== end configuration ============

//Debug Definitions
#ifdef DEBUG
 #define DEBUG_PRINT(x)  Serial.println (x)
#else
 #define DEBUG_PRINT(x)
#endif

const int button = D3;  // Button
const int LED = 2;      // LED
Ticker ticker;          // Ticker

const char* host = "maker.ifttt.com"; // Host
const int httpsPort = 443;    // Port

void setup() {
  Serial.begin(115200);

  // Set LED Pins as Output and pull low for further status indication
  pinMode(BUILTIN_LED, OUTPUT);
  ticker.attach(0.6, status_led);

  pinMode(button, INPUT); // Setup Button

  // Connect to WiFi or start Configuration Mode
  WiFiManager wifiManager;
  WiFiManagerParameter custom_auth_key("auth_key", "IFTTT key", auth_key, 44);
  wifiManager.addParameter(&custom_auth_key);
  wifiManager.setConfigPortalTimeout(180);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.autoConnect();
  
  DEBUG_PRINT("WiFi connected");
  DEBUG_PRINT(WiFi.localIP());

  //Make LED solid if Sensor is connected
  ticker.detach();
  digitalWrite(BUILTIN_LED, LOW);
}

void loop() {
  int duration = 0;
  while (digitalRead(button) == LOW) {
    duration++;
    delay(100);
  }
  if (duration>0){
    if (duration>10){
      DEBUG_PRINT("Button pressed for a long time");
      duration = 0;
      trigger_event("long_press");
    } else {
      DEBUG_PRINT("Button pressed for a short time");
      duration = 0;
      trigger_event("short_press");
    }
  }

}

void trigger_event(String event){
  WiFiClientSecure client;
  DEBUG_PRINT("Connecting to: ");
  DEBUG_PRINT(host);

  if (!client.connect(host, httpsPort)) {
     DEBUG_PRINT("Connection failed");
    return;
  }
  
  String url = "/trigger/"+ event + "/with/key/" + auth_key;
  DEBUG_PRINT("requesting URL: ");
  DEBUG_PRINT(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: ESP8266btn\r\n" +
               "Connection: close\r\n\r\n");             
               
  DEBUG_PRINT("Request sent");
  DEBUG_PRINT("Reply:");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    DEBUG_PRINT(line);
  }
}

// Helper Function: Blink Onboard LED 
void status_led(){
  int state = digitalRead(LED);  // get the current state of GPIO1 pin
  digitalWrite(LED, !state);     // set pin to the opposite state
}

// Helper Function: Start Configuration mode if WiFi not maintained
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Enter Config Mode");
  Serial.println(WiFi.softAPIP());
  ticker.attach(0.2, status_led); //Make led toggle faster to indicate config mode
}

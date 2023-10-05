/*
Project:  Reed Switch with MQTT und OTA update
Author:   Thomas Edlinger for www.edistechlab.com
Date:     Created 26.11.2021
Version:  V1.0
IDE:      Arduino IDE 1.8.16
 
Required Board (Tools -> Board -> Boards Manager...)
 - Board: esp8266 by ESP8266 Community   V3.0.2 
 - Board: esp32   by Espressif Systems   V1.0.6

Required libraries (sketch -> include library -> manage libraries)
 - PubSubClient by Nick ‘O Leary V2.8.0
 - ArduinoOTA by Juraj Andrassy V1.0.5          
*/

#include <PubSubClient.h>
#include <ArduinoOTA.h>
//#include <WiFi.h>  // ESP32 only

#define wifi_ssid "wifi_ssid"
#define wifi_password "wifi_password"
#define mqtt_server "192.168.69.165"
#define mqtt_user "mqtt_user"         
#define mqtt_password "mqtt_password"
#define ESPHostname "Reed-GAS01"
String clientId = "Reed-Gas01-"; 

#define status_topic "reed_gas_01"
#define intopic "intopic"

const int reedPin = 13;  // D1=5 D7=13 - Wemos D1 mini
bool switchState;
bool reedStatusOpen = true;
const int sensorTakt = 1000; //alle 1 Sekunden wird der Sensor ausgelesen
long lastMsg = 0;
char msg[50];

WiFiClient espClient;  
PubSubClient client(espClient);  

void setup() {
  Serial.begin(115200);
  setup_wifi();
  ArduinoOTA.setHostname(ESPHostname);
  // ArduinoOTA.setPassword("admin");
  ArduinoOTA.begin();
  client.setServer(mqtt_server, 1883); 
  client.setCallback(callback); 
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  ArduinoOTA.handle(); 
  long now = millis();
  if (now - lastMsg > sensorTakt) {   
    lastMsg = now;
    readReedSwitch();
  }
}

void readReedSwitch() {
  switchState = digitalRead(reedPin);
  if (switchState == LOW) {
    if (reedStatusOpen == true) {
      Serial.println("Reed Switch is Closed.");
      client.publish(status_topic, "closed");
      reedStatusOpen = false;
    }
  } 
  else {
    if (reedStatusOpen == false) {
      Serial.println("Reed Switch is Open.");
      client.publish(status_topic, "open");
      reedStatusOpen = true;
    }
  } 
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}  

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if (String(topic) == intopic) {
    
  }
}

 void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(status_topic, ESPHostname);
      // ... and resubscribe
      client.subscribe(intopic);
    } else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        // Wait 5 seconds before retrying
        delay(5000);
      }
   }
}

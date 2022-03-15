#define ESP32
#define ISR_PREFIX ICACHE_RAM_ATTR

#include <WiFi.h>
#include <Wire.h>
#include <LoRa.h>             
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

//-----------------------------------------------init stuff-----------------------------------------------

//pins used by LoRa
#define ss 5
#define rst 14
#define dio0 2

//wifi creds
const char *ssid =  "";
const char *pass =  "";
WiFiClient client;

//adafruit io details
#define MQTT_SERV "io.adafruit.com"
#define MQTT_PORT 1883
#define MQTT_NAME "" // Your Adafruit IO Username
#define MQTT_KEY "" //  Your Adafruit IO AIO key
//adafruit io init
Adafruit_MQTT_Client mqtt(&client, MQTT_SERV, MQTT_PORT, MQTT_NAME, MQTT_KEY);
Adafruit_MQTT_Publish soil_moisture_feed = Adafruit_MQTT_Publish(&mqtt, MQTT_NAME "/f/ais.soilmoisture");
Adafruit_MQTT_Publish temp_feed = Adafruit_MQTT_Publish(&mqtt, MQTT_NAME "/f/ais.temperature");
Adafruit_MQTT_Publish hum_feed = Adafruit_MQTT_Publish(&mqtt, MQTT_NAME "/f/ais.humidity");
Adafruit_MQTT_Publish irrigation_event_feed = Adafruit_MQTT_Publish(&mqtt, MQTT_NAME "/f/ais.isirrigating");
Adafruit_MQTT_Publish water_level_feed = Adafruit_MQTT_Publish(&mqtt, MQTT_NAME "/f/ais.waterlevel");
Adafruit_MQTT_Subscribe water_pump = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/f/ais.isirrigating");

float soil_moisture = 0;
float temperature = 0;
float humidity = 0;
float waterlevel = 0;
int isPumpOn = 0;
int lastCommandTime = 0;
int lastPublishTime = 0;
int lastSubCheckTime = 0;

//-----------------------------------------------init stuff-----------------------------------------------


//-----------------------------------------------setup-----------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(10);

  //WiFi connect
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");            
  }
  Serial.println("WiFi connected");

  mqtt.subscribe(&water_pump);

  //initialize LoRa
  LoRa.setPins(ss, rst, dio0);
  while (!LoRa.begin(433E6)) {
    Serial.println(".");
    delay(500);
  }
  Serial.println("LoRa Initialized");
  LoRa.onReceive(recvCallback);
  LoRa.receive();

}

//-----------------------------------------------setup-----------------------------------------------


//-----------------------------------------------loop-----------------------------------------------

void loop() {

  MQTT_connect();

  //check for waterpump feed sub every 2 seconds
 // if (millis() - lastSubCheckTime > 500) {
    //check waterpump manual control feed from adafruit io
    Adafruit_MQTT_Subscribe *subscription;
    while ((subscription = mqtt.readSubscription(5000))) {
      if (subscription == &water_pump) {
        if (strcmp((char*)water_pump.lastread, "1") == 0 && isPumpOn == 0) {
          Serial.println("received manual irrigation command");
          sendCommand(1);
          isPumpOn = 1;
        }
        else if (strcmp((char*)water_pump.lastread, "0") == 0 && isPumpOn == 1) {
          Serial.println("received manual stop irrigation command");
          sendCommand(0);
          isPumpOn = 0;
        }
      }
    }
    if (!mqtt.ping()) mqtt.disconnect();
  //}

  //print current values
  Serial.println("Current Sensor Values: " + String(soil_moisture, 3)+ ", " + String(temperature, 3)+ ", " + String(humidity, 3));
 
  //publish current values to adafruit every 10 sec
  if (millis() - lastPublishTime > 10000) {
    if (!soil_moisture_feed.publish(soil_moisture)){delay(5000);}
    if (!temp_feed.publish(temperature)){delay(5000);}
    if (!hum_feed.publish(humidity)){delay(5000);}
    if (!water_level_feed.publish(waterlevel)){delay(5000);}
    if (!irrigation_event_feed.publish(isPumpOn)){delay(5000);} 
    Serial.println("values sent to adafruit");
    lastPublishTime = millis();
  }

  //if pump has been on for 2 seconds, turn it off
  if (isPumpOn && (millis() - lastCommandTime > 2000)) {
    sendCommand(0);
    isPumpOn = 0;
  }

}

//called when receive (need to create task cause interrupt wdt timeout exceptions)
void recvCallback(int packetSize) {
  if (packetSize == 0) return;
  xTaskCreate(recvTask, "recvTask", 5000, NULL, 4, NULL);
}
void recvTask(void* parameter) {

  Serial.println("receiving values through LoRa");

  int recipient = LoRa.read();          // recipient address
  byte incomingLength = LoRa.read();    // incoming msg length

  String incoming = "";                 // payload
  while (LoRa.available()) {            // can't use readString() in callback, so
    incoming += (char)LoRa.read();      // add bytes one by one
  }

  //corrupted or different message or not broadcast
  if (incomingLength != incoming.length() || recipient != 0xFF) {
    return;
  }

  stringParser(incoming);
  
  //send the appropriate command to endnode
  if (shouldIrrigate() == 1) {
    isPumpOn = 1;
    sendCommand(1);
  }

  vTaskDelete(NULL);

}

//helper for parsing incoming
ICACHE_RAM_ATTR void stringParser(String s){
  int i = s.indexOf(",");
  String temp = s.substring(0, i);
  soil_moisture=temp.toFloat();
  s=s.substring(i+1);
  
  i=s.indexOf(",");
  temp = s.substring(0, i);
  temperature = temp.toFloat();
  s=s.substring(i+1);
  
  humidity = s.toFloat();
  s.substring(i+1);
  
}

//send irrigation command to endnode
ICACHE_RAM_ATTR void sendCommand(int command) {
  if (command == 1) Serial.println("sending irrigation command through LoRa");
  String payload = String(command);

  LoRa.beginPacket();
  LoRa.write(0xFF);
  LoRa.write(payload.length());
  LoRa.print(payload);
  LoRa.endPacket();

  lastCommandTime = millis();

  LoRa.receive();
}

//irrigation logic based on sensor values
//TODO
ICACHE_RAM_ATTR int shouldIrrigate() {
  return 0;
}



















//connect to MQTT
void MQTT_connect() {
  int8_t again;
  uint8_t retry = 5;

  //if already connected leave
  if (mqtt.connected()) {
    return;
  }
  
  Serial.print("Connecting to Adafruit IO");
  while ((again = mqtt.connect()) != 0) { 
    //if could not connect, print reason for not connected, reduce no. of retries left, retry
    Serial.println(mqtt.connectErrorString(again));
    Serial.println("Retrying Adafruit connection");

    mqtt.disconnect();
    delay(5000);  

    retry--;
    if (retry == 0) {
      while (1);
    }
  }
  
  Serial.println("Adafruit IO is Connected!");
}

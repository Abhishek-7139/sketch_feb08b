#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <Wire.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

//define the pins used by the transceiver module
#define ss 5
#define rst 14
#define dio0 2

//WiFi Credentials
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
//Adafruit_MQTT_Subscribe water_pump = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/f/ais.isirrigating");

//variables
float soil_moisture = 0;
float temperature = 0;
float humidity = 0;
float waterlevel = 0;
int lastPublishTime = 0;

void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Receiver");
  
  //WiFi connect
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");            
  }
  Serial.println("WiFi connected");
//  mqtt.subscribe(&water_pump);

  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);
  
  //replace the LoRa.begin(---E-) argument with your location's frequency 
  //433E6 for Asia
  //866E6 for Europe
  //915E6 for North America
  while (!LoRa.begin(433E6)) {
    Serial.println(".");
    delay(500);
  }
   // Change sync word (0xF3) to match the sender
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");
  LoRa.onReceive(recvCallback);
  LoRa.receive();
}

void loop() {
  MQTT_connect();
  Serial.println("Current Sensor Values: " + String(soil_moisture, 3)+ ", " + String(temperature, 3)+ ", " + String(humidity, 3)+", "+String(waterlevel));
  
  //publish current values to adafruit every 20 sec
  if (millis() - lastPublishTime > 20000) {
    soil_moisture_feed.publish(soil_moisture);
    temp_feed.publish(temperature);
    hum_feed.publish(humidity);
    water_level_feed.publish(waterlevel);
    Serial.println("values sent to adafruit");
    lastPublishTime = millis();
  }

//  Adafruit_MQTT_Subscribe *subscription;
//  while ((subscription = mqtt.readSubscription(5000))) {
//    if (subscription == &water_pump) {
//      char* val = (char *)water_pump.lastread;
//      Serial.println(val);
//      Serial.print(F("Got: "));
//      if(strcmp(val, "0")==0){
//        sendCommand(0);
//      }else if(strcmp(val, "1")==0){
//        sendCommand(1);
//      }
//    }
//  }

//  int packetSize = LoRa.parsePacket();
 
  delay(5000);
}

void recvCallback(int packetSize){
  Serial.print("Lora Packet Recevied: "); Serial.println(packetSize);
   if (packetSize) {
    // received a packet
    int Recipient = LoRa.read();
    byte payload_length = LoRa.read();

    String payload = "";

    // read packet
    while (LoRa.available()) {
      String LoRaData = LoRa.readString();
      payload += LoRaData;
      Serial.print(LoRaData); 
    }
    Serial.println();
    Serial.print(payload_length);Serial.print(" ");Serial.println(payload.length());
    stringParser(payload);
  }
}

void stringParser(String s){
  int i = s.indexOf(",");
  String temp = s.substring(0, i);
  soil_moisture=temp.toFloat();
  s=s.substring(i+1);
  
  i=s.indexOf(",");
  temp = s.substring(0, i);
  temperature = temp.toFloat();
  s=s.substring(i+1);

  i=s.indexOf(",");
  temp = s.substring(0, i);
  humidity = temp.toFloat();
  s=s.substring(i+1);

  waterlevel = s.toFloat();
}

//void sendCommand(int command){
//  LoRa.beginPacket();
//  LoRa.write(command);
//  LoRa.endPacket();
//  LoRa.receive();
//}

void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
  }
  Serial.println("MQTT Connected!");
}

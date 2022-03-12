#include <WiFi.h>
#include <Wire.h>
#include <LoRa.h>             
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

//pins used by LoRa transceiver
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
#define MQTT_PASS "" //  Your Adafruit IO AIO key
//adafruit io init
Adafruit_MQTT_Client mqtt(&client, MQTT_SERV, MQTT_PORT, MQTT_NAME, MQTT_PASS);
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

void setup()
{
  Serial.begin(115200);
  delay(10);

  LoRa.setPins(ss, rst, dio0);
  while (!LoRa.begin(433E6)) {
    Serial.println(".");
    delay(500);
  }
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initialized");
  LoRa.receive();
  LoRa.onReceive(packetParser);
  
//  wifi connect
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");            
  }
  Serial.println("WiFi connected");

  mqtt.subscribe(&water_pump);
}

void loop()
{
  //connect to adafruit io
  MQTT_connect();

  Serial.println("New Loop");
//  int packetSize = LoRa.parsePacket();
//  if(packetSize){packetParser(packetSize);}
  Serial.println(String(soil_moisture, 3)+ "," + String(temperature, 3)+ "," + String(humidity, 3)+ "," + String(isPumpOn));
  
  if (!soil_moisture_feed.publish(soil_moisture)){delay(5000);}
  if (!temp_feed.publish(temperature)){delay(5000);}
  if (!hum_feed.publish(humidity)){delay(5000);}
  if (!water_level_feed.publish(waterlevel)){delay(5000);}
  if (!irrigation_event_feed.publish(isPumpOn)){delay(5000);}
  Serial.println("Kardiya send");

//  LoRa.beginPacket();
//  LoRa.print("1");
//  LoRa.endPacket();

/*Subscription code*/
//  Adafruit_MQTT_Subscribe *subscription;
//  while ((subscription = mqtt.readSubscription(5000))) {
//    // Check if its the onoff button feed
//    if (subscription == &water_pump) {
//      Serial.print(F("On-Off button: "));
//      Serial.println((char *)water_pump.lastread);
//      
//      if ((int)water_pump.lastread == 0) {
//        LoRa.beginPacket();
//        LoRa.print("0");
//        LoRa.end(); 
//      }
//      
//      if (strcmp((int)water_pump.lastread, "OFF") == 0) {
//        LoRa.beginPacket();
//        LoRa.print("1");
//        LoRa.end(); 
//      }
//    }
//  }

//  if(! mqtt.ping()) {
//    mqtt.disconnect();
//  }
  
  delay(10000);
}

void MQTT_connect() {
  
  int8_t again;

  //if already connected leave
  if (mqtt.connected()) {
    return;
  }
  Serial.print("Connecting to Adafruit IO");

  uint8_t retry = 5;

  while ((again = mqtt.connect()) != 0) { 
    //if could not connect, print reason for not connected, reduce no. of retries left, retry
    Serial.println(mqtt.connectErrorString(again));
    Serial.println("Retrying Adafruit connection in 5 seconds...");

    mqtt.disconnect();
    delay(5000);  

    retry--;
    if (retry == 0) {
      while (1);
    }
  }
  
  Serial.println("Adafruit IO is Connected!");
}

void packetParser(int packetSize){
  Serial.print("Lora packet recevied");
    while(LoRa.available()) {
      String LoRaReceived = LoRa.readString();
      Serial.println(LoRaReceived);
      stringParser(LoRaReceived);
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
  
  isPumpOn = s.toInt();
  s.substring(i+1);
}

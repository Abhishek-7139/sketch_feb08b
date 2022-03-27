#include <WiFi.h>
#include <Wire.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

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
Adafruit_MQTT_Subscribe water_pump = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/f/ais.isirrigating");

//variables
float soil_moisture = 0;
float temperature = 0;
float humidity = 0;
float waterlevel = 0;
int lastPublishTime = 0;

#define PUMP1 5

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
  mqtt.subscribe(&water_pump);

  pinMode(PUMP1, OUTPUT);
  digitalWrite(PUMP1, HIGH);
}

void loop() {
  MQTT_connect();

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &water_pump) {
      char* val = (char *)water_pump.lastread;
      Serial.print(F("Got: "));
      Serial.println(val);
      if(strcmp(val, "0")==0){
//        digitalWrite(PUMP1, HIGH);
//        Serial.println("Pump Off");
//        delay(2000);
      }else if(strcmp(val, "1")==0){
//        digitalWrite(PUMP1, LOW);
//        Serial.println("Pump On");
//        delay(2000);
      }
    }
  }
  delay(100);
}

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

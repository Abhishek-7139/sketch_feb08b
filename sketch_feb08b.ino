#include <WiFi.h>
#include <Wire.h>              
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "DHT.h"

//wifi creds
const char *ssid =  "";
const char *pass =  "";
WiFiClient client;

//adafruit io details
#define MQTT_SERV "io.adafruit.com"
#define MQTT_PORT 1883
#define MQTT_NAME "not_a_weeaboo" // Your Adafruit IO Username
#define MQTT_PASS "aio_eKHI207mJJRH3uitCFj3LleNKb4W" //  Your Adafruit IO AIO key
//adafruit io init
Adafruit_MQTT_Client mqtt(&client, MQTT_SERV, MQTT_PORT, MQTT_NAME, MQTT_PASS);
Adafruit_MQTT_Publish soil_moisture_feed = Adafruit_MQTT_Publish(&mqtt, MQTT_NAME "/f/soil_moisture");
Adafruit_MQTT_Publish temp_feed = Adafruit_MQTT_Publish(&mqtt, MQTT_NAME "/f/temp");
Adafruit_MQTT_Publish hum_feed = Adafruit_MQTT_Publish(&mqtt, MQTT_NAME "/f/hum");

//soil moisture sensor pin and var
const int Sensor_pin = 36;             
int Sensor_value;      

//dht init
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

void setup()
{
  Serial.begin(115200);
  delay(10);
  
  //wifi connect
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");            
  }
  Serial.println("WiFi connected");

  //begin dht sensor
  pinMode(DHTPIN, INPUT_PULLUP);
  dht.begin();
}

void loop()
{
  //connect to adafruit io
  MQTT_connect();

  //read soil moisture value
  //Sensor_value = ( 100 - ( (analogRead(Sensor_pin) / 1023.00) * 100 ) );
  Sensor_value = -analogRead(Sensor_pin);
  Serial.print("Soil Moisture is  = ");
  Serial.print(Sensor_value);
  Serial.println("%");
  //publish soil moisture to adafruit io
  if (!soil_moisture_feed.publish(Sensor_value)) 
  {                     
    delay(5000);   
  }

  //read temp and publish to adafruit io
  delay(2000);
  float temp = dht.readTemperature();
  if (!temp_feed.publish(temp)) 
  {                     
    delay(5000);   
  }
  //read humidity and publish to adafruit io
  delay(2000);
  float hum = dht.readHumidity();
  if (!hum_feed.publish(hum)) 
  {                     
    delay(5000);   
  }
  
  delay(6000);
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

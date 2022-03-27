#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>

//define the pins used by the transceiver module
#define ss 10
#define rst 9
#define dio0 2

//dht init
#define DHTPIN 3
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define MOISTUREPIN A1
#define SENSOR_MIN_MOISTURE 200
#define SENSOR_MAX_MOISTURE 1200

#define WATERLEVELPOWER 6//digital pin
#define WATERLEVELREADER A0//analog pin
#define SENSOR_MIN 0
#define SENSOR_MAX 521

#define PUMPIN 4
#define PUMPOUT 5

int lastSentTime = 0;

void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial);

  //dht setup
  pinMode(DHTPIN, INPUT_PULLUP);
  dht.begin();

  //water level sensor setup
  pinMode(WATERLEVELPOWER, OUTPUT);
  digitalWrite(WATERLEVELPOWER, LOW);

  //water pump pins
//  pinMode(PUMPIN, INPUT);
//  pinMode(PUMPOUT, OUTPUT);
//  digitalWrite(PUMPOUT, HIGH);
    
  Serial.println("LoRa Sender");

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
   // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");
}

void loop() {
  float moisture_value = analogRead(MOISTUREPIN);
  Serial.print("RAW MOSITURE: ");
  Serial.println(moisture_value);
  moisture_value = 100 - map(moisture_value, SENSOR_MIN_MOISTURE, SENSOR_MAX_MOISTURE, 0, 100);
  Serial.print("moisture_value: ");
  Serial.println(moisture_value);

  delay(2000);
  float temp_value = dht.readTemperature();
  Serial.print("Temperature: ");
  Serial.println(temp_value);
  
  delay(2000);
  float humidity_value = dht.readHumidity();
  Serial.print("Humidity: ");
  Serial.println(humidity_value);

  delay(2000);
  digitalWrite(WATERLEVELPOWER, HIGH);
  delay(10);
  int water_level = analogRead(WATERLEVELREADER);
  digitalWrite(WATERLEVELPOWER, LOW);
  water_level = map(water_level, SENSOR_MIN, SENSOR_MAX, 0, 100);
  Serial.print("water_level: ");
  Serial.println(water_level);

  if(millis()-lastSentTime >= 10000){
    String payload = String(moisture_value, 1)+","+String(temp_value, 1)+","+String(humidity_value, 1)+","+String(water_level);
    Serial.println("Sending Lora Packet....");
    LoRa.beginPacket();
    LoRa.write(0xFF);
    LoRa.write(payload.length());
    LoRa.print(payload);
    LoRa.endPacket();
    Serial.println("ended the packet");
    lastSentTime = millis();
  }

//  int pump_val = digitalRead(PUMPIN);
//  digitalWrite(PUMPOUT, pump_val);
//  delay(2000);
}

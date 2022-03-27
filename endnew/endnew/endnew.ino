#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>

//-----------------------------------------------init stuff-----------------------------------------------

//pins used by LoRa transceiver
#define ss 10
#define rst 9
#define dio0 2

#define MOISTUREPIN A1

//dht init
#define DHTPIN 3
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define WATERLEVELPOWER 6//digital pin
#define WATERLEVELREADER A0//analog pin

#define RELAYPIN 7
#define SIGPIN 5
//#define LEDPIN 13

#define SENSOR_MIN 0
#define SENSOR_MAX 521

#define SENSOR_MIN_MOISTURE 200
#define SENSOR_MAX_MOISTURE 1200

int lastCheck = 0;

//-----------------------------------------------init stuff-----------------------------------------------




//-----------------------------------------------setup-----------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(10);
//  pinMode(LEDPIN, OUTPUT);
  lastCheck = 0;

  //dht setup
  pinMode(DHTPIN, INPUT_PULLUP);
  dht.begin();

  pinMode(WATERLEVELPOWER, OUTPUT);
  digitalWrite(WATERLEVELPOWER, LOW);

  pinMode(RELAYPIN, OUTPUT);
  digitalWrite(RELAYPIN, HIGH);

//  lora setup
  LoRa.setPins(ss, rst, dio0);
  while (!LoRa.begin(433E6)) {
    Serial.println(".");
    delay(500);
  }
  Serial.println("LoRa Initialized");
//  LoRa.onReceive(recvCallback);
//  LoRa.receive();
}

//-----------------------------------------------setup-----------------------------------------------


//-----------------------------------------------loop-----------------------------------------------

void loop() {

  //read moisture, need to convert
  float moisture_value = analogRead(MOISTUREPIN);
  Serial.print("RAW MOSITURE: ");
  Serial.println(moisture_value);
  //moisture_value = ( 100 - ( (moisture_value / 1023.00) * 100 ) );
  moisture_value = 100 - map(moisture_value, SENSOR_MIN_MOISTURE, SENSOR_MAX_MOISTURE, 0, 100);
  Serial.print("moisture_value: ");
  Serial.println(moisture_value);

  //read temp, humidity
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


  String payload = String(moisture_value, 1)+","+String(temp_value, 1)+","+String(humidity_value, 1)+","+String(water_level);
//String payload = "A";
//  sendPackets(payload);
  LoRa.beginPacket();
  LoRa.write(0xFF);
  LoRa.write(payload.length());
  LoRa.print(payload);
Serial.println("ending the packet");
  LoRa.endPacket();
  Serial.println("ended the packet");
//  LoRa.receive();
  
//  Serial.println("isPumpOn" + String(isPumpOn));
  delay(2000);
//  if((millis()-lastCheck) > 10000){
//    shouldIrrigate(moisture_value, humidity_value, temp_value);
//  }
//  delay(100);
}

void shouldIrrigate(float soil_moisture, float humidity, float temperature) {
  if(soil_moisture<30 && humidity<70){
    if(temperature > 30){
      digitalWrite(RELAYPIN, LOW);
      delay(5000);
      digitalWrite(RELAYPIN, HIGH);
    }else{
      digitalWrite(RELAYPIN, LOW);
      delay(2000);
      digitalWrite(RELAYPIN, HIGH);
    }
  }
}

//void recvCallback(int packetSize) {
//  Serial.println("Lora callback: " + String(packetSize));
//  if (packetSize == 0) return;
//
//  int recipient = LoRa.read();          // recipient address
//  Serial.print("Recipient: "); Serial.println(recipient);
//  byte incomingLength = LoRa.read();    // incoming msg length
//  Serial.print("incomming length: "); Serial.println(incomingLength);
//
//  String incoming = "";                 // payload
//  while (LoRa.available()) {            // can't use readString() in callback, so
//    incoming += (char)LoRa.read();      // add bytes one by one
//  }
//  Serial.println("INCOMING:" + incoming);
//
////  corrupted or different message or not broadcast
////  if (incomingLength != incoming.length() || recipient != 0xFF) {
////    return;
////  }
//  
////
//  if (incoming.toInt() == 0) {
//    digitalWrite(RELAYPIN, HIGH);
//    Serial.println("recvd stop command");
//////    digitalWrite(LEDPIN, LOW);
//  }
//  else if (incoming.toInt() == 1) {
//    digitalWrite(RELAYPIN, LOW);
//    Serial.println("recvd irrigation command");
//////    digitalWrite(LEDPIN, HIGH);
//  }
//}

//-----------------------------------------------loop-----------------------------------------------

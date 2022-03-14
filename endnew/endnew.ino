#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>

//-----------------------------------------------init stuff-----------------------------------------------

//pins used by LoRa transceiver
#define ss 5
#define rst 14
#define dio0 2

#define MOISTUREPIN 32

//dht init
#define DHTPIN 22
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define RELAYPIN 25

//-----------------------------------------------init stuff-----------------------------------------------




//-----------------------------------------------setup-----------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(10);

  //dht setup
  pinMode(DHTPIN, INPUT_PULLUP);
  dht.begin();

  pinMode(RELAYPIN, OUTPUT);

  //lora setup
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

  digitalWrite(RELAYPIN, LOW);

  //read moisture, need to convert
  float moisture_value = -analogRead(MOISTUREPIN);
  moisture_value = ( 100 - ( (moisture_value / 1023.00) * 100 ) );
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

  LoRa.beginPacket();
  String payload = String(moisture_value, 3)+","+String(temp_value, 3)+","+String(humidity_value, 3);
  LoRa.beginPacket();
  LoRa.write(0xFF);
  LoRa.write(payload.length());
  LoRa.print(payload);
  LoRa.endPacket();

  LoRa.receive();

  delay(2000);
  
}

void recvCallback(int packetSize) {
  if (packetSize == 0) return;

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

  if (incoming.toInt() == 0) {
    digitalWrite(RELAYPIN, LOW);
    Serial.println("recvd stop command");
  }
  else if (incoming.toInt() == 1) {
    digitalWrite(RELAYPIN, HIGH);
    Serial.println("recvd irrigation command");
  }
  
}

//-----------------------------------------------loop-----------------------------------------------
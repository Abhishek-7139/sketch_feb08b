#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>

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

int isPumpOn = 0;

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
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initialized");

}

void loop() {

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

  Serial.println("Sending data by LoRa");
  //send sensors by lora
  LoRa.beginPacket();
  LoRa.print(String(moisture_value, 3) + "," + String(temp_value, 3) + "," + String(humidity_value, 3) + "," + String(isPumpOn) + "\n");
  LoRa.endPacket();

  //dont know how long need to wait before receiving
  //need to look at this
  delay(2000);

  //receive lora packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    while(LoRa.available()) {
      String LoRaReceived = LoRa.readString();
      Serial.print(LoRaReceived);
      //pump_on_off logic
      int pump_on = LoRaReceived[0]-'0';
      if(pump_on){digitalWrite(RELAYPIN, HIGH); isPumpOn = 1;}
      else{digitalWrite(RELAYPIN, LOW); isPumpOn = 0;}
    }
  }
  
  delay(10000);
}

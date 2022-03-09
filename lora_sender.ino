/*********
  Modified from the examples of the Arduino LoRa library
  More resources: https://randomnerdtutorials.com
*********/

#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>

//define the pins used by the transceiver module
#define ss 5
#define rst 14
#define dio0 2

#define moisture_pin 32
#define dht_pin 22
int counter = 0;

void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Sender");
  pinMode(dht_pin, INPUT_PULLUP);

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

  /*
  1. Read humidity, temperature (use DHT)
  2. Read soil moisture
  3. Send data using lora
  4. Receive command from main node
  5. Actuate based on command
  */

  /*
   1. Weather read from API
   2. Receive data
   3. Run inference
   4. Return on-off command 
  */
  
  //----------------------------------------------------
  Serial.print("Sending packet: ");
  Serial.println(counter);

  //Send LoRa packet to receiver
  LoRa.beginPacket();
  LoRa.print("hello ");
  LoRa.print(counter);
  LoRa.endPacket();

  counter++;

//-------------------------------------------------------------

  delay(10000);
}

// From https://docs.arduino.cc/tutorials/mkr-wan-1310/lora-sensor-data/
#include <SPI.h>
#include <LoRa.h>

int counter = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("LoRa Sender");
  
  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  
    delay(1000);
}

void loop() {
  Serial.print("Sending packet: ");
  Serial.println(counter);
  // send packet
  LoRa.beginPacket();
  LoRa.print(10.0);
  LoRa.print(" celsius");
  LoRa.endPacket();
  counter++;
  delay(5000);
}
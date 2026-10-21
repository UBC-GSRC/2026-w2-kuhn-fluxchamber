#include <SPI.h>
#include <LoRa.h>

const byte localAddress = 0xFF;      // Receiver address
const byte expectedSender = 0xBB;    // Transmitter address

void blinkCalibration()
{
  for (int i = 0; i < 20; i++)
  {
    digitalWrite(LED_BUILTIN, LOW);   // MKR LEDs are usually active LOW
    delay(50);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
  }
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // LED off

  Serial.begin(9600);

  // Give Serial a chance to connect, but don't block forever
  unsigned long start = millis();
  while (!Serial && (millis() - start < 3000))
  {
  }

  if (Serial)
  {
    Serial.println("LoRa Receiver Starting...");
  }

  if (!LoRa.begin(915E6))
  {
    if (Serial)
    {
      Serial.println("Starting LoRa failed!");
    }

    while (1)
    {
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
    }
  }

  if (Serial)
  {
    Serial.println("LoRa Receiver Ready");
  }
}

void loop()
{
  int packetSize = LoRa.parsePacket();

  if (!packetSize)
  {
    return;
  }

  // Read destination and source bytes
  int destinationAddr = LoRa.read();
  int sourceAddr = LoRa.read();

  // Read payload
  String message = "";

  while (LoRa.available())
  {
    message += (char)LoRa.read();
  }

  // Address filtering
  if (destinationAddr != localAddress)
  {
    return;
  }

  if (sourceAddr != expectedSender)
  {
    return;
  }

  if (Serial)
  {
    Serial.print("Received packet '");
    Serial.print(message);
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }

  // Calibration detection
  if (message.indexOf("Calibration") >= 0)
  {
    if (Serial)
    {
      Serial.println("*** CALIBRATION MESSAGE RECEIVED ***");
    }

    blinkCalibration();
  }
}
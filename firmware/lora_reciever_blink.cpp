#include <SPI.h>
#include <LoRa.h>

const byte localAddress   = 0xFF; // Receiver address
const byte expectedSender = 0xBB; // Transmitter address

// Rapid blink for calibration messages
void blinkCalibration()
{
  for (int i = 0; i < 20; i++)
  {
    digitalWrite(LED_BUILTIN, HIGH);   // ON (MKR LEDs are active LOW)
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);  // OFF
    delay(100);
  }
}

// Double blink for measurement messages
void blinkMeasurement()
{
  for (int i = 0; i < 2; i++)
  {
    digitalWrite(LED_BUILTIN, HIGH);   // ON
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);  // OFF
    delay(500);
  }
}

// Double blink for measurement messages
void blink()
{
  for (int i = 0; i < 3; i++)
    {
      digitalWrite(LED_BUILTIN, HIGH);   // ON
      delay(1000);
      digitalWrite(LED_BUILTIN, LOW);  // OFF
      delay(500);
    }
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);

  // LED OFF by default
  digitalWrite(LED_BUILTIN, LOW);

  Serial.begin(9600);

  // Allow serial monitor connection, but don't require it
  unsigned long start = millis();
  while (!Serial && (millis() - start < 5000))
  {
    delay(100);
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
    
    // Error indication
    while (1)
    {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
    }
  }
  
  if (Serial)
  {
    Serial.println("LoRa Receiver Ready");
  }
  blink();
}

void loop()
{
  int packetSize = LoRa.parsePacket();

  if (!packetSize)
  {
    return;
  }

  // First two bytes are destination and source addresses
  int destinationAddr = LoRa.read();
  int sourceAddr = LoRa.read();

  // Read payload
  String message = "";

  while (LoRa.available())
  {
    message += (char)LoRa.read();
  }

  // Ignore packets not meant for us
  if (destinationAddr != localAddress)
  {
    return;
  }

  // Ignore packets from unexpected senders
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

  // Blink pattern based on packet type
  if (message.indexOf("Calibration") >= 0)
  {
    if (Serial)
    {
      Serial.println("*** CALIBRATION MESSAGE RECEIVED ***");
    }

    blinkCalibration();
  }
  else if (message.indexOf("Measurement") >= 0)
  {
    if (Serial)
    {
      Serial.println("*** MEASUREMENT MESSAGE RECEIVED ***");
    }

    blinkMeasurement();
  }

  // Ensure LED is OFF after any blink pattern
  digitalWrite(LED_BUILTIN, LOW);
}
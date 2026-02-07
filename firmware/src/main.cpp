#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "utils.h"
#include <Co2Meter_K33.h>
#include "ArduinoLowPower.h"
#include "SerialCommands.h"
#include <LoRa.h>

enum State {
  INIT,             // Initialize the system
  FLUSH_CHAMBER,    // Fill chamber with fresh air
  ACCUMULATE_GAS,   // Let gas accumulate in chamber before measurement
  READ_DATA,        // Read data from sensor(s)
  LOG_DATA,         // Log data to SD card
  SLEEP,            // Enter low-power sleep mode
  FAKE_SLEEP,       // Fake sleep for testing
  LORA_RECEIVE,     // Listen for serial commands
  LORA_TRANSMIT,    // Transmit status over communication module
  SERIAL_RECEIVE,   // Listen for serial commands
  CALIBRATE,        // Calibrate sensors
  ERROR             // Handle errors
};

Co2Meter_K33 k33;
MethaneSensor methaneSensor(0);
// State state = INIT;
State state = CALIBRATE;
// State state = LORA_TRANSMIT;

const char* datalogFile = "datalog.csv";
unsigned long stateStartMillis = 0;
const unsigned long FLUSH_DURATION = 3 * 1000; // 10 seconds
const unsigned long ACCUMULATE_DURATION = 3 * 1000; // 10 seconds
const unsigned long FAKE_SLEEP_DURATION = 15 * 1000; // 15 seconds for testing
SensorData data;
bool shouldSleep = false;


unsigned PIN_WAKEUP = 3; // Pin to wake up from sleep
unsigned PIN_FAN = 7; // Pin to control fan
unsigned PIN_SWITCH = 4;
unsigned PIN_REED_SWITCH = 5; // Pin for reed switch
unsigned localAddress = 0xBB; // LoRa local address
unsigned destinationAddress = 0xFF; // LoRa destination address

void wakeupCallback() {
  // This function will be called once on device wakeup
  state = LORA_RECEIVE;
}

void turnOnFan(int duration_ms) {
    digitalWrite(PIN_FAN, HIGH);
    delay(duration_ms);
    digitalWrite(PIN_FAN, LOW);
}

SensorData readData(){
  SensorData data;
  K33Reading k33Data = k33.getReadings();
  rtc_get_time(1, data.date, sizeof(data.date));
  rtc_get_time(2, data.time, sizeof(data.time));
  snprintf(data.co2, sizeof(data.co2), "%.1f", k33Data.co2);
  snprintf(data.temp, sizeof(data.temp), "%.1f", k33Data.temp);
  snprintf(data.rh, sizeof(data.rh), "%.1f", k33Data.rh);
  snprintf(data.ch4, sizeof(data.ch4), "%.1f", 0.0); // Placeholder for methane voltage
  return data;
}

void setup() {
    Serial.begin(9600);
    Serial.println("SD Initializing...");
    if (!SD.begin(chipSelectSD)) {
    }

    // Create file if missing
    if (!SD.exists(datalogFile)) {
        File file = SD.open(datalogFile, FILE_WRITE);
        if (file) {
            Serial.print("Created file: ");
            Serial.println(datalogFile);
            file.close();
        } else {
            Serial.print("Failed to create file: ");
            Serial.println(datalogFile);
        }
    }

    Serial.println("SD initialization done.");
    rtc_init(false); // false = do not set time
    Wire.begin();
    methaneSensor.begin();

    pinMode(PIN_WAKEUP, INPUT_PULLUP);
    pinMode(PIN_FAN, OUTPUT);
    pinMode(PIN_SWITCH, INPUT_PULLUP);
    pinMode(PIN_REED_SWITCH, INPUT_PULLUP);

    LowPower.attachInterruptWakeup(PIN_WAKEUP, wakeupCallback, CHANGE);

    if (!LoRa.begin(915E6)) {
    while (1){
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
      }
    }
    delay(1000);
}

void loop() {
  switch (state) {

    case INIT:{
      // Initialization 
      if (Serial){
          Serial.println("INIT");
      }
      state = FLUSH_CHAMBER;
      break;
    }

    case FLUSH_CHAMBER:{
      // Flush the chamber to reach equilibrium with ambient air
      if (stateStartMillis == 0) {
        if (Serial){
          Serial.println("FLUSH_CHAMBER");
        }
        stateStartMillis = millis();
        // TODO: start flushing logic
        turnOnFan(FLUSH_DURATION);
      }
      else if (millis() - stateStartMillis >= FLUSH_DURATION) { // Flush for 10 seconds
        state = ACCUMULATE_GAS;
        stateStartMillis = 0;
      }
      break;
    }

    case ACCUMULATE_GAS:{
      // Let gas accumulate in chamber through diffusive flux 
      if (stateStartMillis == 0) {
        if (Serial){
          Serial.println("ACCUMULATE_GAS");
        }
        stateStartMillis = millis();
      }
      else if (millis() - stateStartMillis >= ACCUMULATE_DURATION) { // Accumulate for 10 seconds
        state = READ_DATA;
        stateStartMillis = 0;
      }
      break;
    }

    case READ_DATA: {
      // Read data from sensors 
      if (Serial){
          Serial.println("READ_DATA");
      }

      data = readData();

      if (Serial) {
        Serial.print("Date: "); Serial.print(data.date);
        Serial.print(" Time: "); Serial.print(data.time);
        Serial.print(" CO2 (ppm): "); Serial.print(data.co2);
        Serial.print(" Temp (C): "); Serial.print(data.temp);
        Serial.print(" RH (%): "); Serial.print(data.rh);
        Serial.print(" CH4 (V): "); Serial.print(data.ch4);
      }

      state = LOG_DATA;
      break;
    }

    case LOG_DATA: {
      // Log data to SD card
      if (Serial){
          Serial.println("LOG_DATA");
      }

      log_data(data, datalogFile); 
      state = LORA_RECEIVE;
      break;
    }

    case LORA_RECEIVE:{
      // Listen for incoming LORA packets which match a known format
      if (Serial){
          Serial.println("LORA_RECEIVE");
      }
      state = FAKE_SLEEP;

      break;
    }

    case SLEEP:{
      // Enter low-power sleep mode 
      if (Serial){
          Serial.println("SLEEP");
      } 

      rtc.setAlarm1(rtc.now() + TimeSpan(3), DS3231_A1_Second); // Wake up after 10 seconds
      LowPower.sleep(2000); // Gets out of sleep mode from interrupt
      state = INIT;
      break;
    }

    case FAKE_SLEEP:{
      // Fake sleep for testing purposes
      if (stateStartMillis == 0) {
        stateStartMillis = millis();
        if (Serial){
          Serial.println("FAKE_SLEEP");
        } 
        // TODO: start flushing logic
      }
      else if (millis() - stateStartMillis >= FAKE_SLEEP_DURATION) { // Flush for 10 seconds
        state = ACCUMULATE_GAS;
        stateStartMillis = 0;
        state = INIT;
      }
      break;
    }

    case LORA_TRANSMIT:{
      // Transmit information over LORA
        if (Serial){
          Serial.println("LORA_TRANSMIT");
        } 

        if (digitalRead(PIN_REED_SWITCH) == LOW){
          digitalWrite(LED_BUILTIN, HIGH);
          delay(500);
          digitalWrite(LED_BUILTIN, LOW);
          delay(500);
          digitalWrite(LED_BUILTIN, HIGH);
          delay(500);
          digitalWrite(LED_BUILTIN, LOW);
        }
      char t[16];
      rtc_get_time(2, t, sizeof(t));
      LoRa.beginPacket();
      LoRa.write(destinationAddress);
      LoRa.write(localAddress);
      LoRa.print(t);
      LoRa.endPacket();

      delay(5000);
      break;
    }

    case SERIAL_RECEIVE:{
      // Listen for serial commands 
      if (stateStartMillis == 0) {
        stateStartMillis = millis();

        if (Serial){
          Serial.println("SERIAL_RECEIVE");
        } 
      }
      else if (millis() - stateStartMillis >= 5000) { // Wait for 5 seconds
        state = SLEEP;
        stateStartMillis = 0;
      }
      Serial.begin(9600);
      
      break;
    }
    case CALIBRATE:{
      // Calibrate sensors. Print sensor readings to Serial for comparison with licor 
      if (!Serial){
          Serial.begin(9600);
          Serial.println("CALIBRATE");
      }

      data = readData();
      Serial.print("Date: "); Serial.print(data.date);
      Serial.print(" Time: "); Serial.print(data.time);
      Serial.print(" CO2 (ppm): "); Serial.print(data.co2);
      Serial.print(" Temp (C): "); Serial.print(data.temp);
      Serial.print(" RH (%): "); Serial.print(data.rh);
      Serial.print(" CH4 (V): "); Serial.print(data.ch4);
      Serial.println();

      // turnOnFan(3000); // Turn on fan for 3 seconds 
      turnOnFan(10000); // Turn on fan for 10 seconds 

      if (digitalRead(PIN_SWITCH) == LOW){
        // state = CALIBRATE; // Stay in calibration mode
        delay(500);
        Serial.println("Switch is low");

        checkSerial();
      } else {
        // state = INIT; // Move to logging mode
        delay(500);
        Serial.println("Switch is high");
        Serial.end();
        state = INIT;
      }
      // delay(15000); //    Wait 15 seconds more, CO2 sensor already has 15 second delay
      delay(5000); //    Wait 15 seconds more, CO2 sensor already has 15 second delay
      break;
    }
    case ERROR:{
      // Error handling code
      break;
    }
  }
}


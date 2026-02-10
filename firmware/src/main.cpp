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
  BLINK,            // Blink LED for testing
  CALIBRATE,        // Calibrate sensors
  TEST_BLINK_DELAY, // Test blinking with delay (blocking)
  TEST_BLINK_MILLIS, // Test blinking with millis (non-blocking)
  ERROR             // Handle errors
};

Co2Meter_K33 k33;
MethaneSensor methaneSensor(0);
// State state = INIT;
State statePrev;
State state = CALIBRATE;
// volatile State state = BLINK;
// State state = LORA_TRANSMIT;

const char* datalogFile = "datalog.csv";
unsigned long stateStartMillis = 0;
const unsigned long FLUSH_DURATION = 3 * 1000; // 10 seconds
const unsigned long ACCUMULATE_DURATION = 3 * 1000; // 3 seconds
const unsigned long CO2_GAS_DIFFUSION_DURATION = 25 * 1000; // 25 seconds
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

void reedSwitchCallback(){
  // Change this to start a full read sequence instead of just blinking
  state = BLINK;
}

SensorData readData(){
  // SensorData data;
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
    LowPower.attachInterruptWakeup(PIN_REED_SWITCH, reedSwitchCallback, FALLING);
    // attachInterrupt(digitalPinToInterrupt(PIN_REED_SWITCH), reedSwitchCallback, FALLING);

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
        digitalWrite(PIN_FAN, HIGH);
      }
      else if (millis() - stateStartMillis >= FLUSH_DURATION) { // Flush for 10 seconds
        state = ACCUMULATE_GAS;
        stateStartMillis = 0;
        digitalWrite(PIN_FAN, LOW);
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
      if (stateStartMillis == 0) {
        if (Serial){
          Serial.println("READ_DATA");
        }
        stateStartMillis = millis();
        k33.initPoll();
      }
      else if (millis() - stateStartMillis >= CO2_GAS_DIFFUSION_DURATION) { // Warmup for 16 seconds
        // wait until CO2 sensor is ready, then read data        
        // progress in the state machine
        rtc_get_time(1, data.date, sizeof(data.date));
        rtc_get_time(2, data.time, sizeof(data.time));
        snprintf(data.temp, sizeof(data.temp), "%.1f", k33.readTemp());
        delay(20);
        snprintf(data.rh, sizeof(data.rh), "%.1f", k33.readRh());
        delay(20);
        snprintf(data.co2, sizeof(data.co2), "%.1f", k33.readCo2());
        delay(20);
        // snprintf(data.ch4, sizeof(data.ch4), "%.1f", methaneSensor.voltageToPPM(methaneSensor.readVoltage()));
        // delay(20);
        stateStartMillis = 0;
        
        if (Serial) {
          Serial.print("Date: "); Serial.print(data.date);
          Serial.print(" Time: "); Serial.print(data.time);
          Serial.print(" CO2 (ppm): "); Serial.print(data.co2);
          Serial.print(" Temp (C): "); Serial.print(data.temp);
          Serial.print(" RH (%): "); Serial.print(data.rh);
          Serial.print(" CH4 (V): "); Serial.print(data.ch4);
        }

        if (statePrev == CALIBRATE){
          // If coming from calibration mode, stay in calibration mode to compare with licor readings
          state = CALIBRATE;
          statePrev = READ_DATA;
        } else {
          // Otherwise, move to logging mode
          state = LOG_DATA;
        }
      }
      else {
        digitalWrite(LED_BUILTIN, HIGH); // Turn on LED while waiting for sensor to be ready
        delay(1000);
        digitalWrite(LED_BUILTIN, LOW); // Turn off LED after reading data
        delay(1000);
      }
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
    case BLINK: {
      unsigned counter = 0;
      while (counter < 10){
        digitalWrite(LED_BUILTIN, HIGH);
        delay(500);
        digitalWrite(LED_BUILTIN, LOW);
        delay(100);
        counter++;
      }

      state = CALIBRATE; // Return to calibration mode after blinking
      break;
    }
    case CALIBRATE:{
      // Calibrate sensors. Print sensor readings to Serial for comparison with licor 
      if (!Serial){
          Serial.begin(9600);
          Serial.println("CALIBRATE");
      }

      if (digitalRead(PIN_SWITCH) == LOW){
        // state = CALIBRATE; // Stay in calibration mode
        delay(100);
        Serial.println("Switch is low");
        
        checkSerial();
        stateStartMillis = 0;
        statePrev = CALIBRATE;
        state = READ_DATA;
      } else {
        // state = INIT; // Move to logging mode
        delay(100);
        Serial.println("Switch is high");
        Serial.end();
        
        stateStartMillis = 0;
        state = INIT;
      }
      // delay(15000); //    Wait 15 seconds more, CO2 sensor already has 15 second delay
      // delay(5000); //    Wait 15 seconds more, CO2 sensor already has 15 second delay
      break;
    }
  case TEST_BLINK_DELAY: {
    Serial.println("TEST_BLINK_DELAY (blocking)");

    unsigned long start = millis();
    digitalWrite(LED_BUILTIN, HIGH);
    delay(5000); // Keep LED on for 5 seconds
    digitalWrite(LED_BUILTIN, LOW);

    Serial.println("TEST_BLINK_DELAY done");
    state = TEST_BLINK_DELAY;   // return where you want
    break;
    }
  
  case TEST_BLINK_MILLIS: {
    static bool initialized = false;
    static unsigned long start = 0;
    static unsigned long lastToggle = 0;
    static bool ledState = false;

    if (!initialized) {
        Serial.println("TEST_BLINK_MILLIS (non-blocking)");
        initialized = true;
        start = millis();
        lastToggle = millis();
        ledState = false;
        digitalWrite(LED_BUILTIN, LOW);
    }

  
    digitalWrite(LED_BUILTIN, HIGH);

    // End after 5 seconds
    if (millis() - start >= 5000) {
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println("TEST_BLINK_MILLIS done");
        initialized = false;
        state = TEST_BLINK_MILLIS;   // return where you want
    }

    break;
    }

    case ERROR:{
      // Error handling code
      break;
    }
  }
}


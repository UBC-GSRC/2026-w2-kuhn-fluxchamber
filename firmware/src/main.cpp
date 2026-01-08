#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "utils.h"
#include <Co2Meter_K33.h>
#include "ArduinoLowPower.h"

enum State {
  INIT,             // Initialize the system
  FLUSH_CHAMBER,    // Fill chamber with fresh air
  ACCUMULATE_GAS,   // Let gas accumulate in chamber before measurement
  READ_DATA,        // Read data from sensor(s)
  LOG_DATA,         // Log data to SD card
  SERIAL_LISTEN,    // Listen for serial commands
  SLEEP,            // Enter low-power sleep mode
  TRANSMIT_STATUS,  // Transmit status over communication module
  ERROR             // Handle errors
};

Co2Meter_K33 k33;
State state = INIT;
const char* datalogFile = "datalog.csv";
unsigned long stateStartMillis = 0;
const unsigned long FLUSH_DURATION = 3 * 1000; // 10 seconds
const unsigned long ACCUMULATE_DURATION = 3 * 1000; // 10 seconds

const unsigned long FAKE_SLEEP = 15 * 1000; // 15 seconds for testing

const char* row_data[5]; // Persistent array
bool shouldSleep = false;

// For CO2 sensor data reading
char co2Buf[16];
char tempBuf[16];
char rhBuf[16];
char rtcDate[16];
char rtcTime[16];

unsigned wakeupPin = 1; // Pin to wake up from sleep

void wakeupCallback() {
  // This function will be called once on device wakeup
  state = SERIAL_LISTEN;
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
    rtc_init(true);
    Wire.begin();

    pinMode(wakeupPin, INPUT_PULLUP);
    // Attach a wakeup interrupt on wakeupPin 8, calling repetitionsIncrease when the device is woken up
    LowPower.attachInterruptWakeup(wakeupPin, wakeupCallback, CHANGE);
}


void loop() {
  switch (state) {
    case INIT:{
      // Initialization code here
      Serial.println("In INIT");
      state = FLUSH_CHAMBER;
      break;
    }
    case FLUSH_CHAMBER:{
      if (stateStartMillis == 0) {
        Serial.println("In FLUSH_CHAMBER");
        stateStartMillis = millis();
        // TODO: start flushing logic
      }
      else if (millis() - stateStartMillis >= FLUSH_DURATION) { // Flush for 10 seconds
        state = ACCUMULATE_GAS;
        stateStartMillis = 0;
      }
      break;
    }
    case ACCUMULATE_GAS:{
      if (stateStartMillis == 0) {
        Serial.println("In ACCUMULATE_GAS");
        stateStartMillis = millis();
      }
      else if (millis() - stateStartMillis >= ACCUMULATE_DURATION) { // Accumulate for 10 seconds
        state = READ_DATA;
        stateStartMillis = 0;
      }
      break;
    }
    case READ_DATA: {
      Serial.println("In READ_DATA");
      K33Reading data = k33.getReadings(); // Read sensors

      rtc_get_time(1, rtcDate, sizeof(rtcDate));
      rtc_get_time(2, rtcTime, sizeof(rtcTime));

      snprintf(co2Buf, sizeof(co2Buf), "%.1f", data.co2);
      snprintf(tempBuf, sizeof(tempBuf), "%.1f", data.temp);
      snprintf(rhBuf, sizeof(rhBuf), "%.1f", data.rh);

      // Store in persistent row array
      row_data[0] = rtcDate;
      row_data[1] = rtcTime;
      row_data[2] = co2Buf;
      row_data[3] = tempBuf;
      row_data[4] = rhBuf;

      Serial.print("Date: "); Serial.print(rtcDate);
      Serial.print(" Time: "); Serial.print(rtcTime);
      Serial.print(" CO2 (ppm): "); Serial.print(co2Buf);
      Serial.print(" Temp (C): "); Serial.print(tempBuf);
      Serial.print(" RH (%): "); Serial.println(rhBuf);

      state = LOG_DATA;
      break;
    }
    case LOG_DATA: {
      Serial.println("In LOG_DATA");
      log_data(row_data, 5, datalogFile); 
      state = SERIAL_LISTEN;
      break;
    }
    case SERIAL_LISTEN:{
      // Code to listen for serial commands
      Serial.println("In SERIAL_LISTEN");
      state = SLEEP;

      break;
    }
    case SLEEP:{
      // Serial.println("In SLEEP");
      // rtc.setAlarm1(rtc.now() + TimeSpan(3), DS3231_A1_Second); // Wake up after 10 seconds
      // LowPower.sleep(2000); // Gets out of sleep mode from interrupt
      // state = INIT;

      if (stateStartMillis == 0) {
        Serial.println("In SLEEP");
        stateStartMillis = millis();
        // TODO: start flushing logic
      }
      else if (millis() - stateStartMillis >= FAKE_SLEEP) { // Flush for 10 seconds
        state = ACCUMULATE_GAS;
        stateStartMillis = 0;
        state = INIT;
      }
      break;
    }
    case TRANSMIT_STATUS:{
      // Code to transmit status
      Serial.println("In TRANSMIT_STATUS");
      break;
    }
    case ERROR:{
      // Error handling code
      Serial.println("In ERROR");
      break;
    }
  }
	// delay(60000 * 5); // Log every 5 minutes
}


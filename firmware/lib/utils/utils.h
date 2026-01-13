#pragma once

#include <SPI.h>
#include <SD.h>
#include "Adafruit_SHT4x.h"
#include <vector>
#include "RTClib.h"
#include "ArduinoLowPower.h"
#include <ADS1115-SOLDERED.h>

// SD 
extern int chipSelectSD; // SD card chip select pin for SPI
extern char logFilename[64]; 
void SD_init();
void log_data(const char* data[], size_t count,
              const char* filename);

// RTC 
extern char daysOfTheWeek[7][12];
extern RTC_DS3231 rtc;
extern int CLOCK_INTERRUPT_PIN;
void rtc_init(bool setTime);
void rtc_print_time(int mode = 0);
void rtc_get_time(int mode, char* out, size_t outSize);
void onAlarm();

// SHT45 Temperature and Humidity
Adafruit_SHT4x SHT45_init();

// Methane Sensor NG2611 through ADS1115

class MethaneSensor {
public:
    MethaneSensor(int adcChannel);
    void begin();
    float readVoltage();
    float voltageToPPM(float voltage);

private:
    float voltage_factor;
    int adcChannel;
    ADS1115 ads;
};

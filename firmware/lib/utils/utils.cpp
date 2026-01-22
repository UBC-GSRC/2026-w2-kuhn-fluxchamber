#include "utils.h"

#define CLOCK_INTTERUPT_PIN 1


int chipSelectSD = 2;
char logFilename[64] = "datalog.txt";
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
RTC_DS3231 rtc;
int CLOCK_INTERRUPT_PIN = 3;

void SD_init() {
    SD.begin(chipSelectSD);
    Serial.println("SD initialization done.");
}

Adafruit_SHT4x SHT45_init() {
    Adafruit_SHT4x sht4 = Adafruit_SHT4x();
    sht4.begin();
    sht4.setPrecision(SHT4X_HIGH_PRECISION);
    return sht4;
}

// void log_data(const char* data[], size_t count, const char* filename)
// {
//     File dataFile = SD.open(filename, FILE_WRITE);
//     if (!dataFile) {
//         Serial.println("Failed to open file for writing");
//         return;
//     }

//     for (size_t i = 0; i < count; ++i) {
//         dataFile.print(data[i]);
//         if (i < count - 1) {
//             dataFile.print(",");
//         }
//     }
//     dataFile.println();

//     dataFile.close();
// }

void log_data(const SensorData& d, const char* filename) {
  File f = SD.open(filename, FILE_WRITE);
  if (!f) {
    Serial.println("Failed to open file");
    return;
  }

  f.print(d.date); f.print(',');
  f.print(d.time); f.print(',');
  f.print(d.co2);  f.print(',');
  f.print(d.temp); f.print(',');
  f.print(d.rh);   f.print(',');
  f.print(d.ch4);
  f.println();

  f.close();
}

void rtc_init(bool setTime) {
    if (! rtc.begin()) {
        Serial.println("Couldn't find RTC");
        Serial.flush();
    }

    if (setTime) {
        // When time needs to be set on a new device, or after a power loss, the
        // following line sets the RTC to the date & time this sketch was compiled
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        // This line sets the RTC with an explicit date & time, for example to set
        // January 21, 2014 at 3am you would call:
        // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    }

    pinMode(CLOCK_INTERRUPT_PIN, INPUT_PULLUP);

    rtc.clearAlarm(1);
    rtc.clearAlarm(2);
    rtc.disable32K();

    rtc.disableAlarm(2);
    rtc.writeSqwPinMode(DS3231_OFF);
}

void rtc_print_time(int mode){
    DateTime now = rtc.now();

    if (mode == 0){ // Include date, day, and time
        Serial.print(now.year(), DEC);
        Serial.print('/');
        Serial.print(now.month(), DEC);
        Serial.print('/');
        Serial.print(now.day(), DEC);
        Serial.print(" (");
        Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
        Serial.print(") ");
        Serial.print(now.hour(), DEC);
        Serial.print(':');
        Serial.print(now.minute(), DEC);
        Serial.print(':');
        Serial.print(now.second(), DEC);
        Serial.println();
    }
    else if (mode == 1){ // Include ONLY date
        Serial.print(now.year(), DEC);
        Serial.print('/');
        Serial.print(now.month(), DEC);
        Serial.print('/');
        Serial.print(now.day(), DEC);
        Serial.println();
    }
    else if (mode == 2){ // Include ONLY time
        Serial.print(now.hour(), DEC);
        Serial.print(':');
        Serial.print(now.minute(), DEC);
        Serial.print(':');
        Serial.print(now.second(), DEC);
        Serial.println();
    }
    else if (mode == 3){ // Include ONLY day
        Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
        Serial.println();
    }    
}

void rtc_get_time(int mode, char* out, size_t outSize)
{
    DateTime now = rtc.now();

    switch (mode) {
        case 0: // date + day + time
            snprintf(
                out, outSize,
                "%04d/%02d/%02d (%s) %02d:%02d:%02d",
                now.year(),
                now.month(),
                now.day(),
                daysOfTheWeek[now.dayOfTheWeek()],
                now.hour(),
                now.minute(),
                now.second()
            );
            break;

        case 1: // date only
            snprintf(
                out, outSize,
                "%04d/%02d/%02d",
                now.year(),
                now.month(),
                now.day()
            );
            break;

        case 2: // time only
            snprintf(
                out, outSize,
                "%02d:%02d:%02d",
                now.hour(),
                now.minute(),
                now.second()
            );
            break;

        case 3: // day of week only
            snprintf(
                out, outSize,
                "%s",
                daysOfTheWeek[now.dayOfTheWeek()]
            );
            break;

        default:
            out[0] = '\0';  // empty string on invalid mode
            break;
    }
}


int rtc_get_hour(){
    DateTime now = rtc.now();

    return now.hour();
}

MethaneSensor::MethaneSensor(int adcChannel) {
    this->adcChannel = adcChannel;
    this->voltage_factor = 0.0f;
}

void MethaneSensor::begin() {
    ads.begin();          // Initialize hardware
    ads.setGain(0);
    voltage_factor = ads.toVoltage(1);
}

float MethaneSensor::readVoltage() {
    int16_t val = ads.readADC(adcChannel);
    return round(val * voltage_factor * 1000.0f) / 1000.0f;
}

float MethaneSensor::voltageToPPM(float voltage) {
    return 0.0f; // calibration later
}
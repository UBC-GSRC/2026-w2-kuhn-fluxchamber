#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "utils.h"
#include <Co2Meter_K33.h>

Co2Meter_K33 k33;
const char* datalogFile = "datalog.txt";


void SD_debug_suggestions(){
    Serial.println("initialization failed. Things to check:");
    Serial.println("1. is a card inserted?");
    Serial.println("2. is your wiring correct?");
    Serial.println("3. did you change the chipSelect pin to match your shield or module?");
    Serial.println("Note: press reset button on the board and reopen this Serial Monitor after fixing your issue!");
    while (true);
  }

void setup() {
  Serial.begin(9600);
//   while (!Serial); // BLOCK until serial monitor connects. Bad if you want to deploy
  
  Serial.println("SD Initializing...");
  if (!SD.begin(chipSelectSD)) {
    SD_debug_suggestions();
  }

  Serial.println("SD initialization done.");
  rtc_init(true);

  Wire.begin();

  
	std::vector<const char*> row = { "Date", "Time", "CO2_ppm", "Temp_C", "RH_%" };

	log_data(row, datalogFile);
}

void loop() {
  K33Reading data = k33.getReadings();	// Prepare test data (NO String)
	
	char co2Buf[16];
	char tempBuf[16];
	char rhBuf[16];

	char rtcDate[16];
	char rtcTime[16];

  rtc_get_time(1, rtcDate, sizeof(rtcDate));  // mode 1 = date only
	rtc_get_time(2, rtcTime, sizeof(rtcTime));  // mode 2 = time only
  
	snprintf(co2Buf, sizeof(co2Buf), "%.1f", data.co2);
	snprintf(tempBuf, sizeof(tempBuf), "%.1f", data.temp);
	snprintf(rhBuf, sizeof(rhBuf), "%.1f", data.rh);

	std::vector<const char*> row = { rtcDate, rtcTime, co2Buf, tempBuf, rhBuf };

	// log_data(row, datalogFile);
  Serial.print("Date: ");
  Serial.print(rtcDate);
  Serial.print(" Time: ");
  Serial.print(rtcTime);
  Serial.print(" CO2 (ppm): ");
  Serial.print(co2Buf);
  Serial.print(" Temp (C): ");
  Serial.print(tempBuf);
  Serial.print(" RH (%): ");
  Serial.println(rhBuf);

	// delay(60000 * 5); // Log every 5 minutes
  delay(15000); // Query every 30 seconds
}

#pragma once
#include <Arduino.h>

void checkSerial();           // call from loop()
void handleCommandLine(char* line);

struct DT {
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
};

struct Command {
    int32_t id;
    int32_t year;
    int32_t month;
    int32_t day;
    int32_t hour;
    int32_t minute;
    int32_t second;
};

const byte rxDataLen = sizeof(Command); 

// Parses a timestamp string "YYYY-MM-DD_HH:MM:SS" into a DateTime struct
// Returns true if parsing succeeded, false if invalid format
bool parseTimestamp(const char* str, DT& dt);

// New function stubs 
void readSerialKnownLength();
void readSensorsContinuous();
void readSensorsOnce();
void openVent();
void closeVent();
void startFan();
void stopFan();
void setDateTime();
void getDateTime();

bool recvStruct(Command* cmd);
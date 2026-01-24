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

// Parses a timestamp string "YYYY-MM-DD_HH:MM:SS" into a DT struct
// Returns true if parsing succeeded, false if invalid format
bool parseTimestamp(const char* str, DT& dt);
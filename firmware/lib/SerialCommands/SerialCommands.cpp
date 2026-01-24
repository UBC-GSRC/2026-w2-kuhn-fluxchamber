#include "SerialCommands.h"
#include <string.h>
#include "utils.h"

#define SERIAL_BUFFER 64

static char serialBuf[SERIAL_BUFFER];
static size_t bufIndex = 0;  // keeps track of the next free position

// --- Blocking read of a line from Serial ---
bool readSerialLineBlocking(char* buf, size_t bufSize) {
    size_t index = 0;

    while (true) {
        while (Serial.available() == 0) delay(1);

        char c = Serial.read();

        if (c == '\n' || c == '\r') {
            if (index == 0) continue;
            buf[index] = '\0';
            return true;
        }

        if (index < bufSize - 1) buf[index++] = c;
        else {
            buf[index] = '\0';
            return false; // buffer full
        }
    }
}

// --- Parse timestamp string into DT struct ---
bool parseTimestamp(const char* str, DT& dt) {
    if (strlen(str) != 19) return false;

    if (str[4] != '-' || str[7] != '-' || str[10] != '_' ||
        str[13] != ':' || str[16] != ':') return false;

    dt.year   = atoi(str);
    dt.month  = atoi(str + 5);
    dt.day    = atoi(str + 8);
    dt.hour   = atoi(str + 11);
    dt.minute = atoi(str + 14);
    dt.second = atoi(str + 17);

    if(dt.month < 1 || dt.month > 12) return false;
    if(dt.day < 1 || dt.day > 31) return false;
    if(dt.hour < 0 || dt.hour > 23) return false;
    if(dt.minute < 0 || dt.minute > 59) return false;
    if(dt.second < 0 || dt.second > 59) return false;

    return true;
}

// --- Handle TIME_SET command ---
void onTimeSet() {
    Serial.println("BLOCKING: Please enter the new time to set with the format (YYYY-MM-DD_HH:MM:SS)");

    if (!readSerialLineBlocking(serialBuf, SERIAL_BUFFER)) {
        Serial.println("ERR TIME_INPUT_TIMEOUT");
        return;
    }

    DT dt;
    if (!parseTimestamp(serialBuf, dt)) {
        Serial.println("ERR INVALID_TIMESTAMP");
        return;
    }

    // Update RTC immediately
    rtc.adjust(DateTime(dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second));
    Serial.println("ACK TIME_SET");
}

// --- Handle a single command line ---
void handleCommandLine(char* line) {
    // Trim trailing whitespace
    size_t len = strlen(line);
    while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r' || line[len-1] == ' ')) {
        line[--len] = '\0';
    }
    if (len == 0) return;

    if (strcmp(line, "TIME_SET") == 0) {
        onTimeSet();
    } 
    else if (strcmp(line, "RAISE_CHAMBER") == 0) {
        Serial.println("ACK RAISE_CHAMBER");
        // TODO: raise chamber logic
    } 
    else if (strcmp(line, "LOWER_CHAMBER") == 0) {
        Serial.println("ACK LOWER_CHAMBER");
        // TODO: lower chamber logic
    } 
    else {
        Serial.println("ERR UNKNOWN_CMD");
    }
}

// --- Call from loop() to process incoming Serial characters ---
void checkSerial() {
    while (Serial.available()) {
        char c = Serial.read();

        // Command is only sent if newline or carriage return is received
        if (c == '\n' || c == '\r') {
            if (bufIndex == 0) continue; // skip empty lines
            serialBuf[bufIndex] = '\0';  // terminate string
            handleCommandLine(serialBuf);
            bufIndex = 0;                // reset for next line
        } 
        else {
            // Append char if buffer not full
            if (bufIndex < SERIAL_BUFFER - 1) {
                serialBuf[bufIndex++] = c;
            }
            // else: ignore excess chars
        }
    }
}
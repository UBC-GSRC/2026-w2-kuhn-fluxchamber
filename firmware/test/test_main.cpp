#include <Arduino.h>
#include <unity.h>
#include "utils.h"
#include <Co2Meter_K33.h>

// Unity Assertions Cheat Sheet: https://github.com/ThrowTheSwitch/Unity/blob/master/docs/UnityAssertionsCheatSheetSuitableforPrintingandPossiblyFraming.pdf

void setUp() {}
void tearDown() {}

void test_led_builtin_pin_number() {
    TEST_ASSERT_EQUAL(6, LED_BUILTIN);
}

void test_sd_card_available() {
    SD_init();
    File dataFile = SD.open(logFilename, FILE_WRITE);
    TEST_ASSERT_TRUE_MESSAGE(dataFile, "SD card not available");
}

void test_data_logging() {
  // Clear the file first (optional)
  const char* testFile = "testFile.txt";

  // Prepare test data (NO String)
  std::vector<const char*> testRow = {
      "Test1",
      "Test2",
      "Test3"
  };

  log_data(testRow, testFile);

  // Read back file
  File dataFile = SD.open(testFile, FILE_READ);
  TEST_ASSERT_TRUE_MESSAGE(dataFile, "Failed to open testFile.txt for reading");

  // Read entire file into buffer
  String fileContent;
  while (dataFile.available()) {
      fileContent += (char)dataFile.read();
  }
  dataFile.close();

  // Extract last line (ignore trailing newline)
  int lastNewline = fileContent.lastIndexOf('\n');
  int prevNewline = fileContent.lastIndexOf('\n', lastNewline - 1);

  String lastLine;
  if (prevNewline >= 0) {
      lastLine = fileContent.substring(prevNewline + 1, lastNewline);
  } else {
      lastLine = fileContent.substring(0, lastNewline);
  }

  // Validate CSV output
  const char* expectedLine = "Test1,Test2,Test3";
  TEST_ASSERT_EQUAL_STRING_MESSAGE(
      expectedLine,
      lastLine.c_str(),
      "CSV last line does not match expected output"
  );

  // Delete the test file
  bool deleted = SD.remove(testFile);
  TEST_ASSERT_TRUE_MESSAGE(deleted, "Failed to delete log file after test");
}

void test_SHT45_sensor()
{
  Adafruit_SHT4x sht4 = SHT45_init();
  sensors_event_t humidity, temp;
  sht4.getEvent(&humidity, &temp);
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(50.0, 50.0, humidity.relative_humidity, "Relative humidity not reaeding correctly."); // Example expected value
  TEST_ASSERT_FLOAT_WITHIN_MESSAGE(10.0, 25.0, temp.temperature, "Temperature not reading correctly."); // Example expected value
}

void test_K33_ELG_sensor(void)
{
  // Placeholder for K33 ELG sensor test
  Co2Meter_K33 k33;

  K33Reading data = k33.getReadings();

  TEST_ASSERT_TRUE(data.co2 >= 300.0);
  TEST_ASSERT_TRUE(data.temp >= -40.0 && data.temp <= 85.0);
  TEST_ASSERT_TRUE(data.rh >= 0.0 && data.rh <= 100.0);
}

void test_NGM2611_E13_sensor(void)
{
  // Placeholder for NGM2611 E13 sensor test
  TEST_ASSERT_TRUE(true);
}

void test_LoRa_communication(void)
{
  // Placeholder for LoRa communication test
  TEST_ASSERT_TRUE(true);
}

void test_low_power_mode(void)
{
  // Placeholder for low power mode test
  TEST_ASSERT_TRUE(true);
}

void test_RTC_connected(void){
  TEST_ASSERT_MESSAGE(rtc.begin(), "RTC not found.");
}

String formattedDate() {
    // __DATE__ --> "Dec  5 2025"
    const char* raw = __DATE__;
    char monthStr[4];
    int day, year;

    sscanf(raw, "%3s %d %d", monthStr, &day, &year);

    int month;
    if      (!strcmp(monthStr, "Jan")) month = 1;
    else if (!strcmp(monthStr, "Feb")) month = 2;
    else if (!strcmp(monthStr, "Mar")) month = 3;
    else if (!strcmp(monthStr, "Apr")) month = 4;
    else if (!strcmp(monthStr, "May")) month = 5;
    else if (!strcmp(monthStr, "Jun")) month = 6;
    else if (!strcmp(monthStr, "Jul")) month = 7;
    else if (!strcmp(monthStr, "Aug")) month = 8;
    else if (!strcmp(monthStr, "Sep")) month = 9;
    else if (!strcmp(monthStr, "Oct")) month = 10;
    else if (!strcmp(monthStr, "Nov")) month = 11;
    else if (!strcmp(monthStr, "Dec")) month = 12;
    else month = 13;

    char buffer[20];
    sprintf(buffer, "%04d/%02d/%02d", year, month, day);
    return String(buffer);
}

void test_RTC_date_synchronization(void)
{
    bool setTime = true;
    rtc_init(setTime);

    // Get RTC date into buffer
    char rtcDate[16];
    rtc_get_time(1, rtcDate, sizeof(rtcDate));  // mode 1 = date only

    // Compare against expected formatted date
    TEST_ASSERT_EQUAL_STRING_MESSAGE(
        formattedDate().c_str(),
        rtcDate,
        "RTC date does not match expected formatted date"
    );
}


volatile bool irq = false;
void alarm_isr() { irq = true; }

void test_RTC_alarm(void)
{
  bool setTime = true;
  rtc_init(setTime);

  // Placeholder for RTC alarm function test
  // Making it so, that the alarm will trigger an interrupt
  pinMode(CLOCK_INTERRUPT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN), alarm_isr, FALLING);

  rtc.setAlarm1(
        rtc.now() + TimeSpan(3),
        DS3231_A1_Second // this mode triggers the alarm when the seconds match. See Doxygen for other options
  );

  delay(3100);
  int fired = rtc.alarmFired(1);
  rtc.clearAlarm(1);
  detachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN));
  TEST_ASSERT_TRUE_MESSAGE(fired & irq, "Is the SQW pin of the RTC connected to the interrupt pin?");
}

void run_all_tests() {
    UNITY_BEGIN();
    RUN_TEST(test_led_builtin_pin_number);
    RUN_TEST(test_sd_card_available);
    RUN_TEST(test_SHT45_sensor);
    RUN_TEST(test_data_logging);
    RUN_TEST(test_RTC_date_synchronization);
    RUN_TEST(test_RTC_alarm);
    RUN_TEST(test_K33_ELG_sensor);
    UNITY_END();
}

void setup() {
    delay(2000);
    pinMode(LED_BUILTIN, OUTPUT);
    run_all_tests();
}

void loop() {
}

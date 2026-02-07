//
//    FILE: ADS_read.ino
//  AUTHOR: Rob.Tillaart
// VERSION: 0.2.1
// PURPOSE: read analog inputs - straightforward.
//

// Used by Soldered EasyC board with minor additions

// test
// connect 1 potmeter per port.
//
// GND ---[   x   ]------ 5V
//            |
//
// measure at x (connect to AIN0).
//

#include "ADS1115-SOLDERED.h"

// Delcare your ADC object
ADS1115 ADS;

void setup()
{
    // init the Serial communication
    Serial.begin(115200);
    Serial.println(__FILE__);
    Serial.print("ADS1X15_LIB_VERSION: ");
    Serial.println(ADS1X15_LIB_VERSION);

    // init the ADC
    ADS.begin();
}

void loop()
{
    // Set the gain
    ADS.setGain(0);

    // read the value
    int16_t val_0 = ADS.readADC(0);

    float f = ADS.toVoltage(1); // voltage factor

    Serial.print("\tAnalog0: ");
    Serial.print(val_0);
    Serial.print('\t');
    Serial.print(val_0 * f, 3);
    Serial.print('\t');
    Serial.print(ADS.toVoltage(val_0));
    Serial.println();

    delay(1000);
}

// -- END OF FILE --
#include <Wire.h>
#include "Adafruit_SGP30.h"
#include <Adafruit_NeoPixel.h>

// include wifi stuff so we can turn it off
#include <ESP8266WiFi.h>


Adafruit_SGP30 sgp;

// https://dash.harvard.edu/bitstream/handle/1/27662232/4892924.pdf?sequence=1

// http://blog.gigabase.org/en/contents/132

/*
 * This part will measure eCO2 (equivalent calculated carbon-dioxide) concentration within a range of 0 to 60,000 
 * parts per million (ppm), and TVOC (Total Volatile Organic Compound) concentration within a range of 0 to 
 * 60,000 parts per billion (ppb).
 */

// change this if it's one of the swapped round pixels we seem to get occasionally
// it seems GRB is the default
//#define ORDER NEO_GRB
// change it to RGB if it's one of the odd ones that flashes white at power up
#define ORDER NEO_RGB
Adafruit_NeoPixel pixel = Adafruit_NeoPixel(1, D3, ORDER);



void setup() {
  pixel.begin();
  delay(10);
  // pale blue to show it's working
  pixel.setPixelColor(0, 0x000040);
  pixel.show();

  // turn off the WiFi;
  WiFi.forceSleepBegin();  


  Serial.begin(9600);
  Serial.println("SGP30 test");

  if (! sgp.begin()){
    Serial.println("Sensor not found :(");
    while (1);
  }
  Serial.print("Found SGP30 serial #");
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);

  // If you have a baseline measurement from before you can assign it to start, to 'self-calibrate'
  //sgp.setIAQBaseline(0x8E68, 0x8F41);  // Will vary for each sensor!
}

int counter = 0;

void loop() {

  if (! sgp.IAQmeasure()) {
    Serial.println("Measurement failed");
    return;
  }
  Serial.print("TVOC "); Serial.print(sgp.TVOC); Serial.print(" ppb\t");
  Serial.print("eCO2 "); Serial.print(sgp.eCO2); Serial.println(" ppm");

  // fart detected!
  if (sgp.TVOC > 1000) { // purple
      pixel.setPixelColor(0, 0xFF00FF);
      pixel.show();   
  }
  else {
    // change the GlowOrb
    if (sgp.TVOC == 0 && sgp.eCO2 == 400) {
      // far niente - still warming up 
    }
    else if (sgp.eCO2 < 600) { // green
        pixel.setPixelColor(0, 0x00FF00);
        pixel.show();
    }
    else if (sgp.eCO2 > 605 && sgp.eCO2 < 995) { // amber
        pixel.setPixelColor(0, 0xFF9900);
        pixel.show();
    }
    else if (sgp.eCO2 > 1000) { // red
        pixel.setPixelColor(0, 0xFF0000);
        pixel.show();
    }
  }

  // read once a second
  delay(1000);

  counter++;

  // every minute, print out the baseline values
  if (counter == 60) {
    counter = 0;

    uint16_t TVOC_base, eCO2_base;
    if (! sgp.getIAQBaseline(&eCO2_base, &TVOC_base)) {
      Serial.println("Failed to get baseline readings");
      return;
    }
    Serial.print("****Baseline values: eCO2: 0x"); Serial.print(eCO2_base, HEX);
    Serial.print(" & TVOC: 0x"); Serial.println(TVOC_base, HEX);
  }
}

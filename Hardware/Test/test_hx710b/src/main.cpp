#include <Arduino.h>

#include "HX710B.h"
#define SCK_PIN 3
#define SDI_PIN 4

HX710B air_press(SCK_PIN, SDI_PIN);

void setup()
{
    Serial.begin(9600);
    if ( !air_press.init() )
    { 
      Serial.println(F("HX710B not Found !"));
      while(1);
    }
}

uint32_t time_update = 0;
void loop()
{
    uint32_t rollOver = millis();
    if( rollOver < time_update )
      time_update = rollOver;
    if( millis() - time_update >= 2000UL )
    {
      uint32_t data_raw = 0;
      if ( air_press.read(&data_raw, 1000UL) != HX710B_OK )
        Serial.println(F("something error !"));
      else
      {
        Serial.print(F("Data raw of ADC is : "));
        Serial.println(((unsigned long) data_raw - 8388608)/209716.425);
      }
      time_update = millis();            
    }
}
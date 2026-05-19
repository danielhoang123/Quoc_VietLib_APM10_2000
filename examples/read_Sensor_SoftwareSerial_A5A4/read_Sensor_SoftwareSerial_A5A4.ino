#include <SoftwareSerial.h>
#include "VNEHC_APM.h"

SoftwareSerial dustSerial(A5, A4);

VNEHC_APM dust(&dustSerial);

VNEHC_APM::Data data;

uint32_t prevPrint = 0;

void setup()
{
  Serial.begin(115200);

  dustSerial.begin(1200);

  Serial.println("APM2000 UART Example");
}

void loop()
{
  if (dust.read(&data))
  {
    Serial.println("===== APM2000 =====");

    Serial.print("PM1.0 : ");
    Serial.print(data.pm1_0);
    Serial.println(" ug/m3");

    Serial.print("PM2.5 : ");
    Serial.print(data.pm2_5);
    Serial.println(" ug/m3");

    Serial.print("PM10  : ");
    Serial.print(data.pm10);
    Serial.println(" ug/m3");

    Serial.println();
  }
  
  delay(1000);
}
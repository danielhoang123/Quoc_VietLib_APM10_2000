#include "VNEHC_APM.h"

VNEHC_APM dust(&Wire, 0x08);

VNEHC_APM::Data data;

void setup() {
  Serial.begin(115200);

  dust.begin();

  dust.startMeasurement();
}

void loop() {
  if (dust.read(&data)) {
    Serial.println("====== APM2000 ======");

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
  } else {
    Serial.println("Read failed!");
  }

  delay(1000);
}
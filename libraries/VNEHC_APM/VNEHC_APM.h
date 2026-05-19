#ifndef _VNEHC_APM_H_
#define _VNEHC_APM_H_

#include <Arduino.h>
#include <Wire.h>

class VNEHC_APM
{
public:

    enum InterfaceType
    {
        INTERFACE_I2C,
        INTERFACE_UART
    };

    struct Data
    {
        uint16_t pm1_0;
        uint16_t pm2_5;
        uint16_t pm10;

        float humidity;
        float temperature;
    };

    VNEHC_APM(TwoWire *wire = &Wire, uint8_t addr = 0x08);
    VNEHC_APM(Stream *serial);

    bool begin();

    bool startMeasurement();

    bool stopMeasurement();

    bool read(Data *data);

private:
    TwoWire *_wire;

    Stream *_serial;

    InterfaceType _interface;

    uint8_t _addr;

private:
    uint8_t crc8(uint8_t *data, uint8_t len);

    //////////////////////////////////////////////////////
    // Interface specific

    bool readI2C(Data *data);

    bool readUART(Data *data);
};

#endif
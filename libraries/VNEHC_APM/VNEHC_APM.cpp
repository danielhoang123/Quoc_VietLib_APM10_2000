#include "VNEHC_APM.h"

////

VNEHC_APM::VNEHC_APM(TwoWire *wire, uint8_t addr)
{
    _interface = INTERFACE_I2C;

    _wire = wire;

    _addr = addr;

    _serial = nullptr;
}

VNEHC_APM::VNEHC_APM(Stream *serial)
{
    _interface = INTERFACE_UART;

    _serial = serial;

    _wire = nullptr;

    _addr = 0;
}

bool VNEHC_APM::begin()
{

    _wire->begin();

    return true;
}

////

uint8_t VNEHC_APM::crc8(uint8_t *data, uint8_t len)
{
    uint8_t crc = 0xFF;

    for (uint8_t i = 0; i < len; i++)
    {
        crc ^= data[i];

        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x80)
            {
                crc = (crc << 1) ^ 0x31;
            }
            else
            {
                crc <<= 1;
            }
        }
    }

    return crc;
}

////

bool VNEHC_APM::startMeasurement()
{
    if (_interface == INTERFACE_I2C)
    {
        _wire->beginTransmission(_addr);

        _wire->write(0x00);
        _wire->write(0x10);

        _wire->write(0x05);
        _wire->write(0x00);
        _wire->write(0xF6);

        return (_wire->endTransmission() == 0);
    }

    else if (_interface == INTERFACE_UART)
    {

        return true;
    }

    return false;
}

////

bool VNEHC_APM::stopMeasurement()
{
    if (_interface == INTERFACE_I2C)
    {
        _wire->beginTransmission(_addr);

        _wire->write(0x01);
        _wire->write(0x04);

        return (_wire->endTransmission() == 0);
    }

    else if (_interface == INTERFACE_UART)
    {

        return true;
    }

    return false;
}

////

bool VNEHC_APM::read(Data *data)
{
    if (_interface == INTERFACE_I2C)
    {
        return readI2C(data);
    }

    else if (_interface == INTERFACE_UART)
    {
        return readUART(data);
    }

    return false;
}

bool VNEHC_APM::readI2C(Data *data)
{
    uint8_t rx[30];

    _wire->beginTransmission(_addr);

    _wire->write(0x03);
    _wire->write(0x00);

    if (_wire->endTransmission(false) != 0)
    {
        return false;
    }

    uint8_t len =
        _wire->requestFrom(_addr, (uint8_t)30);

    if (len != 30)
    {
        return false;
    }

    for (int i = 0; i < 30; i++)
    {
        rx[i] = _wire->read();
    }

    if (crc8(&rx[0], 2) != rx[2])
    {
        return false;
    }

    if (crc8(&rx[3], 2) != rx[5])
    {
        return false;
    }

    if (crc8(&rx[9], 2) != rx[11])
    {
        return false;
    }

    data->pm1_0 =
        ((uint16_t)rx[0] << 8) | rx[1];

    data->pm2_5 =
        ((uint16_t)rx[3] << 8) | rx[4];

    data->pm10 =
        ((uint16_t)rx[9] << 8) | rx[10];

    uint16_t hum_raw =
        ((uint16_t)rx[12] << 8) | rx[13];

    data->humidity =
        hum_raw / 10.0f;

    uint16_t temp_raw =
        ((uint16_t)rx[15] << 8) | rx[16];

    if (temp_raw < 500)
    {
        data->temperature =
            -(500 - temp_raw) / 10.0f;
    }
    else
    {
        data->temperature =
            (temp_raw - 500) / 10.0f;
    }

    return true;
}

bool VNEHC_APM::readUART(Data *data)
{
    static uint8_t rx[11];
    static uint8_t index = 0;

    static bool waitingResponse = false;

    static uint32_t requestTime = 0;

    // send request

    if (!waitingResponse)
    {
        // clear old buffer
        while (_serial->available())
        {
            _serial->read();
        }

        uint8_t cmd[] =
            {
                0xFE,
                0xA5,
                0x00,
                0x01,
                0xA6};

        _serial->write(cmd, sizeof(cmd));

        waitingResponse = true;

        requestTime = millis();

        index = 0;

        return false;
    }

    if (millis() - requestTime > 5000)
    {
        waitingResponse = false;
        index = 0;

        return false;
    }

    // read incoming bytes

    while (_serial->available())
    {
        uint8_t b = _serial->read();

        // sync header

        if (index == 0 && b != 0xFE)
        {
            continue;
        }

        if (index == 1 && b != 0xA5)
        {
            index = 0;
            continue;
        }

        rx[index++] = b;

        // full frame received

        if (index >= 11)
        {
            waitingResponse = false;
            index = 0;

            // checksum

            uint8_t checksum = 0;

            for (uint8_t i = 1; i < 10; i++)
            {
                checksum += rx[i];
            }

            checksum &= 0xFF;

            if (checksum != rx[10])
            {
                return false;
            }

            // parse data

            data->pm1_0 =
                ((uint16_t)rx[4] << 8) | rx[5];

            data->pm2_5 =
                ((uint16_t)rx[6] << 8) | rx[7];

            data->pm10 =
                ((uint16_t)rx[8] << 8) | rx[9];

            data->humidity = 0;
            data->temperature = 0;

            return true;
        }
    }

    return false;
}
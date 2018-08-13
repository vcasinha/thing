#ifndef DHTDEVICE_H
#define DHTDEVICE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <dht.h>
#include "Device.h"


class DHTDevice : public Device
{
  public:
    float _humidity;
    float _temperature;
    dht _dht;
    unsigned int _pin;
    unsigned int _status;

    DHTDevice()
    {
        this->_type = "sensor";
    }

    virtual void config(JsonObject &config)
    {
        this->_pin = config["pin"] | 0;
        Serial.printf("DHT sensor %s on %s PIN:%d\n", this->_id.c_str(), this->_location.c_str(), this->_pin);
    }

    virtual void loop()
    {
        DynamicJsonBuffer buffer;
        JsonObject &json = buffer.createObject();
        String state;

        this->_humidity = this->_dht.humidity;
        this->_temperature = this->_dht.temperature;
        this->_status = this->_dht.read11(this->_pin);

        switch (this->_status)
        {
            case DHTLIB_OK:
                // DISPLAY DATA
                Serial.printf("Humidity %.2f%% Temperature %.2fº\n", this->_humidity, this->_temperature);
                json["temperature"] = this->_temperature;
                json["humidity"] = this->_humidity;
                json.printTo(state);
                this->publishState(state.c_str());
                break;
            case DHTLIB_ERROR_CHECKSUM:
                Serial.printf("* Checksum error\n");
                break;
            case DHTLIB_ERROR_TIMEOUT:
                Serial.printf("* Time out\n");
                break;
            default:
                Serial.printf("* Unknown error\n");
                break;
            }
    }
};

#endif
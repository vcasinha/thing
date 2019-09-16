#ifndef DHTDEVICE_H
#define DHTDEVICE_H

#include <Arduino.h>
#include <ArduinoLog.h>
#include <ArduinoJson.h>
#include "DHTesp.h"
#include "Device.h"

class DHTDevice : public Device
{
    public:
        float _humidity;
        float _temperature;
        DHTesp _dht;
        unsigned int _pin;
        unsigned int _status;
        /** Comfort profile */
        ComfortState _cf;

        DHTDevice()
        {
            this->_type = "sensor";
            Log.notice("(DHT.init) Update period %u", this->_updatePeriod);
        }

        virtual void config(JsonObject &config)
        {
            this->_pin = config["pin"] | 0;
            this->_dht.setup(this->_pin, DHTesp::AUTO_DETECT);
            Log.notice("(DHT.config) DHT on pin %u", this->_pin);
        }

        virtual void loop()
        {
            DynamicJsonDocument buffer(512);
            JsonObject json = buffer.to<JsonObject>();
            String state;

            TempAndHumidity newValues = this->_dht.getTempAndHumidity();
            if (this->_dht.getStatus() != 0)
            {
                Log.error("(DHT) Error status: %s", this->_dht.getStatusString());
                return;
            }

            this->_temperature = newValues.temperature;
            this->_humidity = newValues.humidity;
            /*
            float heatIndex = this->_dht.computeHeatIndex(newValues.temperature, newValues.humidity);
            float dewPoint = this->_dht.computeDewPoint(newValues.temperature, newValues.humidity);
            float cr = this->_dht.getComfortRatio(this->_cf, newValues.temperature, newValues.humidity);

            String comfortStatus;
            switch (this->_cf)
            {
            case Comfort_OK:
                comfortStatus = "OK";
                break;
            case Comfort_TooHot:
                comfortStatus = "Too Hot";
                break;
            case Comfort_TooCold:
                comfortStatus = "Too Cold";
                break;
            case Comfort_TooDry:
                comfortStatus = "Too Dry";
                break;
            case Comfort_TooHumid:
                comfortStatus = "Too Humid";
                break;
            case Comfort_HotAndHumid:
                comfortStatus = "Hot And Humid";
                break;
            case Comfort_HotAndDry:
                comfortStatus = "Hot And Dry";
                break;
            case Comfort_ColdAndHumid:
                comfortStatus = "Cold And Humid";
                break;
            case Comfort_ColdAndDry:
                comfortStatus = "Cold And Dry";
                break;
            default:
                comfortStatus = "Unknown:";
                break;
            };

            json["comfort"] = comfortStatus;
            json["comfortRatio"] = cr;
            json["dewPoint"] = dewPoint;
            json["headIndex"] = heatIndex;
            */

            json["temperature"] = this->_temperature;
            json["humidity"] = this->_humidity;

            serializeJson(buffer, state);
            this->publishState(state.c_str());
        }
};

#endif
#ifndef SWITCH_H
#define SWITCH_H

#include <Arduino.h>
#include "Device.h"

class Switch final: public Device
{
    public:
        unsigned int _pin;

        Switch()
        {
            this->_type = "switch";
            this->_updatePeriod = 60;
        }

        ~Switch()
        {

        }

        virtual void deviceConfig(JsonObject &config)
        {
            this->_pin = config["pin"];
            pinMode(this->_pin, OUTPUT);
        }

        void updateState(bool state)
        {
            if (state != this->_state)
            {
                this->_state = state;
                digitalWrite(this->_pin, this->_state);
            }

            this->publishState(this->_state ? "ON" : "OFF");
        }

        virtual void onCommand(String data)
        {
            this->updateState(data.equals("ON"));
            Serial.printf("Change switch %s state to %s\n", this->_id.c_str(), this->_state ? "ON" : "OFF");
        }

        virtual void loop()
        {
            this->publishState(this->_state ? "ON" : "OFF");
        }
};

#endif
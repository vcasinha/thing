#ifndef BLINK_H
#define BLINK_H

#include <Arduino.h>
#include "Device.h"

class Blink final: public Device
{
    public:
        unsigned int _pin = 0;
        unsigned int _blinkTime = 0;
        boolean _blinkState = false;

        Blink()
        {
            this->_type = "blink";
        }

        virtual void deviceConfig(JsonObject & config)
        {
            Serial.printf("(Blink) Init '%s' on pin %d (topic: '%s')\n", this->_id.c_str(), this->_pin, this->_commandTopic);
            this->_pin = config["pin"];
            if(this->_pin)
                pinMode(this->_pin, OUTPUT);
        }

        virtual void onCommand(String data)
        {
            Serial.printf("(Blink) Change state '%s' to '%s'\n", this->_id.c_str(), data.c_str());
            bool state = data.equals("ON");
            this->setState(state);
        }

        virtual void loop()
        {
            if(this->_state == false)
            {
                return;
            }

            this->_blinkState = !this->_blinkState;

            digitalWrite(this->_pin, this->_blinkState);
        }
};

#endif
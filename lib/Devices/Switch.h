#ifndef SWITCH_H
#define SWITCH_H

#include <Arduino.h>
#include "Device.h"

class Switch : public Device
{
  public:
    unsigned int _pin;
    Switch()
    {
        this->_type = "switch";
    }

    virtual void config(JsonObject &config)
    {
        this->_pin = config["pin"];
    }

    virtual void setup()
    {
        pinMode(this->_pin, OUTPUT);
        Serial.printf("Switch '%s' on pin %d topic %s\n", this->_id.c_str(), this->_pin, this->_commandTopic);
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
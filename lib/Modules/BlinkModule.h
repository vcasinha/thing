#ifndef BLINK_MODULE_H
#define BLINK_MODULE_H

#include <Arduino.h>
#include <WiFiManager.h>
#include "Module.h"
#include "WiFiModule.h"

class BlinkModule : public Module
{
  public:
    unsigned long _time = 0;
    bool _state = false;
    unsigned int _pin = D5;

    BlinkModule()
    {
        this->_name = "blink";
        this->_loop_period_ms = 1000;
    }

    virtual void boot(JsonObject &config)
    {
        pinMode(this->_pin, OUTPUT);
    }

    virtual void loop(void)
    {
        this->_state = !this->_state;
        digitalWrite(this->_pin, this->_state);
        Serial.printf("Blink %d\n", this->_state);
    }
};

#endif
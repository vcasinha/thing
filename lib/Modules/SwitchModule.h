#ifndef SWITCH_MODULE_H
#define SWITCH_MODULE_H

#include <Arduino.h>
#include "DeviceManagerModule.h"
#include "Switch.h"

class SwitchModule : public DeviceManagerModule
{
    public:
        SwitchModule()
        {
            this->_name = "switches";
            this->_loop_period_ms = 60 * 1000;
        }

        virtual Device * makeDevice()
        {
            return new Switch();
        }
};
#endif
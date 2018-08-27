#ifndef SWITCH_MODULE_H
#define SWITCH_MODULE_H

#include <Arduino.h>
#include "DeviceFactory.h"
#include "Switch.h"

class SwitchFactory : public DeviceFactory
{
    public:
        SwitchFactory()
        {
            this->_deviceType = "switch";
        }

        virtual Device * makeDevice()
        {
            return new Switch();
        }
};
#endif
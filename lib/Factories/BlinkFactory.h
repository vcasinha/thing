#ifndef BLINK_MODULE_H
#define BLINK_MODULE_H

#include <Arduino.h>
#include "DeviceFactory.h"
#include "Blink.h"

class BlinkFactory : public DeviceFactory
{
    public:
        BlinkFactory()
        {
            this->_deviceType = "blink";
        }

        virtual Device * makeDevice()
        {
            return new Blink();
        }
};
#endif
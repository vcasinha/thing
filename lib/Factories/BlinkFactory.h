#ifndef BLINK_MODULE_H
#define BLINK_MODULE_H

#include <Arduino.h>
#include "DeviceFactory.h"
#include "Blink.h"

class BlinkFactory : public UnitFactory
{
    public:
        BlinkFactory()
        {
            this->_unitType = "blink";
        }

        virtual Device * make()
        {
            return new Blink();
        }
};
#endif
#ifndef DHTFACTORY_H
#define DHTFACTORY_H

#include <Arduino.h>
#include "DeviceFactory.h"
#include "DHTDevice.h"

class DHTFactory: public DeviceFactory
{
    public:
        DHTFactory()
        {
            this->_deviceType = "dht";
        }

        virtual Device * makeDevice()
        {
            return new DHTDevice();
        }
};

#endif
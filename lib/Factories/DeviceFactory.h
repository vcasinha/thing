#ifndef DEVICEFACTORY_H
#define DEVICEFACTORY_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "Device.h"

class DeviceFactory
{
    public:
        String _deviceType;

        virtual Device * makeDevice()
        {
            return new Device();
        }
};

#endif
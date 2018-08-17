#include <Arduino.h>
#include <ArduinoJson.h>
#include "DeviceManagerModule.h"
#include "DHTDevice.h"


class DHTModule: public DeviceManagerModule 
{
    public:
        DHTModule()
        {
            this->_name = "dht";
            this->_loop_period_ms = 60 * 1000;
        }

        virtual Device * makeDevice()
        {
                return new DHTDevice();
        }
};

#ifndef WIFI_MODULE_H
#define WIFI_MODULE_H

#include <Arduino.h>
#include "Module.h"

class WiFiModule : public Module
{
    public:

        WiFiModule()
        {
            _name = "wifi";
        }

        void setup(void)
        {
        }
        
    private:

};

#endif
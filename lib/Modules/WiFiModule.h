#ifndef WIFI_MODULE_H
#define WIFI_MODULE_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "Module.h"

class ServerModule;
class WiFiModule : public Module
{
    public:
        WiFiClient _wifiClient;

        WiFiModule()
        {
            _name = "wifi";
        }

        void boot(void)
        {

        }

        void setup(void)
        {
        }

        void loop(unsigned long delta_time)
        {
            //Check if still connected to WIFI
            //Start AP when necessary
        }
    private:

};

#endif
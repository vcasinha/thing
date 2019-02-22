#ifndef WIFI_MODULE_H
#define WIFI_MODULE_H

#include <Arduino.h>
#include "PersWiFiManager.h"
#include "Module.h"
#include "ServerModule.h"
#include "DeviceModule.h"

class ServerModule;
class WiFiModule : public Module
{
    public:
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
        
    private:

};

#endif
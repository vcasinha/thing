#ifndef WIFI_MODULE_H
#define WIFI_MODULE_H

    #include <Arduino.h>
    #include <WiFiManager.h>
    #include "Module.h"

    class WiFiModule : public Module 
    {
        public:
            WiFiManager _wifiManager;
            WiFiClient _wirelessClient;

        WiFiModule()
        {
            _name = "WiFi";

        }

        void setup(void)
        {
            _wifiManager.setConfigPortalTimeout(180);
            _wifiManager.autoConnect();
        }
    };

#endif
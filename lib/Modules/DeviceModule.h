#ifndef DEVICE_MODULE_H
    #define DEVICE_MODULE_H

    #include <Arduino.h>
    #include <WiFiManager.h>
    #include "Module.h"
    #include "WiFiModule.h"

    class DeviceModule : public Module
    {
        public:
            WiFiModule *_wifiModule;
            String _boardID;
            String _hostname;
            String _location;

            DeviceModule()
            {
                this->_name = "device";
                char tmp[10];
                sprintf(tmp, "ESP%6X", ESP.getFlashChipId());
                this->_boardID = tmp;
            }

            ~DeviceModule()
            {
            }

            virtual void boot(JsonObject &config)
            {
                this->_wifiModule = (WiFiModule *)this->_application->getModule("wifi");
                this->_hostname = config["name"] | this->_boardID.c_str();
                this->_location = config["location"] | "somewhere";
            }

            void restart()
            {
                Serial.printf("Restarting...\n");
                delay(1000);
                ESP.restart();
                delay(5000);
            }
    };

#endif
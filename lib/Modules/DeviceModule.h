#ifndef DEVICE_MODULE_H
    #define DEVICE_MODULE_H

    #include <Arduino.h>
    #include <WiFiManager.h>
    #include "Module.h"
    #include "WiFiModule.h"

    class DeviceModule : public Module 
    {
        public:
            WiFiModule * _wifiModule;

            String _boardID;
            String _hostname;
            String _location;

            DeviceModule()
            {
                this->_name = "device";
                char tmp[10]; sprintf(tmp, "DEV-%6X", ESP.getFlashChipId());
                this->_boardID = tmp;
            }

            ~DeviceModule()
            {
            }

            virtual void boot(JsonObject & config)
            {
                this->_wifiModule = (WiFiModule *) this->_application->getModule("wifi");
                Serial.printf("Boot module XXX %s\n", this->_wifiModule->_name);
                this->_wifiModule->addParameterHTML("<p>Device</p>");
                this->_wifiModule->addParameter("device_hostname", "hostname", config["hostname"] | this->_boardID.c_str(), 20);
                this->_wifiModule->addParameter("device_location", "location", config["location"] | "somewhere", 20);
            }

            virtual void setup(void)
            {
                Serial.printf("Setup module XXX %s\n", this->_name);
                this->_wifiModule->getParameter("device_hostname");
                this->_hostname = this->_wifiModule->getParameter("device_hostname")->getValue();
                this->_location = this->_wifiModule->getParameter("device_location")->getValue();
            }
    };

#endif
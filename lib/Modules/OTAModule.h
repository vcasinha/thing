#ifndef OTA_MODULE_H
    #define OTA_MODULE_H
    #define NO_GLOBAL_ARDUINOOTA

    #include <Arduino.h>
    #include <ArduinoOTA.h>
    #include <WiFiManager.h>
    #include "Module.h"
    #include "WiFiModule.h"

    class OTAModule : public Module
    {
        public:
            WiFiModule * _wifiModule;
            ArduinoOTAClass * _controller;

            Vector<Module *> _callbacks;
            const char * _hostname;
            const char * _password;

            OTAModule()
            {
                this->_name = "ota";
                this->_controller = new ArduinoOTAClass();
            }

            ~OTAModule()
            {
            }

            virtual void boot(JsonObject & config)
            {
                this->_wifiModule = (WiFiModule *) this->_application->getModule("wifi");

                this->_wifiModule->addParameterHTML("<p>Firmware OTA</p>");
                this->_wifiModule->addParameter("ota_password", "password", "", 20);

                //Configure callbacks
                this->_controller->onStart([this](){this->onStart();});
                this->_controller->onEnd([this](){this->onStart();});
                this->_controller->onProgress([this](unsigned int progress, unsigned int total){this->onProgress(progress, total);});
                this->_controller->onError([this](ota_error_t error){this->onError(error);});
            }

            virtual void setup(void)
            {
                this->_hostname = this->_wifiModule->getParameterValue("device_hostname");
                this->_password = this->_wifiModule->getParameterValue("ota_password");
                this->_controller->begin();
                this->_controller->setHostname(this->_hostname);
                this->_controller->setPassword(this->_password);
            }

            virtual void loop(void)
            {
                this->_controller->handle();
            }

            void onStart(void)
            {
                Serial.printf("(OTA) Start\n");
            }

            void onEnd(void)
            {
                Serial.printf("(OTA) End\n");
            }

            void onError(ota_error_t error)
            {
                Serial.printf("(OTA) Error #%u\n", error);
            }

            void onProgress(unsigned int progress, unsigned int total)
            {
                Serial.printf("(OTA) Progress %u of %u\n", progress, total);
            }
    };

#endif
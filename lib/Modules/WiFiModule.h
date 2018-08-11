#ifndef WIFI_MODULE_H
#define WIFI_MODULE_H

    #include <Arduino.h>
    #include <WiFiManager.h>
    #include "Module.h"

    class WiFiModule : public Module 
    {
        public:
            WiFiManager * _wifiManager;
            WiFiClient _wirelessClient;

            WiFiModule()
            {
                _name = "wifi";
                this->_wifiManager = new WiFiManager();
            }

            void setup(void)
            {
                this->_wifiManager->setConfigPortalTimeout(180);
                this->_wifiManager->autoConnect();
            }

            void addParameter(const char * id, const char * label, const char * value, unsigned int length)
            {
                //Serial.printf("Add parameter %s\n", id);
                WiFiManagerParameter * parameter = new WiFiManagerParameter(id, label, value, length);
                this->_wifiManager->addParameter(parameter);
                this->_parameters.push(parameter);
                //Serial.printf("Total %d\n", this->_parameters.size());
            }

            void addParameterHTML(const char * html)
            {
                //Serial.printf("Add HTML %s\n", html);
                WiFiManagerParameter * parameter = new WiFiManagerParameter(html);
                this->_wifiManager->addParameter(parameter);
            }

            WiFiManagerParameter * getParameter(const char * parameter_id, const char * default_value = "")
            {
                //Serial.printf("Find parameter %s %s\n", parameter_id, this->_name);
                for(unsigned int c = 0;c < this->_parameters.size();c++)
                {
                    WiFiManagerParameter * parameter = this->_parameters[c];
                    String p = parameter->getID();
                    if(p.equals(parameter_id))
                    {
                        return parameter;
                    }
                }

                Serial.printf("Not found parameter %s\n", parameter_id);

                return new WiFiManagerParameter("<p>none</p>");
            }
        private:
            Vector<WiFiManagerParameter *> _parameters;
    };

#endif
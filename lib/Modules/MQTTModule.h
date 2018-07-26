#ifndef MQTT_MODULE_H
#define MQTT_MODULE_H

    #include <Arduino.h>
    #include <ESP8266WiFi.h>
    #include <PubSubClient.h>
    #include <WiFiManager.h>
    #include "WiFiModule.h"
    
    #include "Module.h"

    #define MQTT_HOSTNAME "0.0.0.0"
    #define MQTT_USERNAME "none"
    #define MQTT_PASSWORD "none"

    class MQTTModule : public Module 
    {
        public:
            PubSubClient _client;
            WiFiModule * _wifiModule;
            
            const char * _hostname;
            const char * _username;
            const char * _password;

            Vector<Module *> _callbacks;

            WiFiManagerParameter _mqtt_hostname = WiFiManagerParameter("hostname", "mqtt hostname", "0.0.0.0", 40);
            WiFiManagerParameter _mqtt_username = WiFiManagerParameter("username", "mqtt username", "none", 20);
            WiFiManagerParameter _mqtt_password = WiFiManagerParameter("password", "mqtt password", "none", 20);

        MQTTModule(const char * hostname, const char * username, const char * password)
        {
            _name = "MQTT";
            _mqtt_hostname = WiFiManagerParameter("hostname", "mqtt hostname", hostname, 40);
            _mqtt_username = WiFiManagerParameter("username", "mqtt username", username, 20);
            _mqtt_password = WiFiManagerParameter("password", "mqtt password", password, 20);
        }

        virtual void boot(void)
        {
            this->_wifiModule = (WiFiModule *) this->_application->getModule("WiFi");
            this->_wifiModule->_wifiManager.addParameter(&_mqtt_hostname);
            this->_wifiModule->_wifiManager.addParameter(&_mqtt_username);
            this->_wifiModule->_wifiManager.addParameter(&_mqtt_password);

            _client.setClient(this->_wifiModule->_wirelessClient);
        }

        virtual void setup(void)
        {
            _client.setCallback([this] (char * topic, unsigned char * payload, unsigned int length) { this->callback(topic, payload, length); });
            this->connect();
            this->_client.subscribe("test");
        }

        virtual void loop(void)
        {
            this->_client.loop();
            this->connect();
        }

        void connect(void)
        {
            if(WiFi.status() == WL_CONNECTED)
            {
                // Loop until we're reconnected to the MQTT server
                unsigned int counter = 0;
                while (_client.connected() == false) 
                {
                    Serial.printf("Fetching MQTT settings from WiFiManager\n");
                    _hostname = _mqtt_hostname.getValue();
                    _username = _mqtt_username.getValue();
                    _password = _mqtt_password.getValue();

                    Serial.printf("Connect to MQTT %s as '%s' : '%s' attempt %d\n", _hostname, _username, _password, counter);
                    _client.setServer(_hostname, 1883);
                    _client.connect(_hostname, _username, _password);
                    
                    if(_client.connected() == false && counter > 3)
                    {
                        Serial.printf("Could not connect to MQTT at %s\n", _hostname);
                        abort();
                    }
                    counter++;
                }
            }
        }

        void registerCallback(Module * module)
        {
            Serial.printf("Module %s requested MQTT callback", module->_name);
            _callbacks.push(module);
        }

        void callback(char * topic, unsigned char * payload, unsigned int length)
        {
            Serial.printf("Seriously??? :) topic %s\n", topic);
            for(unsigned int c = 0;c < _callbacks.size();c++)
            {
                _callbacks[c]->callback(topic, payload, length);
            }
        }
    };

#endif
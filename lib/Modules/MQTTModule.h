#ifndef MQTT_MODULE_H
#define MQTT_MODULE_H

    #include <Arduino.h>
    #include <ESP8266WiFi.h>
    #include <PubSubClient.h>
    #include <WiFiManager.h>
    #include "WiFiModule.h"
    

    #include "Module.h"

    class MQTTModule : public Module 
    {
        public:
            PubSubClient _client;
            WiFiClient _wirelessClient;
            std::function<void(char *, uint8_t *, unsigned int)> _callbacks[];
            const char * _hostname = "192.168.0.200";
            const char * _username = "mqtt";
            const char * _password = "mqtt";
            WiFiManagerParameter _mqtt_hostname = WiFiManagerParameter("hostname", "mqtt hostname", "192.168.0.200", 40);
            WiFiManagerParameter _mqtt_username = WiFiManagerParameter("username", "mqtt username", "mqtt", 20);
            WiFiManagerParameter _mqtt_password = WiFiManagerParameter("password", "mqtt password", "mqtt", 20);

        MQTTModule()
        {
            _name = "MQTT";
        }

        virtual void boot(void)
        {
            WiFiModule * wifiModule = (WiFiModule *) this->_application->getModule("WiFi");
            wifiModule->_wifiManager.addParameter(&_mqtt_hostname);
            wifiModule->_wifiManager.addParameter(&_mqtt_username);
            wifiModule->_wifiManager.addParameter(&_mqtt_password);
        }

        virtual void setup(void)
        {
            _client.setClient(this->_wirelessClient);
            _client.setServer(_hostname, 1883);
            _client.setCallback([this] (char * topic, unsigned char * payload, unsigned int length) { this->callback(topic, payload, length); });
            this->connect();
            this->_client.subscribe("test");
        }

        virtual void loop(void)
        {
            this->_client.loop();
        }

        void connect(void)
        {
            if(WiFi.status() == WL_CONNECTED)
            {
                // Loop until we're reconnected to the MQTT server
                while (_client.connected() == false) 
                {            
                    if (_client.connect(_hostname, _username, _password))
                    {
                        Serial.printf("Connected to MQTT");
                    }
                    else
                    {
                        Serial.printf("Could not connect to MQTT at %s\n", _hostname);
                        abort();
                    }
                }
            }
        }

        void callback(char * topic, unsigned char * payload, unsigned int length)
        {
            Serial.printf("Seriously??? :)");
        }
    };

#endif
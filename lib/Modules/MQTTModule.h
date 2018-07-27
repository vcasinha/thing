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
            WiFiModule * _wifiModule;
            unsigned int _counter = 0;

            const char * _hostname;
            const char * _username;
            const char * _password;

            Vector<Module *> _callbacks;

            WiFiManagerParameter _mqtt_hostname = WiFiManagerParameter("mqtt_hostname", "mqtt hostname", "0.0.0.0", 40);
            WiFiManagerParameter _mqtt_username = WiFiManagerParameter("mqtt_username", "mqtt username", "none", 20);
            WiFiManagerParameter _mqtt_password = WiFiManagerParameter("mqtt_password", "mqtt password", "none", 20);

        MQTTModule(const char * hostname, const char * username, const char * password)
        {
            _name = "MQTT";
            _mqtt_hostname = WiFiManagerParameter("hostname", "mqtt hostname", hostname, 40);
            _mqtt_username = WiFiManagerParameter("username", "mqtt username", username, 20);
            _mqtt_password = WiFiManagerParameter("password", "mqtt password", password, 20);
        }

        ~MQTTModule()
        {
            String topic = String("home/") + this->_application->_id + String("/state");
            this->_client.publish(topic.c_str(), "OFF");
        }

        virtual void boot(JsonObject & config)
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
            String topic = String("home/") + this->_application->_id + String("/state");
            this->_client.publish(topic.c_str(), "ON");
        }

        virtual void loop(void)
        {
            this->connect();
            this->_client.loop();
        }

        void connect(void)
        {
            if(WiFi.status() == WL_CONNECTED)
            {
                while (this->_client.connected() == false) 
                {
                    Serial.printf("Fetching MQTT settings from WiFiManager\n");
                    _hostname = _mqtt_hostname.getValue();
                    _username = _mqtt_username.getValue();
                    _password = _mqtt_password.getValue();

                    Serial.printf("Connect to MQTT %s as '%s' : '%s' attempt %d\n", _hostname, _username, _password, _counter);
                    this->_client.setServer(_hostname, 1883);
                    this->_client.connect(_hostname, _username, _password);
                    
                    if(this->_client.connected() == false)
                    {
                        _counter++;
                        Serial.printf("Could not connect to MQTT at %s\n", _hostname);
                        if(_counter > 5)
                        {
                            abort();
                        }
                    }
                }
            }
        }

        void registerCallback(Module * module)
        {
            Serial.printf("Module %s requested MQTT callback\n", module->_name);
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

        void publish(const char * topic, const char * payload)
        {
            this->_client.publish(topic, payload);
        }

        void subscribe(const char * topic)
        {
            this->_client.subscribe(topic);
        }
    };

#endif
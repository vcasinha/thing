#ifndef DEVICEMANAGER_MODULE_H
#define DEVICEMANAGER_MODULE_H

#include <Arduino.h>
#include <WiFiManager.h>
#include "Module.h"
#include "MQTTModule.h"
#include "Vector.h"
#include "Switch.h"

class DeviceManagerModule : public Module
{
    public:
        MQTTModule *_mqtt;
        bool _state = false;
        unsigned int _pin = 0;
        Vector<Device *> _devices;
        

        DeviceManagerModule()
        {
        }

        virtual Device * makeDevice() {
            Serial.printf("Something went wrong with makeDevice\n");
            return new Device();
        }

        virtual void boot(JsonObject &config)
        {
            this->_mqtt = (MQTTModule *)this->_application->getModule("mqtt");
            this->_mqtt->registerCallback(this);
            if (config["devices"])
            {
                Serial.printf("Booting %d devices\n", config["devices"].size());
                for (unsigned int i = 0; i < config["devices"].size(); i++)
                {
                    Device *device = this->makeDevice();
                    device->boot(config["devices"][i]);
                    this->_devices.push(device);
                }
            }
        }

        virtual void setup(void)
        {
            //Serial.printf("Setup devices\n");
            for (unsigned int i = 0; i < this->_devices.size(); i++)
            {
                this->_devices[i]->setMQTT(this->_mqtt);
            }
        }

        void callback(char *topic, unsigned char *payload, unsigned int length)
        {
            String topic_string = topic;
            payload[length] = '\0';
            String data = (char *)payload;

            Serial.printf("Topic: %s Data: %s\n", topic_string.c_str(), data.c_str());
            for (unsigned int i = 0; i < this->_devices.size(); i++)
            {
                this->_devices[i]->callback(topic_string, data);
            }
        }

        virtual void loop(void)
        {
            for (unsigned int i = 0; i < this->_devices.size(); i++)
            {
                this->_devices[i]->loop();
            }
        }
};
#endif
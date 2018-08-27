#ifndef DEVICEMANAGER_MODULE_H
#define DEVICEMANAGER_MODULE_H

#include <Arduino.h>
#include <WiFiManager.h>
#include "DeviceFactory.h"
#include "DHTFactory.h"
#include "SwitchFactory.h"
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
        Vector<DeviceFactory *> _factories;

        DeviceManagerModule()
        {
            this->_name = "device_manager";
            this->_loop_period_ms = 500;

            this->addFactory(new DHTFactory());
            this->addFactory(new SwitchFactory());
        }

        void addFactory(DeviceFactory * factory)
        {
            Serial.printf("Add factory of %s devices.\n", factory->_deviceType.c_str());
            this->_factories.push(factory);
        }

        void makeDevice(JsonObject & config) 
        {
            bool made = false;
            for(unsigned int i = 0;i < this->_factories.size();i++)
            {
                //Serial.printf("* Checking factory %s vs %s\n", this->_factories[i]->_deviceType.c_str(), config["type"] | "undefined");
                if (this->_factories[i]->_deviceType.equals(config["type"] | "undefined"))
                {
                    made = true;
                    //Serial.printf("Device %s@%s type %s on pin %d\n", config["id"] | "", config["location"] | "here", config["type"] | "", config["pin"] | 0);
                    Device * device = this->_factories[i]->makeDevice();
                    device->boot(config);
                    this->addDevice(device);
                }
            }

            if(made == false)
            {
                Serial.printf("### Device factory not found for %s type.\n", config["type"] | "");
            }
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
                     this->makeDevice(config["devices"][i]);
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

        void addDevice(Device * device)
        {
            //Serial.printf("Adding device %s %s\n", device->_id.c_str(), device->_type.c_str());
            this->_devices.push(device);
        }

        void callback(char *topic, unsigned char *payload, unsigned int length)
        {
            String topic_string = topic;
            payload[length] = '\0';
            String data = (char *)payload;

            Serial.printf("Device callback on topic '%s' Data: %s\n", topic_string.c_str(), data.c_str());
            for (unsigned int i = 0; i < this->_devices.size(); i++)
            {
                this->_devices[i]->callback(topic_string, data);
            }
        }

        virtual void loop(void)
        {
            for (unsigned int i = 0; i < this->_devices.size(); i++)
            {
                Device * device = this->_devices[i];
                unsigned long current_time = millis();

                if (device->_update_period == 0 || (current_time - device->_last_update) > (device->_update_period * 1000))
                {
                    device->loop();
                    device->_last_update = current_time;
                }
            }
        }
};
#endif
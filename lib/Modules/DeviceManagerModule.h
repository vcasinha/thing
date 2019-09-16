#ifndef DEVICEMANAGER_MODULE_H
#define DEVICEMANAGER_MODULE_H

#include <Arduino.h>
#include <ArduinoLog.h>
#include "DeviceFactory.h"
//#include "DHTFactory.h"
//#include "SwitchFactory.h"
//#include "BlinkFactory.h"
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
            this->_loop_period_ms = 20;
            //this->addFactory(new BlinkFactory());
        }

        void addFactory(DeviceFactory *factory)
        {
            Log.notice("(deviceManager.addFactory) Load factory of %s devices.", factory->_deviceType.c_str());
            this->_factories.push(factory);
        }

        void makeDevice(JsonObject config)
        {
            bool made = false;
            String device_type = config["type"].as<String>();
            String json_config = "";
            serializeJson(config, json_config);
            Log.notice("(deviceManager.makeDevice) Init '%s' device (%s)", device_type.c_str(), json_config.c_str());

            for (unsigned int i = 0; i < this->_factories.size(); i++)
            {
                if (device_type.equals(""))
                {
                    Log.error("(deviceManager.makeDevice) ERROR undefined device type");
                    continue;
                }

                if (this->_factories[i]->_deviceType.equals(device_type))
                {
                    Device *device = this->_factories[i]->makeDevice();
                    device->boot(config);
                    this->_devices.push(device);

                    made = true;
                }
            }

            if (made == false)
            {
                Log.error("(deviceManager.makeDevice) ERROR Unknown device type '%s'.", device_type.c_str());
            }
        }

        void config(JsonObject & config)
        {
            Log.notice("Device manager");
            serializeJson(config, Serial);
            if (config["devices"])
            {
                for (unsigned int i = 0; i < this->_devices.size(); i++)
                {
                    Device * device = this->_devices[i];
                    if (config["devices"][device->_id])
                    {
                        JsonObject device_config = config["devices"][device->_id];
                        String json_config = "";
                        serializeJson(device_config, json_config);
                        Log.notice("(deviceManager.config) Update device configuration '%s' ()", device->_id.c_str(), json_config.c_str());
                        this->_devices[i]->config(device_config);
                    }
                }
            }
        }

        virtual void boot(JsonObject & config)
        {
            if (config.size() == 0)
            {
                Log.warning("(MQTT) Empty configuration, disabling Device Manager");
                this->disable();
                return;
            }

            this->_mqtt = (MQTTModule *)this->_application->getModule("mqtt");
            this->_mqtt->registerCallback(this);

            if (config.containsKey("devices"))
            {
                JsonObject root = config["devices"].as<JsonObject>();
                Log.notice("(deviceManager.boot) Booting %d devices", config["devices"].size());
                for (JsonObject::iterator it = root.begin();it != root.end();++it)
                {
                    //config.prettyPrintTo(Serial);
                    this->makeDevice(it->value().as<JsonObject>());
                }
            }
        }

        virtual void setup(void)
        {
            //Log.notice("Setup devices");
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
            data.trim();

            Log.trace("(deviceManager.callback) Device callback on topic '%s' Data: %s", topic_string.c_str(), data.c_str());
            for (unsigned int i = 0; i < this->_devices.size(); i++)
            {
                this->_devices[i]->callback(topic_string, data);
            }
        }

        virtual void loop(unsigned long delta_time)
        {
            for (unsigned int i = 0; i < this->_devices.size(); i++)
            {
                Device *device = this->_devices[i];
                unsigned long current_time = millis();

                if (device->_updatePeriod == 0 || (current_time - device->_lastUpdate) > device->_updatePeriod)
                {
                    device->loop();
                    device->_lastUpdate = current_time;
                }
            }
        }
};
#endif
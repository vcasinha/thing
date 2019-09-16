#ifndef MQTT_MODULE_H
#define MQTT_MODULE_H

#include <Arduino.h>
#include <ArduinoLog.h>
#include <PubSubClient.h>
#include "DeviceModule.h"
#include "WiFiModule.h"
#include "Module.h"

class MQTTModule : public Module
{
    public:
        PubSubClient * _client;
        DeviceModule * _deviceModule;
        WiFiModule * _wifiModule;

        unsigned int _totalConnectionCount = 0;

        char _hostname[100];
        char _username[20];
        char _password[20];
        char _rootTopic[20];

        Vector<Module *> _callbacks;
        Vector<String *> _subscriptions;

        MQTTModule()
        {
            this->_name = "mqtt";
        }

        ~MQTTModule()
        {
            String topic = String("home/") + this->_application->_id + String("/state");
            this->publish(topic.c_str(), "OFF");
        }

        virtual void boot(JsonObject & config)
        {
            int port = 1883;
            this->_deviceModule = (DeviceModule *)this->_application->getModule("device");
            this->_wifiModule = (WiFiModule *)this->_application->getModule("wifi");

            if(config.size() == 0)
            {
                Log.warning("(MQTT) Empty configuration, disable MQTT");
                this->disable();
                return;
            }

            strcpy(this->_hostname, config["hostname"]);
            strcpy(this->_username, config["username"]);
            strcpy(this->_password, config["password"]);
            strcpy(this->_rootTopic, config["root_topic"] | "home");

            Log.notice("(MQTT.boot) ** MQTT Server address %s:%d", this->_hostname, port);
            this->_client = new PubSubClient(this->_hostname, port, this->_wifiModule->_wifiClient);
        }

        virtual void setup(void)
        {
            Log.trace("(MQTT.setup) ** Register base callback");
            this->_client->setCallback([this](char *topic, unsigned char *payload, unsigned int length) {
                //Serial.printf("(MQTT) ** Base callback\n");
                this->callback(topic, payload, length);
            });
            this->connect();
        }

        virtual void loop(unsigned long delta_time)
        {
            if(WiFi.getMode() == WIFI_AP)
            {
                return;
            }

            this->connect();
            if (WiFi.status() == WL_CONNECTED && this->_client->connected())
            {
                bool status = this->_client->loop();
                if (!status)
                {
                    Log.error("(MQTT.loop) ** Loop failed - Force disconnection");
                    this->_client->disconnect();
                }
            }
        }

        void connect(void)
        {
            if (WiFi.status() == WL_CONNECTED && this->_client->connected() == false && WiFi.getMode() == WIFI_STA)
            {
                this->_totalConnectionCount++;
                Log.notice("(MQTT.connect) Connect to %s as '%s':'%s' attempt %d", this->_hostname, this->_username, this->_password, this->_totalConnectionCount);
                this->_client->connect(this->_deviceModule->_hostname.c_str(), this->_username, this->_password);
                if (this->_client->connected() == false)
                {
                    Log.error("(MQTT.connect) Not able to connect to server, wait 1s");
                    delay(1000);
                }
                else
                {
                    Log.notice("(MQTT.connect) Connection successfull");
                    for (unsigned int i = 0; i < this->_subscriptions.size(); i++)
                    {
                        const char *topic = this->_subscriptions[i]->c_str();
                        Log.trace("(MQTT.connect) Refresh server on topic '%s'", topic);
                        this->_client->unsubscribe(topic);
                        this->_client->subscribe(topic);
                    }
                }
            }
        }

        void registerCallback(Module * module)
        {
            Log.trace("(MQTT.registerCallback) Module '%s' requested MQTT callback", module->_name);
            this->_callbacks.push(module);
        }

        void callback(char * topic, unsigned char * payload, unsigned int length)
        {
            Log.trace("(MQTT.callback) topic '%s'", topic);
            for(unsigned int c = 0;c < _callbacks.size();c++)
            {
                Log.verbose("(MQTT.callback) Module '%s' callbacks", this->_callbacks[c]->_name);
                this->_callbacks[c]->callback(topic, payload, length);
            }
        }

        void publish(const char * topic, const char * payload)
        {
            if(this->_enabled == false || !this->_client->connected())
            {
                return;
            }
            Log.trace("(MQTT.publish) Publish on %s payload: %s", topic, payload);
            this->_client->publish(topic, payload);
        }

        void publish(const char *topic, const char *payload, unsigned int length)
        {
            if (this->_enabled == false || !this->_client->connected())
            {
                return;
            }
            Log.trace("(MQTT.publish) Publish on '%s' payload %s", topic, payload);
            this->_client->publish(topic, payload, length);
        }

        void publishState(const char *device_type, const char * location, const char * device_class, const char *payload)
        {
            if(this->_enabled == false)
            {
                return;
            }
            char topic[200];
            this->makeTopic(topic, device_type, location, device_class, "state");
            this->publish(topic, payload);
        }

        void makeTopic(char * topic, const char * device_type, const char * location, const char * device_class, const char * subject)
        {
            sprintf(topic, "%s/%s/%s/%s/%s", this->_rootTopic, device_type, location, device_class, subject);
        }

        void subscribe(const char * topic)
        {
            if(this->_enabled == false)
            {
                return;
            }

            Log.trace("(MQTT.subscribe) Subscribe '%s' topic", topic);
            this->_subscriptions.push(new String(topic));
            if (this->_client->connected())
            {
                this->_client->subscribe(topic);
            }
        }
};

#endif
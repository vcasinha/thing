#ifndef MQTT_MODULE_H
    #define MQTT_MODULE_H

    #include <Arduino.h>
    #include <PubSubClient.h>
    #include "DeviceModule.h"
    #include "WiFiModule.h"
    #include "Module.h"

    class MQTTModule : public Module 
    {
        public:
            WiFiClient _wifiClient;
            PubSubClient * _client;
            WiFiModule * _wifiModule;
            DeviceModule *_deviceModule;

            unsigned int _connectionCount = 0;
            unsigned int _counter = 0;

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
                this->_client->publish(topic.c_str(), "OFF");
            }

            virtual void boot(JsonObject & config)
            {
                this->_wifiModule = (WiFiModule *) this->_application->getModule("wifi");
                this->_deviceModule = (DeviceModule * ) this->_application->getModule("device");

                strcpy(this->_hostname, config["hostname"] | "");
                strcpy(this->_username, config["username"] | "");
                strcpy(this->_password, config["password"] | "");
                strcpy(this->_rootTopic, config["root_topic"] | "home");

                this->_client = new PubSubClient(this->_hostname, 1883, this->_wifiClient);
            }

            virtual void setup(void)
            {
                this->_client->setCallback([this] (char * topic, unsigned char * payload, unsigned int length) { this->callback(topic, payload, length); });
                this->connect();
            }
            
            virtual void loop(void)
            {
                if(WiFi.getMode() == WIFI_AP)
                {
                    return;
                }

                this->connect();
                if (WiFi.status() == WL_CONNECTED)
                {
                    bool status = this->_client->loop();
                    if (!status)
                    {
                        Serial.printf("(MQTT) ** Loop failed - Force disconnection\n");
                        this->_client->disconnect();
                    }
                }
            }

            void connect(void)
            {
                if (WiFi.status() == WL_CONNECTED && this->_client->connected() == false)
                {
                    Serial.printf("Connect to MQTT %s as '%s' : '%s' attempt %d\n", this->_hostname, this->_username, this->_password, this->_connectionCount);
                    while (this->_client->connected() == false)
                    {
                        Serial.printf("** Attempt %d of 5\n", this->_counter++);
                        this->_client->connect(this->_deviceModule->_hostname.c_str(), this->_username, this->_password);
                        if (this->_client->connected() == false)
                        {
                            this->_connectionCount++;
                            this->_counter++;
                            Serial.printf("** Not able to connect to MQTT\n");
                            if (this->_counter >= 5)
                            {
                                this->_counter = 0;
                                Serial.printf("** Give up MQTT after %d attempts\n", this->_counter);
                                //this->disable();
                                return;
                            }
                            delay(500);
                        }
                        else
                        {
                            Serial.printf("** MQTT Refresh subscriptions\n");
                            for(unsigned int i = 0;i < this->_subscriptions.size();i++)
                            {
                                const char *topic = this->_subscriptions[i]->c_str();
                                this->_client->unsubscribe(topic);
                                this->_client->subscribe(topic);
                            }
                        }
                    }
                }
            }

            void registerCallback(Module * module)
            {
                Serial.printf("Module '%s' requested MQTT callback\n", module->_name);
                this->_callbacks.push(module);
            }

            void callback(char * topic, unsigned char * payload, unsigned int length)
            {
                Serial.printf("(MQTTModule) topic %s\n", topic);
                for(unsigned int c = 0;c < _callbacks.size();c++)
                {
                    this->_callbacks[c]->callback(topic, payload, length);
                }
            }

            void publish(const char * topic, const char * payload)
            {
                Serial.printf("Publish on %s payload: %s\n", topic, payload);
                this->_client->publish(topic, payload);
            }

            void publishState(const char *device_type, const char * location, const char * device_class, const char *payload)
            {
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
                Serial.printf("Subscribe %s\n", topic);
                this->_client->subscribe(topic);
                this->_subscriptions.push(new String(topic));
            }
    };

#endif
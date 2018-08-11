#ifndef MQTT_MODULE_H
    #define MQTT_MODULE_H

    #include <Arduino.h>
    #include <PubSubClient.h>
    #include "WiFiModule.h"
    #include "Module.h"

    class MQTTModule : public Module 
    {
        public:
            PubSubClient _client;
            WiFiModule * _wifiModule;
            unsigned int _counter = 0;
            const char * _location = "unknown";
            const char * _hostname;
            const char * _username;
            const char * _password;
            const char * _rootTopic;

            Vector<Module *> _callbacks;

            MQTTModule()
            {
                this->_name = "mqtt";
            }

            ~MQTTModule()
            {
                String topic = String("home/") + this->_application->_id + String("/state");
                this->_client.publish(topic.c_str(), "OFF");
                this->_loop_period_ms = 100;
            }

            virtual void boot(JsonObject & config)
            {
                this->_wifiModule = (WiFiModule *) this->_application->getModule("wifi");

                this->_wifiModule->addParameterHTML("<p>MQTT Server</p>");
                this->_wifiModule->addParameter("mqtt_hostname", "Hostname", config["hostname"] | "", 40);
                this->_wifiModule->addParameter("mqtt_username", "Username", config["username"] | "", 40);
                this->_wifiModule->addParameter("mqtt_password", "Password", config["password"] | "", 40);
                this->_wifiModule->addParameter("mqtt_root", "Root topic", config["root"] | "home", 40);

                this->_client.setClient(this->_wifiModule->_wirelessClient);
            }

            virtual void setup(void)
            {
                this->_hostname = this->_wifiModule->getParameter("mqtt_hostname")->getValue();
                this->_username = this->_wifiModule->getParameter("mqtt_username")->getValue();
                this->_password = this->_wifiModule->getParameter("mqtt_password")->getValue();
                this->_location = this->_wifiModule->getParameter("device_location")->getValue();
                this->_rootTopic = this->_wifiModule->getParameter("mqtt_root")->getValue();
                
                this->_client.setCallback([this] (char * topic, unsigned char * payload, unsigned int length) { this->callback(topic, payload, length); });
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
                    while (this->_client.connected() == false && this->_counter < 5) 
                    {
                        Serial.printf("Connect to MQTT %s as '%s' : '%s' attempt %d\n", this->_hostname, this->_username, this->_password, this->_counter);
                        this->_client.setServer(this->_hostname, 1883);
                        this->_client.connect(this->_hostname, this->_username, this->_password);

                        if(this->_client.connected() == false)
                        {
                            this->_counter++;
                            Serial.printf("** Could not connect to MQTT server\n");
                            if(this->_counter >= 5)
                            {
                                Serial.printf("    Ignoring MQTT after %d attempts\n", this->_counter);
                                this->disable();
                            }
                            delay(500);
                        }
                    }
                }
            }

            void registerCallback(Module * module)
            {
                Serial.printf("Module %s requested MQTT callback\n", module->_name);
                this->_callbacks.push(module);
            }

            void callback(char * topic, unsigned char * payload, unsigned int length)
            {
                Serial.printf("Seriously??? :) topic %s\n", topic);
                for(unsigned int c = 0;c < _callbacks.size();c++)
                {
                    this->_callbacks[c]->callback(topic, payload, length);
                }
            }

            void publish(const char * topic, const char * payload)
            {
                this->_client.publish(topic, payload);
            }

            void publishState(const char * device_type, const char * device_class, const char * payload)
            {
                char topic[200];
                sprintf(topic, "%s/%s/%s/%s/state", this->_rootTopic, device_type, this->_location, device_class);
                this->_client.publish(topic, payload);
            }

            void subscribe(const char * topic)
            {
                this->_client.subscribe(topic);
            }
    };

#endif
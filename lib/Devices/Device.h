#ifndef DEVICE_H
    #define DEVICE_H

    #include <Arduino.h>
    #include "MQTTModule.h"

    class Device
    {
        public:
            MQTTModule * _mqtt;
            String _id;
            String _location;
            String _type;

            unsigned int _update_period = 5;
            unsigned int _last_update = 0;

            char _commandTopic[100];
            char _availabilityTopic[100];
            char _stateTopic[100];
            bool _state = false;

            Device()
            {
                this->_type = "device";
            }

            void publish(const char *topic, const char *payload)
            {
                this->_mqtt->publish(topic, payload);
            }

            void publishState(const char *payload)
            {
                this->_mqtt->publish(this->_stateTopic, payload);
            }

            void publishAvailability(const char *payload)
            {
                this->_mqtt->publish(this->_availabilityTopic, payload);
            }
            void setMQTT(MQTTModule * mqtt)
            {
                //Serial.printf("(DEVICE.setMQTT) %s (%s) on %s\n", this->_id.c_str(), this->_type.c_str(), this->_location.c_str());
                this->_mqtt = mqtt;
                this->_mqtt->makeTopic(this->_commandTopic, this->_type.c_str(), this->_location.c_str(), this->_id.c_str(), "command");
                this->_mqtt->makeTopic(this->_stateTopic, this->_type.c_str(), this->_location.c_str(), this->_id.c_str(), "state");
                this->_mqtt->makeTopic(this->_availabilityTopic, this->_type.c_str(), this->_location.c_str(), this->_id.c_str(), "available");
                this->_mqtt->subscribe(this->_commandTopic);

                this->setup();
                this->publishAvailability("available");
            }

            void callback(String topic, String payload)
            {
                if(topic.equals(this->_commandTopic))
                {
                    this->onCommand(payload);
                }
            }

            void boot(JsonObject & config)
            {
                this->_id = config["id"].as<String>();
                this->_location = config["location"].as<String>();
                this->_state = config["state"].as<bool>() | false;
                this->_update_period = config["period"].as<int>() | this->_update_period;
                this->config(config);

                Serial.printf("(DEVICE) Booting as %s@%s (%s)\n", this->_id.c_str(), this->_location.c_str(), this->_type.c_str());
            }

            virtual void onCommand(String) {}
            virtual void config(JsonObject &config) {}
            virtual void setup() {}
            virtual void loop() {}
    };

#endif
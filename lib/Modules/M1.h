#include <Arduino.h>

#include "Module.h"
#include "MQTTModule.h"

class M1: public Module 
{
    public:
        MQTTModule * _mqtt;
        unsigned int _counter = 0;

        M1()
        {
            _name = "M1";
        }

        virtual void boot(void)
        {
            this->_mqtt = (MQTTModule *) this->_application->getModule("MQTT");
            this->_mqtt->registerCallback(this);
        }

        virtual void setup(void)
        {
            //
        }

        virtual void loop(void)
        {
            if(_counter % 1000 == 0)
            {
                Serial.printf("Publish\n");
                _mqtt->_client.publish("test", "something");
            }
            _counter += millis();
        }

        void callback(char * topic, unsigned char * payload, unsigned int length)
        {
            Serial.printf("Inside M1 topic %s", topic);
        }

};

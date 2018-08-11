#include <Arduino.h>

#include "Module.h"
#include "MQTTModule.h"

class LM298Module: public Module 
{
    public:
        MQTTModule * _mqtt;
        unsigned int _counter = 0;
        float _humidity;
        float _temperature;

        LM298Module()
        {
            _name = "actuator.lm298";
            _update_period = 50;
        }

        virtual void boot(void)
        {
            this->_mqtt = (MQTTModule *) this->_application->getInstance("mqtt");
            this->_mqtt->registerCallback(this);
        }

        virtual void setup(void)
        {
            pinMode(D1, OUTPUT);
            pinMode(D2, OUTPUT);
            this->_mqtt->subscribe("home/lm298/command");
        }

        virtual void loop(void)
        {
            if(_counter > 1023)
            {
                _counter = 0;
                analogWrite(D2, _counter);
            }

            _counter++;
        }

        void callback(char * topic, unsigned char * payload, unsigned int length)
        {
            if(strcmp(topic, "home/lm298/command") == 0)
            {
                Serial.printf("Inside LM298 topic %s -> %s\n", topic, payload);
            }
        }
};

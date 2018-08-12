#ifndef SWITCH_MODULE_H
#define SWITCH_MODULE_H

#include <Arduino.h>
#include <WiFiManager.h>
#include "Module.h"
#include "MQTTModule.h"
#include "Vector.h"

class Switch
{
  public:
    unsigned int _pin;
    char _class[20];
    char _topic[100];
    bool _state;
};

class SwitchModule : public Module
{
  public:
    MQTTModule *_mqtt;
    unsigned long _time = 0;
    bool _state = false;
    unsigned int _pin = D5;
    Vector<Switch *> _switches;

    SwitchModule()
    {
        this->_name = "switch";
        //this->_loop_period_ms = 1000;
    }

    virtual void boot(JsonObject &config)
    {
        this->_mqtt = (MQTTModule *)this->_application->getModule("mqtt");
        if (config["switches"])
        {
            for (unsigned int i = 0; i < config["switches"].size(); i++)
            {
                JsonObject &sw_config = config["switches"][i];
                Switch * s = new Switch();
                strcpy(s->_class, sw_config["class"]);
                this->_mqtt->makeTopic(s->_topic, "switch", s->_class, "command");
                s->_pin = sw_config["pin"];
                s->_state = sw_config["state"] | false;
                this->_switches.push(s);
            }
        }
    }

    virtual void setup(void)
    {
        this->_mqtt->registerCallback(this);
        pinMode(this->_pin, OUTPUT);
        for (unsigned int i = 0; i < this->_switches.size(); i++)
        {
            Switch *s = this->_switches[i];

            Serial.printf("Switch '%s' on pin %d topic %s\n", s->_class, s->_pin, s->_topic);
            
            this->_mqtt->_client.subscribe(s->_topic);
            pinMode(s->_pin, OUTPUT);
            digitalWrite(s->_pin, s->_state);
            
        }
    }

    virtual void loop(void)
    {
    }

    void callback(char *topic, unsigned char *payload, unsigned int length)
    {
        //topic[length] = '\0';
        String topic_string = String(topic);
        //Serial.printf("Callback %s\n", topic_string.c_str());
        for (unsigned int i = 0; i < this->_switches.size(); i++)
        {
            Switch *s = this->_switches[i];

            //Serial.printf("Checking against topic %s\n", s->_topic);
            if (topic_string.startsWith(s->_topic))
            {
                //Serial.printf("Matches %s\n", topic);
                s->_state = !s->_state;
                digitalWrite(s->_pin, s->_state);
                this->_mqtt->publishState("switch", s->_class, s->_state ? "ON" : "OFF");
            }
        }
    }
};
#endif
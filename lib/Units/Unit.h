#ifndef UNIT_H
#define UNIT_H
#include <ArduinoLog.h>
#include <Arduino.h>
#include "MQTTModule.h"
#include "Application.h"

class Unit
{
protected:
    unsigned int _updateFrequency = 1;

public:
    Application *_application;
    MQTTModule *_mqtt;
    String _id;
    String _location;
    String _type;

    unsigned long _lastUpdate = 0;
    unsigned long _updatePeriod = 60000;
    unsigned long _MQTTUpdatePeriod = 60;
    unsigned long _previousMQTTUpdate = 0;

    char _commandTopic[100];
    char _availabilityTopic[100];
    char _stateTopic[100];
    bool _state = false;
    bool _useFrequency = false;

    Unit()
    {
        this->_type = "unit";
    }

    void init(String type, float mqtt_period = 60, unsigned long loop_period_ms = 0)
    {
        this->_type = type;
        this->_updatePeriod = loop_period_ms;
        this->_MQTTUpdatePeriod = mqtt_period;
    }

    void setFrequency(unsigned int frequency)
    {
        if (this->_useFrequency)
        {
            Log.notice("(device.setFrequency) '%s' set frequency to %d", this->_id.c_str(), frequency);
            this->_updateFrequency = frequency;
            if (frequency == 0)
            {
                this->_updatePeriod = 0;
            }
            else
            {
                this->_updatePeriod = (1000 / frequency);
            }
        }
    }

    void setPeriod(unsigned int period)
    {
        if (!this->_useFrequency)
        {
            Log.notice("(device.setPeriod) '%s' set period to %d", this->_id.c_str(), period);
            this->_updatePeriod = period;
        }
    }

    void setMQTT(MQTTModule *mqtt)
    {
        //Serial.printf("(DEVICE.setMQTT) %s (%s) on %s", this->_id.c_str(), this->_type.c_str(), this->_location.c_str());
        this->_mqtt = mqtt;
        this->_mqtt->makeTopic(this->_commandTopic, this->_type.c_str(), this->_location.c_str(), this->_id.c_str(), "command");
        this->_mqtt->makeTopic(this->_stateTopic, this->_type.c_str(), this->_location.c_str(), this->_id.c_str(), "state");
        this->_mqtt->makeTopic(this->_availabilityTopic, this->_type.c_str(), this->_location.c_str(), this->_id.c_str(), "available");
        this->_mqtt->subscribe(this->_commandTopic);

        this->setup();
        this->publishAvailability("available");
    }

    virtual void unitConfig(JsonObject & config)
    {
        if (config.containsKey("use_frequency"))
        {
            this->_useFrequency = config["use_frequency"].as<bool>();
        }

        if (config.containsKey("frequency"))
        {
            this->setFrequency(config["frequency"].as<int>());
        }

        if (config.containsKey("period"))
        {
            this->setPeriod(config["period"].as<int>());
        }

        this->config(config);
    }

    void boot(String unitID, JsonObject &config, Application *app);
    void ready();
    Unit * getUnitByID(String unitID);
    void publish(const char *topic, const char *payload);
    void publishState(const char *payload);
    void publishState(DynamicJsonDocument json_document);
    void publishAvailability(const char *payload);
    void setState(bool state);
    void callback(String topic, String payload);

    virtual void trigger() {}
    virtual void onCommand(String) {}
    virtual void config(JsonObject &config) {}
    virtual void setup() {}
    virtual void loop() {}
    virtual void MQTTLoop(unsigned long current_time, unsigned long delta_time) {}
    virtual void getStatus(JsonObject &status) { }
};

#endif
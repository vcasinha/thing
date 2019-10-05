#ifndef UNIT_H
#define UNIT_H
#include <ArduinoLog.h>
#include <Arduino.h>
#include "MQTTModule.h"

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
    bool _useFrequency = false;

    unsigned long _lastUpdate = 0;
    unsigned long _updatePeriod = 60000;
    unsigned long _MQTTUpdatePeriod = 60;
    unsigned long _previousMQTTUpdate = 0;

    char _commandTopic[100];
    char _availabilityTopic[100];
    char _stateTopic[100];
    bool _state = false;

    Unit()
    {
        this->_type = "unit";
    }

    virtual ~Unit()
    {
    }

    void init(const char *type, float mqtt_period = 60, unsigned long loop_period_ms = 0)
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

    void publish(const char *topic, const char *payload)
    {
        this->_mqtt->publish(topic, payload);
    }

    void publishState(const char *payload)
    {
        this->_mqtt->publish(this->_stateTopic, payload);
    }

    void publishState(DynamicJsonDocument json_document)
    {
        String state;
        serializeJson(json_document, state);
        this->_mqtt->publish(this->_stateTopic, state.c_str());
    }

    void publishAvailability(const char *payload)
    {
        this->_mqtt->publish(this->_availabilityTopic, payload);
    }

    void setState(bool state)
    {
        this->_state = state;
        Log.notice("(device.setState) Update state on '%s' to '%s'", this->_id.c_str(), this->_state ? "ON" : "OFF");
        this->publishState(this->_state ? "ON" : "OFF");
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

    void callback(String topic, String payload)
    {
        if (topic.equals(this->_commandTopic))
        {
            this->onCommand(payload);
        }
    }

    void boot(JsonObject & config)
    {
        this->_id = config["id"].as<String>();
        this->_state = config["state"].as<bool>() | false;
        this->_location = config["location"] | "unknown";
        this->setPeriod(this->_updatePeriod);
        this->config(config);

        Log.notice("(device.boot) Booting as %s@%s (%s)", this->_id.c_str(), this->_location.c_str(), this->_type.c_str());
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
    virtual void MQTTLoop() {}
    virtual void getStatus(JsonObject &status) { }
};

#endif
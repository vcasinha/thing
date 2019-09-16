#ifndef DEVICE_H
#define DEVICE_H
#include <ArduinoLog.h>
#include <Arduino.h>
#include "MQTTModule.h"

class Device
{
protected:
    unsigned int _updateFrequency = 1;

public:
    MQTTModule *_mqtt;
    String _id;
    String _location;
    String _type;
    bool _useFrequency = false;

    unsigned int _lastUpdate = 0;
    unsigned int _updatePeriod = 60;

    char _commandTopic[100];
    char _availabilityTopic[100];
    char _stateTopic[100];
    bool _state = false;

    Device()
    {
        this->_type = "device";
    }

    virtual ~Device()
    {
    }

    void setFrenquency(unsigned int frequency)
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
            this->_updatePeriod = period * 1000;
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
        this->_location = config["location"].as<String>();
        this->setPeriod(this->_updatePeriod);
        this->config(config);

        Log.notice("(device.boot) Booting as %s@%s (%s)", this->_id.c_str(), this->_location.c_str(), this->_type.c_str());
    }

    virtual void config(JsonObject & config)
    {
        if (config.containsKey("use_frequency"))
        {
            this->_useFrequency = config["use_frequency"].as<bool>();
        }

        if (config.containsKey("frequency"))
        {
            this->setFrenquency(config["frequency"].as<int>());
        }

        if (config.containsKey("period"))
        {
            this->setPeriod(config["period"].as<int>());
        }

        this->deviceConfig(config);
    }

    virtual void onCommand(String) {}
    virtual void deviceConfig(JsonObject &config) {}
    virtual void setup() {}
    virtual void loop() {}
};

#endif
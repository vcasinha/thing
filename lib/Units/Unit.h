#ifndef DEVICE_H
#define DEVICE_H
#include <ArduinoLog.h>
#include <Arduino.h>
#include "MQTTModule.h"
#include <Ticker.h>

class Unit
{
protected:
    unsigned int _updateFrequency = 1;

public:
    MQTTModule *_mqtt;
    String _id;
    String _location;
    const char * _type;

    unsigned long _loop_period_ms = 1000;
    float _MQTTUpdatePeriod = 60;

    Ticker * _mqttTicker;
    Ticker * _loopTicker;

    char _commandTopic[100];
    char _availabilityTopic[100];
    char _stateTopic[100];
    bool _state = false;

    Unit()
    {
        this->_type = "unit";
        this->_mqttTicker = new Ticker();
        this->_loopTicker = new Ticker();
    }

    void init(const char * type, unsigned long loop_period_ms = 1000, float mqtt_period = 60)
    {
        this->_type = type;
        this->_loop_period_ms = loop_period_ms;
        this->_MQTTUpdatePeriod = mqtt_period;
    }

    void ready(MQTTModule *mqtt)
    {
        this->setMQTT(mqtt);
        this->_mqttTicker->attach_scheduled(this->_MQTTUpdatePeriod, [&](){
            this->MQTTLoop();
        });

        this->_loopTicker->attach_scheduled(this->_loop_period_ms, [&](){
            this->loop();
        });
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
        this->_mqtt->makeTopic(this->_commandTopic, this->_type, this->_location.c_str(), this->_id.c_str(), "command");
        this->_mqtt->makeTopic(this->_stateTopic, this->_type, this->_location.c_str(), this->_id.c_str(), "state");
        this->_mqtt->makeTopic(this->_availabilityTopic, this->_type, this->_location.c_str(), this->_id.c_str(), "available");
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
        this->_loop_period_ms = config["loop_period_ms"] | this->_loop_period_ms;
        this->_MQTTUpdatePeriod = config["mqtt_update_period"] | this->_MQTTUpdatePeriod;
        this->config(config);

        Log.notice("(device.boot) Booting as %s@%s (%s)", this->_id.c_str(), this->_location.c_str(), this->_type);
    }

    virtual void unitConfig(JsonObject & config)
    {
        this->config(config);
    }

    virtual void onCommand(String) {}
    virtual void config(JsonObject &config) {}
    virtual void setup() {}
    virtual void loop() {}
    virtual void MQTTLoop() {}
};

#endif
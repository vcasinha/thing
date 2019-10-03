#ifndef UNIT_H
#define UNIT_H
#include <ArduinoLog.h>
#include <Arduino.h>
#include <Ticker.h>
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

    void init(const char *type, float mqtt_period = 60, unsigned long loop_period_ms = 0)
    {
        this->_type = type;
        this->_loop_period_ms = loop_period_ms;
        this->_MQTTUpdatePeriod = mqtt_period;
    }

    void unitConfig(JsonObject & config)
    {
        this->config(config);
    }

    void boot(Application *app, JsonObject &config);
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
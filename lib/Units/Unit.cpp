#include "Unit.h"
#include "UnitManagerModule.h"

void Unit::boot(Application *app, JsonObject &config)
{
    this->_application = app;
    this->_mqtt = (MQTTModule *)this->_application->getModule("mqtt");

    this->_id = config["id"].as<String>();
    this->_state = config["state"].as<bool>() | false;
    this->_location = config["location"] | "unknown";
    this->_loop_period_ms = config["loop_period_ms"] | this->_loop_period_ms;
    this->_MQTTUpdatePeriod = config["mqtt_update_period"] | this->_MQTTUpdatePeriod;
    this->config(config);

    this->_mqtt->makeTopic(this->_commandTopic, this->_type, this->_location.c_str(), this->_id.c_str(), "command");
    this->_mqtt->makeTopic(this->_stateTopic, this->_type, this->_location.c_str(), this->_id.c_str(), "state");
    this->_mqtt->makeTopic(this->_availabilityTopic, this->_type, this->_location.c_str(), this->_id.c_str(), "available");
    this->_mqtt->subscribe(this->_commandTopic);

    Log.notice("(device.boot) Booting as %s@%s (%s)", this->_id.c_str(), this->_location.c_str(), this->_type);
}

void Unit::ready()
{
    if (this->_MQTTUpdatePeriod > 0)
    {
        this->_mqttTicker->attach_scheduled(this->_MQTTUpdatePeriod, [&]() {
            this->MQTTLoop();
        });
    }

    if (this->_loop_period_ms)
    {
        this->_loopTicker->attach_scheduled(this->_loop_period_ms, [&]() {
            this->loop();
        });
    }

    this->publishAvailability("available");
    this->setup();
}

void Unit::publish(const char *topic, const char *payload)
{
    this->_mqtt->publish(topic, payload);
}

void Unit::publishState(const char *payload)
{
    this->_mqtt->publish(this->_stateTopic, payload);
}

void Unit::publishState(DynamicJsonDocument json_document)
{
    String state;
    serializeJson(json_document, state);
    this->_mqtt->publish(this->_stateTopic, state.c_str());
}

void Unit::publishAvailability(const char *payload)
{
    this->_mqtt->publish(this->_availabilityTopic, payload);
}

void Unit::setState(bool state)
{
    Log.notice("(device.setState) Set state on '%s' to '%s'", this->_id.c_str(), this->_state ? "ON" : "OFF");
    this->_state = state;
    this->publishState(this->_state ? "ON" : "OFF");
}

void Unit::callback(String topic, String payload)
{
    if (topic.equals(this->_commandTopic))
    {
        this->onCommand(payload);
    }
}

Unit * Unit::getUnitByID(String unitID)
{
    return ((UnitManagerModule *) this->_application->getModule("unit_manager"))->getUnitByID(unitID);
}
#include "Unit.h"
#include "UnitManagerModule.h"

void Unit::boot(String unitID, JsonObject & config, Application *app)
{
    this->_application = app;
    this->_mqtt = (MQTTModule *)this->_application->getModule("mqtt");

    this->_id = unitID;
    this->_state = config["state"].as<bool>() | false;
    this->_location = config["location"] | "unknown";
    this->_updatePeriod = config["loop_period_ms"] | this->_updatePeriod;
    this->_MQTTUpdatePeriod = config["mqtt_update_period"] | this->_MQTTUpdatePeriod;

    this->setPeriod(this->_updatePeriod);

    this->_mqtt->makeTopic(this->_commandTopic, this->_type.c_str(), this->_location.c_str(), this->_id.c_str(), "command");
    this->_mqtt->makeTopic(this->_stateTopic, this->_type.c_str(), this->_location.c_str(), this->_id.c_str(), "state");
    this->_mqtt->makeTopic(this->_availabilityTopic, this->_type.c_str(), this->_location.c_str(), this->_id.c_str(), "available");
    this->_mqtt->subscribe(this->_commandTopic);

    this->config(config);

    Log.notice("(device.boot) Booting as %s@%s (%s)", this->_id.c_str(), this->_location.c_str(), this->_type.c_str());
}

void Unit::ready()
{
    this->setup();
    this->publishAvailability("available");
    this->publishState(this->_state ? "ON" : "OFF");
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
    Log.notice("(unit.setState) Set state on '%s' to '%s'", this->_id.c_str(), this->_state ? "ON" : "OFF");
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
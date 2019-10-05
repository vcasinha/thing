#ifndef UNIT_MANAGER_MODULE_H
#define UNIT_MANAGER_MODULE_H

#include <Arduino.h>
#include <ArduinoLog.h>
#include "Module.h"
#include "MQTTModule.h"
#include "TimeModule.h"
#include "ServerModule.h"
#include "Vector.h"
#include "UnitFactory.h"
#include "Unit.h"

class UnitManagerModule : public Module
{
    public:
        MQTTModule *_mqtt;
        TimeModule * _time;
        ServerModule * _server;
        bool _state = false;
        unsigned int _pin = 0;
        Vector<Unit *> _units;
        Vector<UnitFactory *> _factories = Vector<UnitFactory *>();;

        UnitManagerModule()
        {
            this->_name = "unit_manager";
            this->_loop_period_ms = 20;
            //this->addFactory(new BlinkFactory());
        }

        Unit * getUnitByID(string unit_id)
        {
            for (unsigned int i = 0; i < this->_units.size(); i++)
            {
                if (this->_units[i]->id.equals(unit_id))
                {
                    return this->_units[i];
                }
            }
        }

        void addFactory(UnitFactory *factory)
        {
            Log.notice("(unitManager.addFactory) Load factory of %s devices.", factory->_unitType.c_str());
            this->_factories.push(factory);
        }

        UnitFactory * getFactory(String unit_type)
        {
            for (unsigned int i = 0; i < this->_factories.size(); i++)
            {
                if (this->_factories[i]->_unitType.equals(unit_type))
                {
                    return this->_factories[i];
                }
            }
        }

        void makeUnit(JsonObject config)
        {
            bool made = false;
            String unit_type = config["type"].as<String>();
            String json_config = "";
            serializeJson(config, json_config);
            Log.notice("(unitManager.makeDevice) Init '%s' device (%s)", unit_type.c_str(), json_config.c_str());

            for (unsigned int i = 0; i < this->_factories.size(); i++)
            {
                if (unit_type.equals(""))
                {
                    Log.error("(unitManager.makeDevice) ERROR undefined device type");
                    continue;
                }

                if (this->_factories[i]->_unitType.equals(unit_type))
                {
                    Unit *unit = this->_factories[i]->make();
                    unit->boot(config);
                    this->_units.push(unit);

                    made = true;
                }
            }

            if (made == false)
            {
                Log.error("(unitManager.makeDevice) ERROR Unknown device type '%s'.", unit_type.c_str());
            }
        }

        void config(JsonObject & config)
        {
            Log.notice("(UnitManager.config) Set units configuration");
            serializeJson(config, Serial);
            if (config.containsKey("units"))
            {
                for (unsigned int i = 0; i < this->_units.size(); i++)
                {
                    Unit * unit = this->_units[i];
                    if (config["units"][unit->_id])
                    {
                        JsonObject unit_config = config["units"][unit->_id];
                        String json_config = "";
                        serializeJson(unit_config, json_config);
                        Log.notice("(unitsManager.config) Update device configuration '%s' ()", unit->_id.c_str(), json_config.c_str());
                        this->_units[i]->unitConfig(unit_config);
                    }
                }
            }
        }

        virtual void boot(JsonObject & config)
        {
            if (config.size() == 0)
            {
                Log.warning("(MQTT) Empty configuration, disabling Device Manager");
                this->disable();
                return;
            }

            this->_mqtt = (MQTTModule *)this->_application->getModule("mqtt");
            this->_mqtt->registerCallback(this);

            this->_time = (TimeModule * )this->_application->getModule("time");

            if (config.containsKey("units"))
            {
                JsonObject root = config["units"].as<JsonObject>();
                Log.notice("(unitsManager.boot) Booting %d units", config["units"].size());
                for (JsonObject::iterator it = root.begin();it != root.end();++it)
                {
                    //config.prettyPrintTo(Serial);
                    this->makeUnit(it->value().as<JsonObject>());
                }
            }
        }

        void makeUnit(String unitID, JsonObject config);
        void config(JsonObject & config);
        void boot(JsonObject & config);
        void setup(void);
        void callback(char *topic, unsigned char *payload, unsigned int length);
};

#endif

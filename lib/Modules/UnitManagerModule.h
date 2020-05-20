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
        Vector<UnitFactory *> _factories = Vector<UnitFactory *>();

        UnitManagerModule()
        {
            this->init("unit_manager", 100);
        }

        Unit * getUnitByID(String unit_id)
        {
            for (unsigned int i = 0; i < this->_units.size(); i++)
            {
                if (this->_units[i]->_id.equals(unit_id))
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

        virtual void loop(unsigned long delta_time)
        {
            unsigned long current_time = this->_time->getTimestamp();
            for (unsigned int i = 0; i < this->_units.size(); i++)
            {
                Unit *unit = this->_units[i];
                unsigned long current_millis = millis();
                unsigned long delta_millis = current_millis - unit->_lastUpdate;
                if (unit->_updatePeriod == 0 || delta_millis > unit->_updatePeriod)
                {
                    //unit->loop(current_time, current_millis, delta_millis);
                    unit->loop();
                    unit->_lastUpdate = current_millis;
                }

                unsigned long delta_time = current_time - unit->_previousMQTTUpdate;
                if(delta_time > unit->_MQTTUpdatePeriod)
                {
                    //unit->MQTTLoop(current_time, delta_time);
                    unit->MQTTLoop(current_time, delta_time);
                    unit->_previousMQTTUpdate = current_time;
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

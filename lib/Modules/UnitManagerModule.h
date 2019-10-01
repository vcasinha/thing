#ifndef UNIT_MANAGER_MODULE_H
#define UNIT_MANAGER_MODULE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoLog.h>
#include "Module.h"
#include "MQTTModule.h"
#include "TimeModule.h"
#include "Vector.h"
#include "UnitFactory.h"
#include "Unit.h"

class UnitManagerModule : public Module
{
    public:
        MQTTModule *_mqtt;
        TimeModule * _time;
        bool _state = false;
        unsigned int _pin = 0;
        Vector<Unit *> _units;
        static Vector<UnitFactory *> _factories;

        static void addFactory(UnitFactory *factory)
        {
            Log.notice("(unitManagerModule.addFactory) Load '%s' factory", factory->_unitType.c_str());
            _factories.push(factory);
        }

        Unit * getUnitByID(String id)
        {
            for (unsigned int i = 0; i < this->_units.size(); i++)
            {
                if(this->_units[i]->_id.equals(id))
                {
                    return this->_units[i];
                }
            }

            return NULL;
        }

        static UnitFactory *getFactory(String unit_type)
        {
            for (unsigned int i = 0; i < UnitManagerModule::_factories.size(); i++)
            {
                if (unit_type.equals(""))
                {
                    Log.error("(unitManager.makeDevice) ERROR undefined device type");
                    continue;
                }

                if (UnitManagerModule::_factories[i]->_unitType.equals(unit_type))
                {
                    return _factories[i];
                }
            }

            return NULL;
        }

        UnitManagerModule();

        void makeUnit(JsonObject config);
        void config(JsonObject & config);
        void boot(JsonObject & config);
        void setup(void);
        void callback(char *topic, unsigned char *payload, unsigned int length);
};

#endif
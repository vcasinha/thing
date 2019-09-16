#ifndef UNIT_FACTORY
#define UNIT_FACTORY

#include <Arduino.h>
#include <ArduinoJson.h>
#include "Unit.h"

class UnitFactory
{
    public:
        String _unitType;

        virtual Unit * make()
        {
            return new Unit();
        }
};

#endif
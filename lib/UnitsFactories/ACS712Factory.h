#ifndef ACS712_FACTORY
#define ACS712_FACTORY

#include <Arduino.h>
#include "UnitFactory.h"
#include "ACS712Unit.h"

class ACS712Factory : public UnitFactory
{
    public:
        ACS712Factory()
        {
            this->_unitType = "power_meter";
        }

        virtual Unit * make()
        {
            return new ACS712Unit();
        }
};
#endif
#ifndef SWITCH_FACTORY
#define SWITCH_FACTORY

#include <Arduino.h>
#include "UnitFactory.h"
#include "RelayUnit.h"

class RelayFactory : public UnitFactory
{
    public:
        RelayFactory()
        {
            this->_unitType = "relay";
        }

        virtual Unit * make()
        {
            return new RelayUnit();
        }
};
#endif
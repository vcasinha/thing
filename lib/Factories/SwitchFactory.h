#ifndef SWITCH_FACTORY
#define SWITCH_FACTORY

#include <Arduino.h>
#include "UnitFactory.h"
#include "Switch.h"

class SwitchFactory : public UnitFactory
{
    public:
        SwitchFactory()
        {
            this->_unitType = "switch";
        }

        virtual Unit * make()
        {
            return new Switch();
        }
};
#endif
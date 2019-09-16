#ifndef DHTFACTORY_H
#define DHTFACTORY_H

#include <Arduino.h>
#include "UnitFactory.h"
#include "DHTUnit.h"

class DHTFactory: public UnitFactory
{
    public:
        DHTFactory()
        {
            this->_unitType = "dht";
        }

        virtual Unit * make()
        {
            return new DHTUnit();
        }
};

#endif
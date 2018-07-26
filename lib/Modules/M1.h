#include <Arduino.h>

#include "Module.h"
class M1: public Module 
{
    public:
        M1()
        {
            _name = "M1";
        }
        virtual void setup(void)
        {
        }
};

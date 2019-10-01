#ifndef RELAYUNIT_H
#define RELAYUNIT_H

#include <Arduino.h>
#include <ArduinoLog.h>
#include "Unit.h"
#include "UnitManagerModule.h"

class RelayUnit final : public Unit
{
    public:
        unsigned int _pin;
        unsigned int _buttonPin;
        bool _useButton;
        bool _buttonState;
        bool _triggerButton;
        bool _invertPin;
        bool _invertState;
        unsigned int _previousPinState;

        RelayUnit();
        virtual void config(JsonObject & config);
        virtual void setup();
        void updateState(bool state);
        virtual void onCommand(String data);
        virtual void loop();
        virtual void MQTTLoop();
};

#endif
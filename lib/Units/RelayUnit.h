#ifndef SWITCH_H
#define SWITCH_H

#include <Arduino.h>
#include <ArduinoLog.h>
#include "Unit.h"

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

        RelayUnit()
        {
            this->init("relay", 100, 60);
        }

        virtual void config(JsonObject & config)
        {
            this->_pin = config["pin"];
            Log.notice("(relay.construct) Switch on pin %u", this->_pin);
            pinMode(this->_pin, OUTPUT);

            this->_useButton = config["useButton"];
            if(this->_useButton)
            {
                this->_buttonPin = config["buttonPin"];
                this->_triggerButton = config["triggerButton"];
                this->_invertPin = config["invertPin"];
                this->_invertState = config["invertState"];

                pinMode(this->_buttonPin, INPUT);

                Log.notice("(relay.construct) Linked button on pin %u", this->_pin);
            }
        }

        void updateState(bool state)
        {
            if (state != this->_state)
            {
                this->_state = state;
                digitalWrite(this->_pin, this->_state);
            }

            this->publishState(this->_state ? "ON" : "OFF");
        }

        virtual void onCommand(String data)
        {
            this->updateState(data.equals("ON"));
            Serial.printf("Change switch %s state to %s\n", this->_id.c_str(), this->_state ? "ON" : "OFF");
        }

        virtual void loop()
        {
            unsigned int currentPinState = digitalRead(this->_buttonPin);
            if(currentPinState != this->_previousPinState)
            {
                if(this->_triggerButton)
                {
                    this->_buttonState = (currentPinState == HIGH);
                    Log.notice("(relay.loop) Button changed to %s", this->_buttonState ? "ON" : "OFF");
                    this->updateState(this->_buttonState);
                }
                else if ((currentPinState == HIGH) != this->_invertPin)
                {
                    this->_buttonState = !this->_buttonState;
                    Log.notice("(relay.loop) Button changed to %s", this->_buttonState ? "ON" : "OFF");
                    this->updateState(this->_buttonState);
                }
            }

            this->_previousPinState = currentPinState;
        }

        virtual void MQTTLoop()
        {
            this->publishState(this->_state ? "ON" : "OFF");
        }
};

#endif
#ifndef SWITCH_H
#define SWITCH_H

#include <Arduino.h>
#include <ArduinoLog.h>
#include "Unit.h"

class Switch final : public Unit
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

        Switch()
        {
            this->_type = "switch";
            this->_updatePeriod = 100;
            this->_MQTTUpdatePeriod = 30;
            Log.notice("(switch.construct) Update period %u", this->_updatePeriod);
        }

        ~Switch()
        {
        }

        virtual void deviceConfig(JsonObject &config)
        {
            this->_pin = config["pin"];
            Log.notice("(switch.construct) Switch on pin %u", this->_pin);
            pinMode(this->_pin, OUTPUT);

            this->_useButton = config["useButton"];
            if(this->_useButton)
            {
                this->_buttonPin = config["buttonPin"];
                this->_triggerButton = config["triggerButton"];
                this->_invertPin = config["invertPin"];
                this->_invertState = config["invertState"];

                pinMode(this->_buttonPin, INPUT);

                Log.notice("(switch.construct) Linked button on pin %u", this->_pin);
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

        virtual void loop(unsigned long time, unsigned long delta_millis)
        {
            unsigned int currentPinState = digitalRead(this->_buttonPin);
            if(currentPinState != this->_previousPinState)
            {
                if(this->_triggerButton)
                {
                    this->_buttonState = (currentPinState == HIGH);
                    Log.notice("(switch.loop) Button changed to %s", this->_buttonState ? "ON" : "OFF");
                    this->updateState(this->_buttonState);
                }
                else if ((currentPinState == HIGH) != this->_invertPin)
                {
                    this->_buttonState = !this->_buttonState;
                    Log.notice("(switch.loop) Button changed to %s", this->_buttonState ? "ON" : "OFF");
                    this->updateState(this->_buttonState);
                }
            }

            this->_previousPinState = currentPinState;
        }

        virtual void MQTTLoop(unsigned long timestamp, unsigned long delta_time)
        {
            this->publishState(this->_state ? "ON" : "OFF");
        }
};

#endif
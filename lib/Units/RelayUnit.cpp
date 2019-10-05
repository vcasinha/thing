#include <ArduinoJson.h>
#include "RelayUnit.h"

RelayUnit::RelayUnit()
{
    this->init("relay", 60, 100);
}

void RelayUnit::config(JsonObject &config)
{
    this->_pin = config["pin"];
    this->_invertState = config["invertState"];
    pinMode(this->_pin, OUTPUT);
    this->updateState((digitalRead(this->_pin) == HIGH));
    Log.notice("(relay.construct) Switch on pin %u", this->_pin);

    this->_useButton = config["useButton"];
    if (this->_useButton)
    {
        this->_buttonPin = config["buttonPin"];
        this->_buttonTrigger = config["buttonTrigger"];
        this->_buttonInvert = config["buttonInvert"];

        if(1 == 0)
        {
            pinMode(this->_buttonPin, INPUT_PULLUP);
        }
        else
        {
            pinMode(this->_buttonPin, INPUT);
        }

        Log.notice("(relay.construct) Linked button on pin %u", this->_pin);
    }
}

void RelayUnit::getStatus(JsonObject & status)
{
    status["pin"] = this->_pin;
    status["state"] = this->_state;
    status["useButton"] = this->_useButton;
    status["buttonPin"] = this->_buttonPin;
    status["triggerButton"] = this->_buttonTrigger;
    status["buttonInvert"] = this->_buttonInvert;
    status["invertState"] = this->_invertState;
    status["pinState"] = digitalRead(this->_pin) != this->_invertState;
}

void RelayUnit::updateState(bool state)
{
    this->_state = state;
    bool pinState = this->_state != this->_invertState;
    digitalWrite(this->_pin, pinState);
    this->publishState(pinState ? "ON" : "OFF");
    Log.notice("(relay.updateState) Change relay %s state to %s\n", this->_id.c_str(), pinState ? "ON" : "OFF");
}

void RelayUnit::setup()
{

}

void RelayUnit::onCommand(String data)
{
    this->updateState(data.equals("ON"));
}

void RelayUnit::loop()
{
    unsigned int currentPinState = digitalRead(this->_buttonPin);
    if (currentPinState != this->_previousPinState)
    {
        if (this->_buttonTrigger)
        {
            this->_buttonState = (currentPinState == HIGH);
            Log.notice("(relay.loop) Button changed to %s", this->_buttonState ? "ON" : "OFF");
            this->updateState(this->_buttonState);
        }
        else if ((currentPinState == HIGH) != this->_buttonInvert)
        {
            this->_buttonState = !this->_buttonState;
            Log.notice("(relay.loop) Button changed to %s", this->_buttonState ? "ON" : "OFF");
            this->updateState(this->_buttonState);
        }
    }

    this->_previousPinState = currentPinState;
}

void RelayUnit::MQTTLoop()
{
    this->publishState(this->_state ? "ON" : "OFF");
}
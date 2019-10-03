#include <ArduinoJson.h>
#include "RelayUnit.h"

RelayUnit::RelayUnit()
{
    this->init("relay", 60, 100);
}

void RelayUnit::config(JsonObject &config)
{
    this->_pin = config["pin"];
    pinMode(this->_pin, OUTPUT);
    this->_state = digitalRead(this->_pin) == HIGH;
    Log.notice("(relay.construct) Switch on pin %u", this->_pin);

    this->_useButton = config["useButton"];
    if (this->_useButton)
    {
        this->_buttonPin = config["buttonPin"];
        this->_triggerButton = config["triggerButton"];
        this->_invertPin = config["invertPin"];
        this->_invertState = config["invertState"];

        //pinMode(this->_buttonPin, INPUT_PULLUP);

        Log.notice("(relay.construct) Linked button on pin %u", this->_pin);
    }
}

void RelayUnit::getStatus(JsonObject & status)
{
    status["pin"] = this->_pin;
    status["state"] = this->_state;
    status["useButton"] = this->_useButton;
    status["buttonPin"] = this->_buttonPin;
    status["triggerButton"] = this->_triggerButton;
    status["buttonInvertPin"] = this->_invertPin;
    status["butonInvertState"] = this->_invertState;
}

void RelayUnit::updateState(bool state)
{
    if (state != this->_state)
    {
        this->_state = state;
        digitalWrite(this->_pin, this->_state);
    }

    this->publishState(this->_state ? "ON" : "OFF");
}

void RelayUnit::setup()
{

}

void RelayUnit::onCommand(String data)
{
    this->updateState(data.equals("ON"));
    Log.notice("(relay.onCommand) Change switch %s state to %s\n", this->_id.c_str(), this->_state ? "ON" : "OFF");
}

void RelayUnit::loop()
{
    unsigned int currentPinState = digitalRead(this->_buttonPin);
    if (currentPinState != this->_previousPinState)
    {
        if (this->_triggerButton)
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

void RelayUnit::MQTTLoop()
{
    this->publishState(this->_state ? "ON" : "OFF");
}
#ifndef ACS712_UNIT
#define ACS712_UNIT

#include <Arduino.h>
#include <ArduinoLog.h>
#include "TimeModule.h"
#include "Unit.h"

class ACS712Unit final : public Unit
{
public:
    unsigned int _pin;
    unsigned int _mVPerAmp = 185; // 185mV/A for 5A, 100 mV/A for 20A and 66mV/A for 30A Module
    unsigned long _previousTime;
    TimeModule * _time;
    float _calibration = 0; // V2 slider calibrates this
    float _referenceVCC = 3.3;
    float _pricePerKWH = 0.9;
    float _supplyVoltage = 233.0;   // reading from DMM
    float _wattHour;
    float _totalCost;

    ACS712Unit()
    {
        this->init("sensor", 60, 5000);
    }

    ~ACS712Unit()
    {
    }

    virtual void config(JsonObject &config)
    {
        this->_time = (TimeModule *)this->_application->getModule("time");
        this->_previousTime = this->_time->getTimestamp();
        this->_pin = config["pin"] | A0;
        this->_mVPerAmp = config["_mVPerAmp"] | 185;
        this->_referenceVCC = config["referenceVCC"] | 5;
        this->_pricePerKWH = config["pricePerKWH"] | 0.9;
        Log.notice("(acs712.construct) Sensor on pin %u", this->_pin);
        pinMode(this->_pin, INPUT);
        this->calibrate();
    }

    void calibrate()
    {
        this->_calibration = this->getVPP();
    }

    float getVPP()
    {
        float result;
        int readValue;
        int maxValue = 0;
        int minValue = 1024;
        uint32_t start_time = millis();
        while ((millis() - start_time) < 1000)
        {
            readValue = analogRead(this->_pin);
            if (readValue > maxValue)
            {
                maxValue = readValue;
            }
            if (readValue < minValue)
            {
                minValue = readValue;
            }
        }
        result = ((maxValue - minValue) * this->_referenceVCC) / 1024.0;

        Log.notice("Min: %s Max: %s Result: %s", String(minValue, 3).c_str(), String(maxValue, 3).c_str(), String(result, 3).c_str());
        return result;
    }

    virtual void loop()
    {
        unsigned long current_time = this->_time->getTimestamp();
        float Vpp = getVPP() - this->_calibration;
        float vRMS = (Vpp / 2.0) * 0.707; //root 2 is 0.707
        float Arms = (vRMS * 1000) / this->_mVPerAmp;
        float power = Arms * this->_supplyVoltage;
        if(power < 0)
        {
            power = 0;
        }
        this->_wattHour += power * (current_time - this->_previousTime) / 3600.0;
        this->_totalCost = this->_wattHour * (this->_pricePerKWH / 1000);

        this->_previousTime = current_time;

        Log.notice("I(RMS): %sA Power: %sW Consumption: %sKWh Cost: %s€", String(Arms, 3).c_str(),
            String(power, 3).c_str(), String(this->_wattHour/1000, 3).c_str(), String(this->_totalCost, 2).c_str());
    }

    virtual void MQTTLoop(unsigned long timestamp, unsigned long delta_time)
    {
        DynamicJsonDocument json_document(512);
        JsonObject json = json_document.to<JsonObject>();

        json["wattHour"] = this->_wattHour;
        json["totalCost"] = this->_totalCost;

        this->publishState(json_document);
    }
};

#endif
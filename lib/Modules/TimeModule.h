#ifndef TIME_MODULE
#define TIME_MODULE
#include "Module.h"
#include <ArduinoLog.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

class TimeModule : public Module
{
    public:
        WiFiUDP _ntpUDP;
        NTPClient * _ntpClient;
        String _ntpServer;
        unsigned int _timeOffset;
        unsigned int _updateInterval;
        unsigned long _currentTime;


        TimeModule()
        {
            this->init("time", 1000);
        }

        virtual void boot(JsonObject &config)
        {
            this->_ntpServer = config["NTPServerAddress"] | "europe.pool.ntp.org";
            _timeOffset = config["offset"] | 3600;
            _updateInterval = config["updateIntervalMS"] | 60000;
        }

        virtual void setup()
        {
            Log.notice("(time.boot) Set server to %s offset %lu Update Interval %lu", _ntpServer.c_str(), _timeOffset, _updateInterval);
            this->_ntpClient = new NTPClient(this->_ntpUDP, this->_ntpServer.c_str(), this->_timeOffset, this->_updateInterval);
            _ntpClient->update();
            Log.notice("(time.boot) The time is: %s", _ntpClient->getFormattedTime().c_str());
        }

        virtual void loop(unsigned int delta_time)
        {
            _ntpClient->update();
        }

        unsigned long getTimestamp()
        {
            return _ntpClient->getEpochTime();
        }
};
#endif
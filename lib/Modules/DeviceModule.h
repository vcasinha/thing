#ifndef DEVICE_MODULE_H
#define DEVICE_MODULE_H

#define FW_VERSION "0.0"

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ArduinoLog.h>
//#include <ESP8266httpUpdate.h>
#include "Module.h"

class DeviceModule : public Module
{
    public:
        String _boardID;
        String _hostname;
        String _password;
        String _location;
        bool _secure;
        unsigned long _previousTime;
        unsigned long _startTime;

        DeviceModule()
        {
            this->_name = "device";
            char tmp[10];
            sprintf(tmp, "ESP%6X", ESP.getFlashChipId());
            this->_boardID = tmp;
        }

        ~DeviceModule()
        {
        }

        virtual void boot(JsonObject &config)
        {
            this->_hostname = config["name"] | this->_boardID.c_str();
            this->_location = config["location"] | "unknown";
            this->_password = config["password"] | "password";
            this->_secure = config["secure"] | false;

            Log.notice("(device.boot) Booting %s @ %s", this->_hostname.c_str(), this->_location.c_str(), this->_password.c_str());
            if(this->_secure)
            {
                Log.notice("(device.boot) Device is in secure mode");
            }

            ArduinoOTA.onStart([this]() {
                Log.notice("(device.OTA) Begin update");
                String type;
                if (ArduinoOTA.getCommand() == U_FLASH)
                {
                    type = "sketch";
                }
                else
                {
                    type = "filesystem";
                }
                this->_startTime = millis();
                // NOTE: if updating FS this would be the place to unmount FS using FS.end()
                Log.notice("(device.OTA) Updating %s", type.c_str());
            });

            ArduinoOTA.onEnd([this]() {
                String t = String((float)(millis() - this->_startTime) / 1000, 2);
                Log.notice("(device.OTA) Update completed in %s seconds", t.c_str());
            });

            ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total) {
                unsigned long currentTime = millis();
                if(currentTime - this->_previousTime  > 2000)
                {
                    Log.notice("(device.OTA) Progress: %i%%", (progress / (total / 100)));
                    this->_previousTime = currentTime;
                }
            });

            ArduinoOTA.onError([this](ota_error_t error) {
                Log.notice("Error [%u]: ", error);
                if (error == OTA_AUTH_ERROR)
                    Log.error("(device.OTA) OTA Authentication failed");
                else if (error == OTA_BEGIN_ERROR)
                    Log.error("(device.OTA) OTA Failed to start");
                else if (error == OTA_CONNECT_ERROR)
                    Log.error("(device.OTA) OTA Connection error");
                else if (error == OTA_RECEIVE_ERROR)
                    Log.error("(device.OTA) OTA Receive error");
                else if (error == OTA_END_ERROR)
                    Log.error("(device.OTA) OTA Failed to finish");
            });
            //ArduinoOTA.setPort(8266);
            ArduinoOTA.setHostname(this->_hostname.c_str());
            if(this->_secure)
            {
                ArduinoOTA.setPassword(this->_password.c_str());
            }

            Log.notice("(device.boot) OTA Initialize");
            ArduinoOTA.begin();
        }

        virtual void loop(unsigned long delta_time)
        {
            ArduinoOTA.handle();
        }

        bool update(String filename)
        {
            bool status = false;
            // String uri = "/local/";

            // WiFiClient client;

            // uri.concat(filename);
            // String hostname = "petitmaison.duckdns.org";
            // unsigned int port = 8123;
            // Log.notice("(Device) Update from https://%s:%d%s", hostname.c_str(), port, uri.c_str());

            // ESPhttpUpdate.rebootOnUpdate(false);
            // delay(500);

            // t_httpUpdate_return ret = ESPhttpUpdate.update(client, uri, "");
            // switch (ret)
            // {
            // case HTTP_UPDATE_FAILED:
            //     Log.notice("(Device) Update Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
            //     break;

            // case HTTP_UPDATE_NO_UPDATES:
            //     Log.notice("(Device) Update ignored");
            //     break;

            // case HTTP_UPDATE_OK:
            //     status = true;
            //     Log.notice("(Device) HTTP update done");
            //     break;
            // }

            return status;
        }

        void eraseWiFiSettings()
        {
            Log.notice("(Device) Erasing configuration.");
            ESP.eraseConfig();
            this->restart();
        }

        void restart()
        {
            Log.notice("(Device) Restarting.");
            ESP.restart();
            delay(2000);
        }

        void checkForUpdates()
        {
            // //String mac = WiFi.macAddress();
            // String fwUrlBase = "https://petitmaison.duckdns.org:8123";
            // String fwVersionURL = "/local/firmware_version";
            // String fwFilename = "firmware.wemos.bin";

            // Log.notice("(Device) Checking for firmware updates");
            // //Log.notice("(Device) MAC address: %s", mac.c_str());
            // Log.notice("(Device) Firmware version URL: %s", fwVersionURL.c_str());

            // WiFiClient client;
            // HTTPClient httpClient;
            // httpClient.begin(client, fwVersionURL.c_str());
            // int httpCode = httpClient.GET();
            // if (httpCode == 200)
            // {
            //     String newFWVersion = httpClient.getString();

            //     Log.notice("(Device) Current firmware version: %s", FW_VERSION);
            //     Log.notice("(Device) Available firmware version: %s", newFWVersion.c_str());

            //     if (newFWVersion.toInt() > String(FW_VERSION).toInt())
            //     {
            //         Log.notice("(Device) Preparing to update");
            //         this->update(fwFilename);
            //     }
            //     else
            //     {
            //         Log.notice("(Device) Already on latest version");
            //     }
            // }
            // else
            // {
            //     Log.error("(Device) Firmware version check failed, (%i) %s", httpCode, httpClient.getString().c_str());
            // }
            // httpClient.end();
        }
};

#endif
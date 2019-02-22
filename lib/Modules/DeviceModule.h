#ifndef DEVICE_MODULE_H
#define DEVICE_MODULE_H

#define FW_VERSION "0.0"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
#include <ArduinoOTA.h>
#include "Module.h"

class DeviceModule : public Module
{
  public:
    String _boardID;
    String _hostname;
    String _location;

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
        this->_location = config["location"] | "somewhere";
        ArduinoOTA.onStart([this]() {
            Serial.println("Inicio...");
        });

        ArduinoOTA.onEnd([this]() {
            Serial.println("nFim!");
        });

        ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total) {
            Serial.printf("Progresso: %u%%r", (progress / (total / 100)));
        });

        ArduinoOTA.onError([this](ota_error_t error) {
            Serial.printf("Erro [%u]: ", error);
            if (error == OTA_AUTH_ERROR)
                Serial.println("Autenticacao Falhou");
            else if (error == OTA_BEGIN_ERROR)
                Serial.println("Falha no Inicio");
            else if (error == OTA_CONNECT_ERROR)
                Serial.println("Falha na Conexao");
            else if (error == OTA_RECEIVE_ERROR)
                Serial.println("Falha na Recepcao");
            else if (error == OTA_END_ERROR)
                Serial.println("Falha no Fim");
        });
        //ArduinoOTA.setPort(8266);
        //ArduinoOTA.setHostname(this->_hostname);
        //ArduinoOTA.setPassword((const char *)"123");

        ArduinoOTA.begin();
    }

    virtual void loop(void)
    {
        ArduinoOTA.handle();
    }

    bool update(String filename)
    {
        bool status = false;
        String uri = "/local/";

        WiFiClient client;

        uri.concat(filename);
        String hostname = "petitmaison.duckdns.org";
        unsigned int port = 8123;
        Serial.printf("Requesting update file from https://%s:%d%s\n", hostname.c_str(), port, uri.c_str());

        ESPhttpUpdate.rebootOnUpdate(false);
        delay(500);

        t_httpUpdate_return ret = ESPhttpUpdate.update(uri, "");
        switch (ret)
        {
        case HTTP_UPDATE_FAILED:
            Serial.printf("Update Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
            break;

        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("Update ignored");
            break;

        case HTTP_UPDATE_OK:
            status = true;
            Serial.println("HTTP update done");
            break;
        }

        return status;
    }

    void restart()
    {
        Serial.printf("Restarting...\n");
        delay(1000);
        ESP.restart();
        delay(5000);
    }

    void checkForUpdates()
    {
        String mac = WiFi.macAddress();
        String fwUrlBase = "https://petitmaison.duckdns.org:8123";
        String fwVersionURL = "/local/firmware_version";
        String fwFilename = "firmware.wemos.bin";

        Serial.println("Checking for firmware updates.");
        Serial.print("MAC address: ");
        Serial.println(mac);
        Serial.print("Firmware version URL: ");
        Serial.println(fwVersionURL);

        HTTPClient httpClient;
        httpClient.begin(fwVersionURL);
        int httpCode = httpClient.GET();
        if (httpCode == 200)
        {
            String newFWVersion = httpClient.getString();

            Serial.print("Current firmware version: ");
            Serial.println(FW_VERSION);
            Serial.print("Available firmware version: ");
            Serial.println(newFWVersion);

            if (newFWVersion.toInt() > String(FW_VERSION).toInt())
            {
                Serial.println("Preparing to update");
                this->update(fwFilename);
            }
            else
            {
                Serial.println("Already on latest version");
            }
        }
        else
        {
            Serial.print("Firmware version check failed, got HTTP response code ");
            Serial.println(httpCode);
            Serial.println(httpClient.getString());
        }
        httpClient.end();
    }
};

#endif
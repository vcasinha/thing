#ifndef WIFI_MODULE_H
#define WIFI_MODULE_H

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <WiFiManager.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include "Module.h"

class WiFiModule : public Module
{
    public:
        WiFiManager *_wifiManager;
        WiFiClient _wirelessClient;

        WiFiModule()
        {
            _name = "wifi";
            this->_wifiManager = new WiFiManager();
            this->_wifiManager->setDebugOutput(false);
        }

        void setup(void)
        {
            this->_wifiManager->setConfigPortalTimeout(180);
            this->_wifiManager->autoConnect();
            //ArduinoOTA.setPort(8266);
            //ArduinoOTA.setHostname(this->_hostname);
            //ArduinoOTA.setPassword((const char *)"123");
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

            ArduinoOTA.begin();
            this->checkForUpdates();
        }

        void checkForUpdates()
        {
        //     String fwUrlBase = "http://google.com";

        //     Serial.println("Checking for firmware updates.");
        //     Serial.print("MAC address: ");
        //     Serial.println(mac);
        //     Serial.print("Firmware version URL: ");
        //     Serial.println(fwVersionURL);

        //     HTTPClient httpClient;
        //     httpClient.begin(fwVersionURL);
        //     int httpCode = httpClient.GET();
        //     if (httpCode == 200)
        //     {
        //         String newFWVersion = httpClient.getString();

        //         Serial.print("Current firmware version: ");
        //         Serial.println(FW_VERSION);
        //         Serial.print("Available firmware version: ");
        //         Serial.println(newFWVersion);

        //         int newVersion = newFWVersion.toInt();

        //         if (newVersion > FW_VERSION)
        //         {
        //             Serial.println("Preparing to update");

        //             String fwImageURL = fwURL;
        //             fwImageURL.concat(".bin");
        //             t_httpUpdate_return ret = ESPhttpUpdate.update(fwImageURL);

        //             switch (ret)
        //             {
        //             case HTTP_UPDATE_FAILED:
        //                 Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        //                 break;

        //             case HTTP_UPDATE_NO_UPDATES:
        //                 Serial.println("HTTP_UPDATE_NO_UPDATES");
        //                 break;
        //             }
        //         }
        //         else
        //         {
        //             Serial.println("Already on latest version");
        //         }
        //     }
        //     else
        //     {
        //         Serial.print("Firmware version check failed, got HTTP response code ");
        //         Serial.println(httpCode);
        //     }
        //     httpClient.end();
        }

        virtual void loop(void)
        {
            ArduinoOTA.handle();
        }

        void addParameter(const char *id, const char *label, const char *value, unsigned int length)
        {
            //Serial.printf("Add parameter %s\n", id);
            WiFiManagerParameter *parameter = new WiFiManagerParameter(id, label, value, length);
            this->_wifiManager->addParameter(parameter);
            this->_parameters.push(parameter);
            //Serial.printf("Total %d\n", this->_parameters.size());
        }

        void addParameterHTML(const char *html)
        {
            //Serial.printf("Add HTML %s\n", html);
            WiFiManagerParameter *parameter = new WiFiManagerParameter(html);
            this->_wifiManager->addParameter(parameter);
        }

        WiFiManagerParameter *getParameter(const char *parameter_id, const char *default_value = "")
        {
            //Serial.printf("Find parameter %s %s\n", parameter_id, this->_name);
            for (unsigned int c = 0; c < this->_parameters.size(); c++)
            {
                WiFiManagerParameter *parameter = this->_parameters[c];
                String p = parameter->getID();
                if (p.equals(parameter_id))
                {
                    return parameter;
                }
            }

            Serial.printf("Not found parameter %s\n", parameter_id);

            return new WiFiManagerParameter("<p>none</p>");
        }

    private:
        Vector<WiFiManagerParameter *> _parameters;
};

#endif
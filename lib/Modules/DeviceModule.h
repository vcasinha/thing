#ifndef DEVICE_MODULE_H
    #define DEVICE_MODULE_H

    #include <Arduino.h>
    #include "Module.h"
    #include "WiFiModule.h"
    #include <ESP8266HTTPClient.h>
    #include <ESP8266httpUpdate.h>
    #include <ArduinoOTA.h>
    
    class DeviceModule : public Module
    {
        public:
            WiFiModule *_wifiModule;
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
                String url = "https://petitmaison.duckdns.org:8123/local/";
                String uri = "/local/";
                String fingerprint = "9E 13 A5 BF 39 B4 18 B4 AE BF 8F 87 B6 87 90 D6 EE AF 44 FB";
                const uint8_t httpsFingerprint[20] = {0x9E, 0x13, 0xA5, 0xBF, 0x39, 0xB4, 0x18, 0xB4, 0xAE, 0xBF, 0x8F, 0x87, 0xB6, 0x87, 0x90, 0xD6, 0xEE, 0xAF, 0x44, 0xFB};
                //String fingerprint = "9E:13:A5:BF:39:B4:18:B4:AE:BF:8F:87:B6:87:90:D6:EE:AF:44:FB";
                uri.concat(filename);
                url.concat(filename);

                Serial.printf("Requesting update file from %s\n", url.c_str());
                ESPhttpUpdate.rebootOnUpdate(true);
                delay(500);

                //t_httpUpdate_return ret = ESPhttpUpdate.update(url, "", fingerprint);
                t_httpUpdate_return ret = ESPhttpUpdate.update("petitmaison.duckdns.org", 8123, uri, "", httpsFingerprint);
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
    };

#endif
#ifndef WIFI_MODULE_H
#define WIFI_MODULE_H

#ifndef WIFI_CONNECT_TIMEOUT
#define WIFI_CONNECT_TIMEOUT 30
#endif

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include "Module.h"
#include "ServerModule.h"
#include "DeviceModule.h"

class WiFiModule : public Module
{
    public:
        WiFiClient _wifiClient;
        ServerModule * _serverModule;
        DeviceModule * _device;
        unsigned long _connectStartTime;
        unsigned int _timeSinceLastConnection = 0;
        unsigned long _elapsedTime = 0;
        MDNSResponder * _mdns;
        WiFiModule()
        {
            _name = "wifi";
        }

        void boot(JsonObject & config)
        {
            this->_serverModule = (ServerModule *)this->_application->getModule("server");
            this->_device = (DeviceModule *)this->_application->getModule("device");
            this->_mdns = new MDNSResponder();
            WiFi.begin();
        }

        void setup(void)
        {
            String hostname = ((DeviceModule *)this->_application->getModule("device"))->_hostname;

            this->initWiFiHandlers();
            this->attemptConnection("", "");

            Log.notice("(wiFiModule.setup) Configure MDNS with hostname %s.local", hostname.c_str());
            if (!this->_mdns->begin(hostname))
            {
                Log.error("(wiFiModule.setup) Failed to initialize mDNS responder");
            }
            else
            {
                Log.notice("(wiFiModule.setup) mDNS responder started");
                this->_mdns->addService("http", "tcp", 80);
            }

            WiFi.hostname(hostname.c_str());
        }

        void initWiFiHandlers()
        {
            IPAddress apIP(192, 168, 1, 1);
            this->_serverModule->_nameServer->setErrorReplyCode(DNSReplyCode::NoError);
            this->_serverModule->_nameServer->start((byte)53, "*", apIP);

            this->_serverModule->_webServer->on("/wifi/list", [&]() {
                if (!this->_serverModule->isAuthenticated())
                {
                    return;
                }
                //scan for wifi networks
                int n = WiFi.scanNetworks();

                //build array of indices
                int ix[n];
                for (int i = 0; i < n; i++)
                    ix[i] = i;

                //sort by signal strength
                for (int i = 0; i < n; i++)
                    for (int j = 1; j < n - i; j++)
                        if (WiFi.RSSI(ix[j]) > WiFi.RSSI(ix[j - 1]))
                            std::swap(ix[j], ix[j - 1]);
                //remove duplicates
                for (int i = 0; i < n; i++)
                    for (int j = i + 1; j < n; j++)
                        if (WiFi.SSID(ix[i]).equals(WiFi.SSID(ix[j])) && WiFi.encryptionType(ix[i]) == WiFi.encryptionType(ix[j]))
                            ix[j] = -1;

                //build plain text string of wifi info
                //format [signal%]:[encrypted 0 or 1]:SSID
                String s = "";
                s.reserve(2050);
                for (int i = 0; i < n && s.length() < 2000; i++)
                { //check s.length to limit memory usage
                    if (ix[i] != -1)
                    {
                        s += String(i ? "\n" : "") + ((constrain(WiFi.RSSI(ix[i]), -100, -50) + 100) * 2) + "," + ((WiFi.encryptionType(ix[i]) == ENC_TYPE_NONE) ? 0 : 1) + "," + WiFi.SSID(ix[i]);
                    }
                }

                this->_serverModule->_webServer->send(200, "text/plain", s);
            });

            this->_serverModule->_webServer->on("/wifi/connect", [&]() {
                if (!this->_serverModule->isAuthenticated())
                {
                    return;
                }
                this->_serverModule->_webServer->send(200, "text/html", "connecting...");
                this->attemptConnection(this->_serverModule->_webServer->arg("ssid"), this->_serverModule->_webServer->arg("password"));
            });

            this->_serverModule->_webServer->on("/wifi/ap", [&]() {
                if (!this->_serverModule->isAuthenticated())
                {
                    return;
                }
                this->_serverModule->_webServer->send(200, "text/html", "access point: " + this->_device->_hostname);
                delay(2000);
                startApMode();
            });

            this->_serverModule->_webServer->on("/wifi/reset", HTTP_GET, [&]() {
                if (!this->_serverModule->isAuthenticated())
                {
                    return;
                }

                Log.notice("(webServer.resetWiFi) WiFi reset requested");
                this->_serverModule->_webServer->send(200, "application/json", "{\"_status\":\"WiFi Reset (restarting)\"}");
                this->_serverModule->_webServer->handleClient();
                this->eraseWiFiSettings();
            });
        }

        void eraseWiFiSettings()
        {
            Log.notice("(Device) Erasing configuration.");
            ESP.eraseConfig();
            this->_device->restart();
        }

        bool attemptConnection(const String & ssid, const String & password)
        {
            //attempt to connect to wifi
            Log.notice("(wiFiModule.attemptConnection) Set WiFi to station mode");
            WiFi.mode(WIFI_STA);
            if (ssid.length())
            {
                if (password.length())
                {
                    Log.notice("(wiFiModule.attemptConnection) Connecting to Secure SSID");
                    WiFi.begin(ssid.c_str(), password.c_str());
                }
                else
                {
                    Log.notice("(wiFiModule.attemptConnection) Connecting to Public SSID");
                    WiFi.begin(ssid.c_str());
                }
            }
            else
            {
                Log.notice("(wiFiModule.attemptConnection) Connecting to Stored SSID");
                WiFi.begin();
            }
            //if in nonblock mode, skip this loop
            unsigned long connection_start = millis();
            Log.notice("(wiFiModule.attemptConnection) Attempt connection to %s", WiFi.SSID().c_str());
            delay(1000);
            while (WiFi.status() != WL_CONNECTED && (millis() - connection_start) < (1000 * WIFI_CONNECT_TIMEOUT))
            {
                Log.notice("(wiFiModule.attemptConnection) Retrying connection to SSID %u", (millis() - connection_start) / 1000);
                delay(2000);
            }

            if ((WiFi.status() == WL_CONNECT_FAILED) || ((WiFi.status() != WL_CONNECTED)))
            {
                Log.notice("(wiFiModule.attemptConnection) Start AP");
                startApMode();
                _connectStartTime = 0;
            }
            else
            {
                Log.notice("(wiFiModule.attemptConnection) Connected to %s", WiFi.SSID().c_str());
            }

            return (WiFi.status() == WL_CONNECTED);
        }

        void startApMode()
        {
            this->_timeSinceLastConnection = 0;
            //start AP mode
            IPAddress apIP(192, 168, 1, 1);
            WiFi.mode(WIFI_AP);
            WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
            if(this->_device->_secure)
            {
                Log.notice("(wiFiModule.startAPMode) Starting Secure AP %s", this->_device->_hostname.c_str());
                WiFi.softAP(this->_device->_hostname.c_str(), this->_device->_password.c_str());
            }
            else
            {
                Log.notice("(wiFiModule.startAPMode) Starting Public AP %s", this->_device->_hostname.c_str());
                WiFi.softAP(this->_device->_hostname.c_str());
            }

            Log.notice("(wiFiModule.startAPMode)### AP Mode http://%s.local (%s)", this->_device->_hostname.c_str(), WiFi.localIP().toString().c_str());
        }

        void loop(unsigned long delta_time)
        {
            //Check if still connected to WIFI
            //Start AP when necessary
            this->_elapsedTime += delta_time;
            if (this->_elapsedTime > 1000)
            {
                this->_elapsedTime = 0;
                if (WiFi.getMode() != WIFI_STA)
                {
                    this->_timeSinceLastConnection += 1;
                    Log.verbose("(wifiModule.loop) WiFi not connected since %d", this->_timeSinceLastConnection);
                    if (this->_timeSinceLastConnection > 300)
                    {
                        Log.notice("(wifiModule.loop) Disconnect AP (timeout)");
                        this->attemptConnection("", "");
                        this->_timeSinceLastConnection = 0;
                    }
                }
                else if (WiFi.getMode() == WIFI_STA && WiFi.status() != WL_CONNECTED)
                {
                    Log.notice("(wifiModule.loop) WiFi disconnected, reconnecting.");
                    this->attemptConnection("", "");
                }
            }
            MDNS.update();
        }
    private:

};

#endif
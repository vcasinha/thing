#ifndef SERVER_MODULE_H
#define SERVER_MODULE_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <PersWiFiManager.h>
#include <DNSServer.h>
#include <FS.h>

#include "Module.h"
#include "WiFiModule.h"

#define WIFI_HTM_PROGMEM

class ServerModule : public Module
{
  public:
    DeviceModule * _deviceModule;
    StorageModule * _storageModule;
    ESP8266WebServer * _webServer;
    DNSServer *_nameServer;
    PersWiFiManager * _wifiManager;
    

    ServerModule()
    {
        this->_name = "server";
    }

    ~ServerModule()
    {
    }

    virtual void boot(JsonObject &config)
    {
        this->_deviceModule = (DeviceModule *)this->_application->getModule("device");
        this->_storageModule = (StorageModule *)this->_application->getModule("storage");
        this->_nameServer = new DNSServer();
        this->_webServer = new ESP8266WebServer(80);
        
        this->_wifiManager = new PersWiFiManager(this->_webServer, this->_nameServer);
    }

    virtual void setup(void)
    {
        const char * hostname;
        hostname = this->_deviceModule->_hostname.c_str();
        MDNS.begin(hostname);

        this->_wifiManager->setApCredentials(this->_deviceModule->_hostname);
        this->_wifiManager->begin();

        Serial.printf("Open browser on http://%s.local\n", hostname);
        this->_webServer->on("/restart", HTTP_GET, [this]() {
            Serial.printf("Restart requested\n");
            this->_webServer->send(200, "application/json", "{\"_status\":\"Restarting\"}");
            this->_webServer->handleClient();
            this->_deviceModule->restart();
        });

        this->_webServer->on("/update", HTTP_GET, [this]() {
            Serial.printf("Update requested\n");
            if (!this->_webServer->hasArg("filename"))
            {
                this->_webServer->send(500, "application/json", "{\"_status\":\"filename argument is missing\"}");
                return;
            }

            this->_webServer->send(200, "application/json", "{\"_status\":\"update requested\"}");
            this->_webServer->handleClient();

            String filename = this->_webServer->arg("filename");
            this->_deviceModule->update(filename);
        });

        this->_webServer->on("/read", HTTP_GET, [this]() {
            this->handleFileRead();
        });

        this->_webServer->on("/write", HTTP_POST, [this]() {
            this->handleFileWrite();
        });

        this->_webServer->on("/list", HTTP_GET, [this]() {
            this->handleFileList();
        });

        this->_webServer->on("/all", HTTP_GET, [this]() {
            String json = "{";
            json += "\"heap\":" + String(ESP.getFreeHeap());
            json += ", \"analog\":" + String(analogRead(A0));
            json += ", \"gpio\":" + String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)));
            json += "}";
            this->_webServer->send(200, "application/json", json);
            json = String();
        });

        this->_webServer->begin();
    }

    virtual void loop(void)
    {
        this->_webServer->handleClient();
        this->_nameServer->processNextRequest();
    }

    void handleFileList()
    {
        if (!this->_webServer->hasArg("path"))
        {
            this->_webServer->send(500, "application/json", "{\"_status\":\"Path argument is missing\"}");
            return;
        }

        String path = this->_webServer->arg("path");
        Serial.println("handleFileList: " + path);

        Dir dir = this->_storageModule->opendir(path.c_str());
        path = String();

        String output = "[";
        while (dir.next())
        {
            File entry = dir.openFile("r");
            if (output != "[")
            {
                output += ',';
            }
            bool isDir = false;
            output += "{\"type\":\"";
            output += (isDir) ? "dir" : "file";
            output += "\",\"name\":\"";
            output += String(entry.name()).substring(1);
            output += "\"}";
            entry.close();
        }

        output += "]";
        this->_webServer->send(200, "text/json", output);
    }

    void handleFileRead()
    {
        String contentType = "application/json";

        if (!this->_webServer->hasArg("path"))
        {
            this->_webServer->send(500, "application/json", "{\"_status\":\"Path argument is missing\"}");
            return;
        }

        String path = this->_webServer->arg("path");
        Serial.printf("** Read file '%s'\n", path.c_str());
        if (this->_storageModule->exists(path.c_str()) == false)
        {
            Serial.printf("** File not found '%s'\n", path.c_str());
            this->_webServer->send(400, "application/json", "{\"_status\":\"File does not exist\"}");
            return;
        }

        File file = this->_storageModule->getFile(path.c_str(), "r");
        this->_webServer->streamFile(file, contentType);
        file.close();
    }

    void handleFileWrite()
    {
        String contentType = "application/json";

        if (!this->_webServer->hasArg("path"))
        {
            this->_webServer->send(500, "application/json", "{\"_status\":\"Path argument is missing\"}");
            return;
        }

        if (!this->_webServer->hasArg("plain"))
        {
            this->_webServer->send(500, "application/json", "{\"_status\":\"Body missing\"}");
            return;
        }
        String body = this->_webServer->arg("plain");
        String path = this->_webServer->arg("path");
        Serial.printf("** Write file '%s'\n", path.c_str());

        if(this->_storageModule->write(path.c_str(), body.c_str()) == false)
        {
            this->_webServer->send(500, "application/json", "{\"_status\":\"Could not write\"}");
            return;
        }

        this->_webServer->send(500, "application/json", "{\"_status\":\"OK\"}");
    }
};

#endif
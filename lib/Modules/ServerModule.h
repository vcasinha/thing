#ifndef SERVER_MODULE_H
#define SERVER_MODULE_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>

#include "Module.h"
#include "WiFiModule.h"
void (*resetFunc)(void) = 0;
class ServerModule : public Module
{
  public:
    DeviceModule * _deviceModule;
    StorageModule * _storageModule;
    ESP8266WebServer * _server;

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
    }

    virtual void setup(void)
    {
        const char * hostname;
        hostname = this->_deviceModule->_hostname.c_str();
        MDNS.begin(hostname);

        this->_server = new ESP8266WebServer(80);
        
        Serial.printf("Open browser on http://%s.local\n", hostname);
        this->_server->on("/restart", HTTP_GET, [this]() {
            Serial.printf("Restart requested\n");
            this->_server->send(200, "application/json", "{\"_status\":\"Restarting\"}");
            this->_server->handleClient();
            this->_deviceModule->restart();
        });
        this->_server->on("/read", HTTP_GET, [this]() {
            this->handleFileRead();
        });

        this->_server->on("/write", HTTP_POST, [this]() {
            this->handleFileWrite();
        });

        this->_server->on("/list", HTTP_GET, [this]() {
            this->handleFileList();
        });

        this->_server->on("/all", HTTP_GET, [this]() {
            String json = "{";
            json += "\"heap\":" + String(ESP.getFreeHeap());
            json += ", \"analog\":" + String(analogRead(A0));
            json += ", \"gpio\":" + String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)));
            json += "}";
            this->_server->send(200, "application/json", json);
            json = String();
        });

        this->_server->begin();
    }

    virtual void loop(void)
    {
        this->_server->handleClient();
    }

    void handleFileList()
    {
        if (!this->_server->hasArg("path"))
        {
            this->_server->send(500, "application/json", "{\"_status\":\"Path argument is missing\"}");
            return;
        }

        String path = this->_server->arg("path");
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
        this->_server->send(200, "text/json", output);
    }

    void handleFileRead()
    {
        String contentType = "application/json";

        if (!this->_server->hasArg("path"))
        {
            this->_server->send(500, "application/json", "{\"_status\":\"Path argument is missing\"}");
            return;
        }

        String path = this->_server->arg("path");
        Serial.printf("** Read file '%s'\n", path.c_str());
        if (this->_storageModule->exists(path.c_str()) == false)
        {
            Serial.printf("** File not found '%s'\n", path.c_str());
            this->_server->send(400, "application/json", "{\"_status\":\"File does not exist\"}");
            return;
        }

        File file = this->_storageModule->getFile(path.c_str(), "r");
        this->_server->streamFile(file, contentType);
        file.close();
    }

    void handleFileWrite()
    {
        String contentType = "application/json";

        if (!this->_server->hasArg("path"))
        {
            this->_server->send(500, "application/json", "{\"_status\":\"Path argument is missing\"}");
            return;
        }

        if (!this->_server->hasArg("plain"))
        {
            this->_server->send(500, "application/json", "{\"_status\":\"Body missing\"}");
            return;
        }
        String body = this->_server->arg("plain");
        String path = this->_server->arg("path");
        Serial.printf("** Write file '%s'\n", path.c_str());

        if(this->_storageModule->write(path.c_str(), body.c_str()) == false)
        {
            this->_server->send(500, "application/json", "{\"_status\":\"Could not write\"}");
            return;
        }

        this->_server->send(500, "application/json", "{\"_status\":\"OK\"}");
    }
};

#endif
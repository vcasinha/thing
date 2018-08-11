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

class ServerModule : public Module
{
  public:
    WiFiModule * _wifiModule;
    ESP8266WebServer * _server;

    ServerModule()
    {
        this->_name = "server";
        
    }

    ~ServerModule()
    {
        this->_loop_period_ms = 100;
    }

    virtual void boot(JsonObject &config)
    {
        this->_wifiModule = (WiFiModule *)this->_application->getModule("wifi");
    }

    virtual void setup(void)
    {
        const char * hostname;
        hostname = this->_wifiModule->getParameter("device_hostname")->getValue();
        this->_server = new ESP8266WebServer(80);
        MDNS.begin(hostname);
        Serial.printf("Open browser on http://%s.local\n", hostname);
        this->_server->on("/edit", HTTP_GET, [this]() {
            if (!handleFileRead("/edit.htm"))
            {
                this->_server->send(404, "text/plain", "FileNotFound");
            }
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
            this->_server->send(200, "text/json", json);
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
        if (!this->_server->hasArg("dir"))
        {
            this->_server->send(500, "text/plain", "BAD ARGS");
            return;
        }

        String path = this->_server->arg("dir");
        Serial.println("handleFileList: " + path);
        Dir dir = SPIFFS.openDir(path);
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

    bool handleFileRead(String path)
    {
        Serial.println("handleFileRead: " + path);
        if (path.endsWith("/"))
        {
            path += "index.htm";
        }
        String contentType = "application/json";
        String pathWithGz = path + ".gz";
        if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path))
        {
            if (SPIFFS.exists(pathWithGz))
            {
                path += ".gz";
            }
            
            File file = SPIFFS.open(path, "r");
            this->_server->streamFile(file, contentType);
            file.close();
            return true;
        }
        return false;
    }
};

#endif
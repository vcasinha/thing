
#ifndef SERVER_MODULE_H
#define SERVER_MODULE_H

#include <Arduino.h>
#include <ArduinoLog.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "PersWiFiManager.h"
#include <DNSServer.h>
#include <FS.h>

#include "Module.h"
#include "WiFiModule.h"
#include "StorageModule.h"
#include "DeviceModule.h"

class ServerModule : public Module
{
public:
    const char *_refreshHTML = "<script>window.location='/'</script><a href='/'>redirecting...</a>";
    DeviceModule *_deviceModule;
    StorageModule *_storageModule;
    ESP8266WebServer *_webServer;
    DNSServer *_nameServer;
    PersWiFiManager *_wifiManager;
    unsigned int _timeSinceLastConnection = 0;
    unsigned long _millisCounter = 0;

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
        this->_wifiManager->setConnectNonBlock(false);
        this->_wifiManager->setApCredentials(this->_deviceModule->_hostname, "password#");
        if(this->_wifiManager->begin())
        {
            Log.notice("(serverModule.setup) Connected to WIFI");
        }

        this->_webServer->on("/restart", HTTP_GET, [this]() {
            Log.notice("(Server) Restart requested");
            this->_webServer->send(200, "application/json", "{\"_status\":\"Restarting\"}");
            this->_webServer->handleClient();
            this->_deviceModule->restart();
        });

        this->_webServer->on("/wifi/reset", HTTP_GET, [this]() {
            Log.notice("(Server) WiFi reset requested");
            this->_webServer->send(200, "application/json", "{\"_status\":\"WiFi Reset (restarting)\"}");
            this->_webServer->handleClient();
            this->_deviceModule->eraseWiFiSettings();
        });

        this->_webServer->on("/update", HTTP_GET, [this]() {
            Log.notice("(Server) Update requested");
            if (!this->_webServer->hasArg("filename"))
            {
                this->_webServer->send(500, "application/json", "{\"_status\":\"filename argument is missing\"}");
                return;
            }

            String filename = this->_webServer->arg("filename");
            bool state = this->_deviceModule->update(filename);

            String response = "{\"_status\":\"";
            response.concat(state ? "success \"}" : "failed \"}");

            this->_webServer->send(state ? 200 : 500, "application/json", response);
            this->_webServer->handleClient();
        });

        this->_webServer->on("/configuration", HTTP_GET, [this]() {
            String contentType = "application/json";
            String path = "/configuration.json";

            Log.notice("(Server) ** Read configuration file '%s'", path.c_str());
            if (this->_storageModule->exists(path.c_str()) == false)
            {
                Log.notice("(Server) ** File not found '%s'", path.c_str());
                this->_webServer->send(400, "application/json", "{\"_status\":\"File does not exist\"}");
                return;
            }

            File file = this->_storageModule->getFile(path.c_str(), "r");
            this->_webServer->streamFile(file, contentType);
            file.close();
        });

        this->_webServer->on("/configuration", HTTP_POST, [this]() {
            String contentType = "application/json";
            String path = "/configuration.json";
            DynamicJsonDocument buffer(1024);

            Log.notice("(Server) ** Write configuration file '%s'", path.c_str());
            if (!this->_webServer->hasArg("plain"))
            {
                this->_webServer->send(500, "application/json", "{\"_status\":\"Body missing\"}");
                return;
            }
            String body = this->_webServer->arg("plain");

            if (this->_storageModule->write(path.c_str(), body.c_str()) == false)
            {
                this->_webServer->send(500, "application/json", "{\"_status\":\"Could not write\"}");
                return;
            }

            this->_webServer->send(200, "application/json", "{\"_status\":\"OK\"}");

            this->_application->updateConfiguration(body.c_str());
        });

        this->_webServer->on("/list", HTTP_GET, [this]() {
            if (!this->_webServer->hasArg("path"))
            {
                this->_webServer->send(500, "application/json", "{\"_status\":\"Path argument is missing\"}");
                return;
            }

            String path = this->_webServer->arg("path");
            Log.notice("(Server) handleFileList: %s", path.c_str());

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

        this->_webServer->on("/", HTTP_GET, [this]() {
            String uri = this->_webServer->uri();
            String output = "Frontpage";

            Log.notice("(Server) Serving %s ", uri.c_str());
            this->_webServer->send(200, "text/html", output);
        });

        this->_webServer->onNotFound([this]() {
            String uri = this->_webServer->uri();

            Log.notice("(Server) Default route, serving file: %s", uri.c_str());

            if (this->_storageModule->exists(uri.c_str()) == false)
            {
                Log.notice("(Server) Path not found %s ", uri.c_str());
                this->_webServer->sendHeader("Cache-Control", " max-age=172800");
                this->_webServer->send(302, "text/html", this->_refreshHTML);
                return;
            }

            File file = this->_storageModule->getFile(uri.c_str(), "r");
            this->_webServer->streamFile(file, getContentType(uri));
            file.close();
        });

        this->_webServer->begin();
    }

    String getContentType(String filename)
    { // convert the file extension to the MIME type
        if (filename.endsWith(".html"))
            return "text/html";
        else if (filename.endsWith(".css"))
            return "text/css";
        else if (filename.endsWith(".js"))
            return "application/javascript";
        else if (filename.endsWith(".ico"))
            return "image/x-icon";
        return "text/plain";
    }

    virtual void loop(unsigned long delta_time)
    {
        //Count time and reconnect if not connected
        this->_millisCounter += delta_time;
        if (this->_millisCounter > 1000 && WiFi.status() != WL_CONNECTED)
        {
            this->_timeSinceLastConnection += 1;
            this->_millisCounter = 0;
            Log.notice("WiFi not connected since %d", this->_timeSinceLastConnection);
            if (this->_timeSinceLastConnection > 300)
            {
                Log.notice("Attempting WiFi connection");
                this->_wifiManager->attemptConnection();
                this->_timeSinceLastConnection = 0;
            }
        }

        this->_webServer->handleClient();
        this->_nameServer->processNextRequest();
        MDNS.update();
    }
};

#endif

#ifndef SERVER_MODULE_H
#define SERVER_MODULE_H

#include <Arduino.h>
#include <ArduinoLog.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
//#include "PersWiFiManager.h"
#include <DNSServer.h>
#include <FS.h>

#include "Module.h"
#include "StorageModule.h"
#include "DeviceModule.h"

class ServerModule : public Module
{
public:
    const char *_refreshHTML = "<script>window.location='/'</script><a href='/'>redirecting...</a>";
    String _username;
    String _password;
    String _realm = "Authentication required";
    String _authFailMessage = "Authentication failed";
    DeviceModule *_deviceModule;
    StorageModule *_storageModule;
    ESP8266WebServer *_webServer;
    DNSServer *_nameServer;
    File _uploadFile;

    //PersWiFiManager *_wifiManager;

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
        //this->_wifiManager = new PersWiFiManager(this->_webServer, this->_nameServer);

    }

    virtual void setup(void)
    {
        // this->_wifiManager->setConnectNonBlock(false);
        // this->_wifiManager->setApCredentials(this->_deviceModule->_hostname, "password#");
        // if(this->_wifiManager->begin())
        // {
        //     Log.notice("(serverModule.setup) Connected to WIFI");
        // }

        this->_webServer->on("/restart", HTTP_GET, [this]() {
            if (!this->isAuthenticated()) {
                return;
            }

            Log.notice("(webServer.restart) Restart requested");
            this->_webServer->send(200, "application/json", "{\"_status\":\"Restarting\"}");
            this->_webServer->handleClient();
            this->_deviceModule->restart();
        });

        // this->_webServer->on("/update", HTTP_GET, [this]() {
        //     if (!this->isAuthenticated()) {
        //         return;
        //     }

        //     Log.notice("(Server) Update requested");
        //     if (!this->_webServer->hasArg("filename"))
        //     {
        //         this->_webServer->send(500, "application/json", "{\"_status\":\"filename argument is missing\"}");
        //         return;
        //     }

        //     String filename = this->_webServer->arg("filename");
        //     bool state = this->_deviceModule->update(filename);

        //     String response = "{\"_status\":\"";
        //     response.concat(state ? "success \"}" : "failed \"}");

        //     this->_webServer->send(state ? 200 : 500, "application/json", response);
        //     this->_webServer->handleClient();
        // });
        this->_webServer->on("upload", HTTP_POST, [&]() {
            if (!this->isAuthenticated())
                return;

            this->_webServer->send(200); }, [&]() {
            this->processUpload();
        });

        this->_webServer->on("/file", HTTP_GET, [this]() {
            if (!this->isAuthenticated())
            {
                return;
            }

            String content_type = "application/json";

            if(!this->_webServer->hasArg("path"))
            {
                Log.error("(web) 'path' is required");
                this->_webServer->send(500, "application/json", "{\"message\":\"'path' is required\"}");
                return;
            }

            String path = this->_webServer->arg("path");

            Log.notice("(webServer.configuration.GET) ** Read configuration file '%s'", path.c_str());
            if (this->_storageModule->exists(path.c_str()) == false)
            {
                Log.error("(web) File not found '%s'", path.c_str());
                this->_webServer->send(400, "application/json", "{\"message\":\"File not found\"}");
                return;
            }

            File file = this->_storageModule->getFile(path.c_str(), "r");
            this->_webServer->streamFile(file, content_type);
            file.close();
        });

        this->_webServer->on("/file", HTTP_POST, [&]() {
            if (!this->isAuthenticated())
            {
                return;
            }

            String content_type = "application/json";

            if (!this->_webServer->hasArg("path"))
            {
                Log.error("(web) 'path' is required");
                this->_webServer->send(500, "application/json", "{\"message\":\"'path' is required\"}");
                return;
            }

            String path = this->_webServer->arg("path");
            Log.notice("(web.configuration.POST) Write file '%s'", path.c_str());

            if (!this->_webServer->hasArg("plain"))
            {
                this->_webServer->send(500, "application/json", "{\"message\":\"Body missing\"}");
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

        this->_webServer->on("/configuration", HTTP_GET, [this]() {
            if (!this->isAuthenticated()) {
                return;
            }

            String contentType = "application/json";
            String path = "/configuration.json";

            Log.notice("(webServer.configuration.GET) ** Read configuration file '%s'", path.c_str());
            if (this->_storageModule->exists(path.c_str()) == false)
            {
                Log.notice("(Server) ** File not found '%s'", path.c_str());
                this->_webServer->send(400, "application/json", "{\"message\":\"File does not exist\"}");
                return;
            }

            File file = this->_storageModule->getFile(path.c_str(), "r");
            this->_webServer->streamFile(file, contentType);
            file.close();
        });

        this->_webServer->on("/configuration", HTTP_POST, [this]() {
            if (!this->isAuthenticated()) {
                return;
            }

            String contentType = "application/json";
            String path = "/configuration.json";

            Log.notice("(web.configuration.POST) ** Write configuration file '%s'", path.c_str());
            if (!this->_webServer->hasArg("plain"))
            {
                this->_webServer->send(500, "application/json", "{\"message\":\"Body missing\"}");
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
            if (!this->isAuthenticated()) {
                return;
            }

            if (!this->_webServer->hasArg("path"))
            {
                this->_webServer->send(500, "application/json", "{\"_status\":\"Path argument is missing\"}");
                return;
            }

            Log.notice("(web.list.GET) Get list of files");

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

        this->_webServer->on("/info", HTTP_GET, [&]() {
            if (!this->isAuthenticated()) {
                return;
            }

            DynamicJsonDocument buffer(256);
            JsonObject json = buffer.to<JsonObject>();
            String response;

            json["hostname"] = this->_deviceModule->_hostname;
            json["wifiMode"] = WiFi.getMode() == WIFI_STA ? "Station" : "AP";
            json["SSID"] = WiFi.SSID();
            json["gpio"] = String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)), BIN);
            json["flashRealSize"] = ESP.getFlashChipRealSize();
            json["flashChipSize"] = ESP.getFlashChipSize();
            json["flashChipSpeed"] = ESP.getFlashChipSpeed();
            FlashMode_t ideMode = ESP.getFlashChipMode();
            json["flashMode"] = (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN");

            serializeJson(buffer, response);

            this->_webServer->send(200, "application/json", response);
        });

        this->_webServer->on("/", HTTP_GET, [&]() {
            if (!this->isAuthenticated()) {
                return;
            }
            String uri = this->_webServer->uri();
            String output = "Frontpage";

            Log.notice("(Server) Serving %s ", uri.c_str());
            this->_webServer->send(200, "text/html", output);
        });

        this->_webServer->onNotFound([&]() {
            if (!this->isAuthenticated()) {
                return;
            }
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

    bool isAuthenticated()
    {
        if(!this->_deviceModule->_secure)
        {
            return true;
        }

        bool authenticated = this->_webServer->authenticate(this->_deviceModule->_hostname.c_str(), this->_deviceModule->_password.c_str());
        if(!authenticated)
        {
            this->_webServer->requestAuthentication();
        }

        return authenticated;
    }

    void processUpload()
    {
        HTTPUpload &upload = this->_webServer->upload();
        if (upload.status == UPLOAD_FILE_START)
        {
            String filename = upload.filename;
            if (!filename.startsWith("/"))
            {
                filename = "/" + filename;
            }

            Serial.print("handleFileUpload Name: ");
            Serial.println(filename);
            this->_uploadFile = SPIFFS.open(filename, "w"); // Open the file for writing in SPIFFS (create if it doesn't exist)
            filename = String();
        }
        else if (upload.status == UPLOAD_FILE_WRITE)
        {
            if (this->_uploadFile)
                this->_uploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
        }
        else if (upload.status == UPLOAD_FILE_END)
        {
            if (this->_uploadFile)
            {                              // If the file was successfully created
                this->_uploadFile.close(); // Close the file again
                Serial.print("handleFileUpload Size: ");
                Serial.println(upload.totalSize);
                this->_webServer->send(200, "application/json", "{\"_status\":\"OK\"}");
            }
            else
            {
                this->_webServer->send(500, "application/json", "{\"_status\":\"Could not write\"}");
            }
        }
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
        this->_webServer->handleClient();
        this->_nameServer->processNextRequest();
    }
};

#endif
#ifndef APPLICATION_H
#define APPLICATION_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include "Vector.h"
#include "Module.h"

class Module;
class Application
{
public:
    WiFiClient _wifiClient;
    PubSubClient _client;
    const char *_id;
    bool _safeMode = false;
    bool _defaultConfig = false;
    String _hostname;

    Application(const char *);

    Module *getModule(const char *);
    void setup(void);
    void loop(void);
    void loadModule(Module *);
    void updateConfiguration(const char *);

private:
    Vector<Module *> _modules;
};
#endif
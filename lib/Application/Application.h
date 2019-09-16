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
            const char * _id;
            String _hostname;

            Application(const char * id);

            Module * getModule(const char * name);
            void setup(void);
            void loop(void);
            void loadModule(Module * module);
            void updateConfiguration(const char *config_json);

        private:
            Vector<Module *> _modules;
    };
#endif
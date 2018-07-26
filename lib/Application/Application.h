#ifndef APPLICATION_H
#define APPLICATION_H

    #include <Arduino.h>
    #include <PubSubClient.h>
    #include "Vector.h"

    #include "Module.h"

    #define SERIAL_MONITOR_BAUD 115200
    class Module;
    class Application
    {
        public:
            PubSubClient _client;
            
            Application();

            void callback(char * topic, unsigned char * payload, unsigned int length)
            {

            }

            void loadModule(Module * module);
            Module * getModule(const char * name);
            void setup(void);
            void loop(void);
        private:
            Vector<const char *> _modulesNames;
            Vector<Module *> _modules;
    };

#endif
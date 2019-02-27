#ifndef APPLICATION_H
#define APPLICATION_H

    #include <Arduino.h>
    #include <PubSubClient.h>
    #include "Vector.h"

    #include "Module.h"

    class Module;
    class Application
    {
        public:
            PubSubClient _client;
            const char * _id;

            Application(const char * id);

            Module * getModule(const char * name);
            void setup(void);
            void loop(void);
            void loadModule(Module * module);
        private:
            Vector<Module *> _modules;
    };
#endif
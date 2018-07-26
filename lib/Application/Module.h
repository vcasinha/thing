#ifndef MODULE_H
#define MODULE_H

    #include <Arduino.h>
    #include "Application.h"
    class Application;
    class Module
    {
        public:
            const char * _name = "unnamed";
            Module();
            void setApplication(Application * application);
            virtual void boot() {};
            virtual void setup() {};
            virtual void loop() {};
            
        protected:
            Application * _application;
    };

#endif
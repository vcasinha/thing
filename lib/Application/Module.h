#ifndef MODULE_H
#define MODULE_H

    #include <Arduino.h>
    #include <ArduinoJson.h>
    #include "Application.h"
    class Application;
    class Module
    {
        public:
            const char * _name = "unnamed";
            unsigned long _last_update = 0;
            unsigned long _update_period = 1;
            Module()
            {

            }

            void setApplication(Application * application)
            {
                this->_application = application;
            }
            
            virtual void boot(JsonObject & config) {};
            virtual void setup() {};
            virtual void loop() {};
            virtual void callback(char * topic, unsigned char * payload, unsigned int length) {};
        protected:
            Application * _application;
    };

#endif
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
            unsigned long _loop_time = 0;
            unsigned long _loop_period_ms = 0;
            unsigned long _last_update = 0;
            bool _enabled = true;
            bool _safeMode = false;

            Module()
            {
            }

            void setApplication(Application * application)
            {
                this->_application = application;
            }

            void ready()
            {
                if(this->_enabled)
                {
                    this->_last_update = millis();
                    Log.notice("(module.ready) Attaching '%s' with period %l ms", this->_name, this->_loop_period_ms);
                }
            }

            void init(const char * name, unsigned long loop_period_ms = 0)
            {
                this->_name = name;
                if(loop_period_ms > 0)
                {
                    this->_loop_period_ms = loop_period_ms;
                }
            }

            void disable()
            {
                _name = name;
                _loop_period_ms = period;
            }

            virtual void boot(JsonObject &) {};
            virtual void config(JsonObject &){};
            virtual void setup() {};
            virtual void loop(unsigned long) {};
            virtual String status() { return "Not implemented";};
            virtual void callback(char * topic, unsigned char * payload, unsigned int length) {};
            void disable(void) { this->_enabled = false; }
            void enable(void) { this->_enabled = true; }

        protected:
            Application * _application;
    };

#endif
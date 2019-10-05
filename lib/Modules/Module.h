#ifndef MODULE_H
#define MODULE_H

    #include <Arduino.h>
    #include <ArduinoLog.h>
    #include <Ticker.h>
    #include <ArduinoJson.h>
    #include "Application.h"

    class Application;
    class Module
    {
        public:
            const char * _name = "unnamed";
            unsigned long _loop_period_ms = 1000;
            unsigned long _last_update = 0;
            Ticker * _ticker;
            bool _enabled = true;
            bool _safeMode = false;

            Module()
            {
                this->_ticker = new Ticker();
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
                    this->_ticker->attach_ms_scheduled(this->_loop_period_ms, [&]() {
                        unsigned long now = millis();
                        unsigned long delta = now - this->_last_update;
                        this->_last_update = now;
                        this->loop(delta);
                    });
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
                this->_enabled = false;
                this->_ticker->detach();
            }

            virtual void boot(JsonObject &) {};
            virtual void config(JsonObject &){};
            virtual void setup() {};
            virtual void loop(unsigned long) {};
            virtual String status() { return "Not implemented";};
            virtual void callback(char * topic, unsigned char * payload, unsigned int length) {};
            void enable(void) { this->_enabled = true; }

        protected:
            Application * _application;
    };

#endif
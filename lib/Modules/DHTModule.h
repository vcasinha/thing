#include <Arduino.h>
#include <ArduinoJson.h>
#include <dht.h>
#include "Module.h"

#include "MQTTModule.h"


class DHTModule: public Module 
{
    public:
        MQTTModule * _mqtt;
        WiFiModule * _wifiModule;
        unsigned int _counter = 0;
        float _humidity;
        float _temperature;
        dht _dht;
        unsigned int _pin;
        const char * default_configuration = "{\"pin\":\"value\"}";
        WiFiManagerParameter * _pinParameter;
        
        DHTModule()
        {
            this->_name = "sensor.dht";
            this->_loop_period_ms = 60 * 1000;
        }

        virtual void boot(JsonObject & config)
        {
            String pin_s((int)config["pin"]);

            this->_mqtt = (MQTTModule *) this->_application->getModule("mqtt");
            this->_wifiModule = (WiFiModule * ) this->_application->getModule("wifi");
            this->_wifiModule->addParameterHTML("<p>DHT</p>");
            this->_wifiModule->addParameter("dht_pin", "Pin number", pin_s.c_str(), 2);

            this->_mqtt->registerCallback(this);
        }

        virtual void setup(void)
        {
            Serial.printf("DHT %s\n", DHT_LIB_VERSION);
            String tnp = this->_wifiModule->getParameter("dht_pin")->getValue();
            this->_pin = tnp.toInt();
            Serial.printf("DHT PIN: %d\n", this->_pin);
        }

        virtual DHTModule * instance(void)
        {
            DHTModule * instance = new DHTModule();
            return instance;
        }

        virtual void loop(void)
        {
            String s;
            float humidity = this->_dht.humidity;
            float temperature = this->_dht.temperature;
            int status = this->_dht.read11(this->_pin);

            switch (status)
            {
                case DHTLIB_OK:  
                    // DISPLAY DATA
                    Serial.printf("Humidity %.2f%% Temperature %.2fº\n", humidity, temperature);
                    s = String(temperature);
                    this->_mqtt->publishState("sensor", "temperature", s.c_str());
                    s = String(humidity);
                    this->_mqtt->publishState("sensor", "humidity", s.c_str());
                    break;
                case DHTLIB_ERROR_CHECKSUM:
                    Serial.print("Checksum error\n"); 
                    break;
                case DHTLIB_ERROR_TIMEOUT: 
                    Serial.print("Time out error\n"); 
                    break;
                default: 
                    Serial.print("Unknown error\n"); 
                    break;
            }
        }

        void callback(char * topic, unsigned char * payload, unsigned int length)
        {
            Serial.printf("Inside M1 topic %s\n", topic);
        }

};

#include <Arduino.h>
#include <ArduinoJson.h>
#include <dht.h>
#include "Module.h"

#include "MQTTModule.h"
#include "ConfigModule.h"


class DHTModule: public Module 
{
    public:
        MQTTModule * _mqtt;
        ConfigModule * _config;
        unsigned int _counter = 0;
        float _humidity;
        float _temperature;
        dht _dht;
        unsigned int _pin;
        const char * default_configuration = "{\"pin\":\"value\"}";
        DHTModule(unsigned int pin)
        {
            this->_name = "DHT";
            this->_update_period = 60 * 1000;
            this->_pin = pin;
        }

        virtual void boot(JsonObject & config)
        {
            this->_mqtt = (MQTTModule *) this->_application->getModule("MQTT");
            this->_mqtt->registerCallback(this);
            this->_pin = config["pin"] | D4;
            Serial.printf("DHT PIN: %d\n", this->_pin);
        }

        virtual void setup(void)
        {
            Serial.println(DHT_LIB_VERSION);
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
                    this->_mqtt->_client.publish("home/sensor/office_temperature/state", s.c_str());
                    s = String(humidity);
                    this->_mqtt->_client.publish("home/sensor/office_humidity/state", s.c_str());
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

#ifndef RFMODULE_H
#define RFMODULE_H
#define RFMQTT_RAW 0
#define RFMQTT_BINARY 1
#define RFMQTT_TRISTATE 2
#define RFMQTT_HEX 3

#include <Arduino.h>
#include <ArduinoLog.h>
#include <ArduinoJson.h>
#include <RCSwitch.h>
#include "Module.h"
#include "MQTTModule.h"
#include "Vector.h"
// todo Sonoff RF Bridge (ESPURNA https://github.com/xoseperez/espurna/blob/dev/code/espurna/rfbridge.ino)
class RFModule : public Module
{
    public:
        unsigned int _format = RFMQTT_HEX;
        MQTTModule *_mqtt;
        RCSwitch _driver;
        unsigned int _rx_pin = 0;
        unsigned int _tx_pin = 0;
        bool _updated = false;
        unsigned long _value;
        unsigned int _delay = 500;
        unsigned long _lastTime = 0;

        RFModule()
        {
            this->init("rf", 100);
        }

        virtual void boot(JsonObject &config)
        {
            this->_mqtt = (MQTTModule *)this->_application->getModule("mqtt");
            this->_mqtt->registerCallback(this);

            this->_rx_pin = config["receive_pin"] | 0;
            this->_tx_pin = config["transmit_pin"] | 0;
            this->_mqtt->subscribe("home/switch/rf/command");
            this->_driver.enableTransmit(this->_tx_pin);
            this->_driver.enableReceive(this->_rx_pin);
            Log.notice("(RF.boot) Receive pin: %d Transmit pin: %d", this->_rx_pin, this->_tx_pin);
        }

        virtual void loop(unsigned long delta_time)
        {
            char payload[128] = "TRIGGER";
            char topic[100];

            if (this->_driver.available())
            {
                this->_updated = true;
                unsigned long value = this->_driver.getReceivedValue();
                unsigned int bit_length = this->_driver.getReceivedBitlength();
                unsigned int delay = this->_driver.getReceivedDelay();
                unsigned int protocol = this->_driver.getReceivedProtocol();

                this->_driver.resetAvailable();
                StaticJsonDocument<256> jsonBuffer;
                JsonObject json_object = jsonBuffer.to<JsonObject>();

                const char *id = dec2binWzerofill(value, bit_length);
                const char *tristate = bin2tristate(id);
                unsigned long current = millis();

                if (value == this->_value && current < this->_lastTime + this->_delay)
                {
                    this->_lastTime = current;
                        Log.notice("(RF.loop) Ignoring repetition...");
                    return;
                }

                json_object["element_id"] = this->_application->_id;

                json_object["id"] = id;
                json_object["protocol"] = protocol;
                json_object["delay"] = delay;
                json_object["bit_length"] = bit_length;
                json_object["tristate"] = tristate;

                if(tristate != NULL)
                {
                    Log.notice("(RF.loop) Received Tri state(%s): - %X", tristate, value);
                    sprintf(topic, "home/switch/rf/%.11s/state", tristate);
                }
                else
                {
                    Log.notice("(RF.loop) Received: %X", value);
                    sprintf(topic, "home/switch/rf/%06lX/state", value);
                }

                this->_mqtt->publish(topic, payload);

                this->_value = value;
                this->_lastTime = current;
            }
        }

        void callback(char *topic, unsigned char *payload, unsigned int length)
        {
            payload[length] = '\0';
            //topic[length] = '\0';
            String topic_string = String(topic);
            Log.notice("(RF.callback) Bridge topic %s", topic_string.c_str());
            if (topic_string.startsWith("home/switch/rf/command"))
            {
                //Serial.printf("RF Bridge code %s", topic_string.c_str());

                Log.notice("(RF.callback) Code: %s", (char *)payload);
                switch (this->_format)
                {
                case RFMQTT_TRISTATE:
                    Log.notice("(RF.callback) *  transmit Tristate %s", (char *)payload);
                    this->_driver.sendTriState((char *)payload);
                    // char tmp[100];
                    // sprintf(tmp, "home/switch/rf/%.11s/state", payload);
                    // this->_mqtt->publish(topic, (char *)payload, length);
                    break;
                case RFMQTT_BINARY:
                    Log.notice("(RF.callback) * transmit Binary %s", (char *)payload);
                    this->_driver.send((char *)payload);
                    break;
                }
            }
        }

        static const char *bin2tristate(const char *bin)
        {
            static char returnValue[50];
            int pos = 0;
            int pos2 = 0;
            while (bin[pos] != '\0' && bin[pos + 1] != '\0')
            {
                if (bin[pos] == '0' && bin[pos + 1] == '0')
                {
                    returnValue[pos2] = '0';
                }
                else if (bin[pos] == '1' && bin[pos + 1] == '1')
                {
                    returnValue[pos2] = '1';
                }
                else if (bin[pos] == '0' && bin[pos + 1] == '1')
                {
                    returnValue[pos2] = 'F';
                }
                else
                {
                    return NULL;
                }
                pos = pos + 2;
                pos2++;
            }
            returnValue[pos2] = '\0';
            return returnValue;
        }

        static char *dec2binWzerofill(unsigned long Dec, unsigned int bitLength)
        {
            static char bin[64];
            unsigned int i = 0;

            while (Dec > 0)
            {
                bin[32 + i++] = ((Dec & 1) > 0) ? '1' : '0';
                Dec = Dec >> 1;
            }

            for (unsigned int j = 0; j < bitLength; j++)
            {
                if (j >= bitLength - i)
                {
                    bin[j] = bin[31 + i - (j - (bitLength - i))];
                }
                else
                {
                    bin[j] = '0';
                }
            }
            bin[bitLength] = '\0';

            return bin;
        }
};
#endif
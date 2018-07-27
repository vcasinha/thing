#ifndef RFMODULE_H
    #define RFMODULE_H
    #define RFMQTT_RAW 0
    #define RFMQTT_BINARY 1
    #define RFMQTT_TRISTATE 2

    #include <Arduino.h>
    #include <ArduinoJson.h>
    #include <RCSwitch.h>
    #include "Module.h"
    #include "MQTTModule.h"
    #include "Vector.h"

    class RFModule: public Module 
    {
        public:
            unsigned int _format = RFMQTT_TRISTATE;
            MQTTModule * _mqtt;
            unsigned int _counter = 0;
            RCSwitch _driver;
            unsigned int _rx_pin = D4;
            unsigned int _tx_pin = D5;
            bool _updated = false;
            unsigned long _value;
            unsigned int _bitLength;
            unsigned int _protocol;
            unsigned int _delay;

            RFModule(unsigned int rx_pin, unsigned int tx_pin)
            {
                this->_name = "RF";
                this->_update_period = 50;
                this->_rx_pin = rx_pin;
                this->_tx_pin = tx_pin;
            }

            virtual void boot(JsonObject & config)
            {
                this->_mqtt = (MQTTModule *) this->_application->getModule("MQTT");
                this->_mqtt->registerCallback(this);
            }

            virtual void setup(void)
            {
                this->_driver.enableTransmit(_tx_pin);
                this->_driver.enableReceive(_rx_pin);
            }

            virtual void loop(void)
            {
                if(this->_driver.available())
                {
                    this->_updated = true;
                    unsigned long value = this->_driver.getReceivedValue();
                    unsigned int bit_length = this->_driver.getReceivedBitlength();
                    unsigned int delay = this->_driver.getReceivedDelay();
                    unsigned int protocol = this->_driver.getReceivedProtocol();

                    this->_driver.resetAvailable();
                    StaticJsonBuffer<200> jsonBuffer;
                    JsonObject& root = jsonBuffer.createObject();

                    const char * id = dec2binWzerofill(value, bit_length);
                    const char * tristate = bin2tristate(id);

                    Serial.printf("RF Received: %ld %s %s\n", value, id, tristate);

                    root["element_id"] = this->_application->_id;
                    root["id"] = id;
                    root["protocol"] = protocol;
                    root["delay"] = delay;
                    root["bit_length"] = bit_length;
                    root["tristate"] = tristate;

                    char payload[256];
                    char topic[100];

                    root.printTo(payload);
                    sprintf(topic, "home/binary_sensor/%s/state", id);
                    
                    this->_mqtt->publish(topic, payload);
                }
                this->_counter++;
            }

            void callback(char * topic, unsigned char * payload, unsigned int length)
            {
                //topic[length] = '\0';
                String topic_string = String(topic);
                Serial.printf("RF Bridge topic %s\n", topic_string.c_str());

                if(topic_string.startsWith("home/rf/main/command"))
                {
                    //Serial.printf("RF Bridge code %s\n", topic_string.c_str());

                    Serial.printf("   RF Code: %s\n", (char *)payload);
                    switch(this->_format)
                    {
                        case RFMQTT_TRISTATE:
                            this->_driver.sendTriState((char *)payload);
                            break;
                        case RFMQTT_BINARY:
                            this->_driver.send((char *)payload);
                            break;
                    }
                }
            }

            static const char* bin2tristate(const char* bin) 
            {
                static char returnValue[50];
                int pos = 0;
                int pos2 = 0;
                while (bin[pos]!='\0' && bin[pos+1]!='\0') 
                {
                    if (bin[pos]=='0' && bin[pos+1]=='0') 
                    {
                        returnValue[pos2] = '0';
                    } 
                    else if (bin[pos]=='1' && bin[pos+1]=='1') 
                    {
                        returnValue[pos2] = '1';
                    } 
                    else if (bin[pos]=='0' && bin[pos+1]=='1') 
                    {
                        returnValue[pos2] = 'F';
                    } 
                    else 
                    {
                        return "not applicable";
                    }
                    pos = pos+2;
                    pos2++;
                }
                returnValue[pos2] = '\0';
                return returnValue;
            }

            static char * dec2binWzerofill(unsigned long Dec, unsigned int bitLength) 
            {
                static char bin[64]; 
                unsigned int i=0;

                while (Dec > 0) {
                    bin[32+i++] = ((Dec & 1) > 0) ? '1' : '0';
                    Dec = Dec >> 1;
                }

                for (unsigned int j = 0; j< bitLength; j++) 
                {
                    if (j >= bitLength - i) 
                    {
                        bin[j] = bin[ 31 + i - (j - (bitLength - i)) ];
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
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
            MQTTModule * _mqttModule;
            WiFiModule * _wifiModule;
            RCSwitch _driver;
            unsigned int _rx_pin = D4;
            unsigned int _tx_pin = D5;
            bool _updated = false;
            const char * _value = "";
            unsigned int _delay = 500;
            unsigned int _lastTime = 0;

            RFModule()
            {
                this->_name = "rf";
                this->_loop_period_ms = 100;
            }

            virtual void boot(JsonObject & config)
            {
                this->_wifiModule = (WiFiModule * ) this->_application->getModule("wifi");
                this->_mqttModule = (MQTTModule *) this->_application->getModule("mqtt");
                this->_mqttModule->registerCallback(this);

                this->_wifiModule->addParameterHTML("<p>DHT</p>");
                String tmp = String((int)config["transmit_pin"]);
                this->_wifiModule->addParameter("rf_transmit_pin", "Transmit pin", tmp.c_str(), 2);
                tmp = String((int)config["receive_pin"]);
                this->_wifiModule->addParameter("rf_receive_pin", "Receive pin", tmp.c_str(), 2);
            }

            virtual void setup(void)
            {
                String p;

                p = this->_wifiModule->getParameter("rf_receive_pin")->getValue();
                this->_rx_pin = p.toInt();
                p = this->_wifiModule->getParameter("rf_transmit_pin")->getValue();
                this->_tx_pin = p.toInt();

                this->_driver.enableTransmit(this->_tx_pin);
                this->_driver.enableReceive(this->_rx_pin);

                Serial.printf("RF Receive pin: %d Transmit pin: %d\n", this->_rx_pin, this->_tx_pin);
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
                    unsigned int current = millis();
                    if(strcmp(tristate, this->_value) == 0 && current < this->_lastTime + this->_delay)
                    {
                        Serial.printf("Ignoring...\n");
                        return;
                    }
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
                    
                    this->_mqttModule->publishState("binary_sensor", "rf", payload);
                    this->_mqttModule->publish(topic, payload);
                    this->_value = tristate;
                    this->_lastTime = current;
                }
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
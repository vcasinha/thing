#ifndef CONFIGMODULE_H
    #define CONFIGMODULE_H
    #include <Arduino.h>
    #include <ArduinoJson.h>
    #include <RCSwitch.h>
    #include <FS.h>

    #include "Module.h"
    #include "MQTTModule.h"

    // template<typename DATA>
    // class ConfigElement
    // {
    //     public:
    //         DATA _element;
    //         const char * _label;

    //         void setLabel(const char * label)
    //         {
    //             this->_label = label;
    //         }

    //         void set(DATA value)
    //         {
    //             this->value = value;
    //         }
    // };

    class ConfigModule: public Module 
    {
        public:
            MQTTModule * _mqtt;
            ConfigModule()
            {
                this->_name = "config";
                this->_update_period = 1000;
                bool result = SPIFFS.begin();
                if(result)
                {
                    Serial.printf("    File system initialized\n");
                    Dir dir = SPIFFS.openDir("/");
                    Serial.printf("    List of files\n");
                    while (dir.next()) 
                    {
                        File f = dir.openFile("r");
                        Serial.printf("    - Filename: %s - %d\n", dir.fileName().c_str(), f.size());
                        f.close();
                    }
                }
                else
                {
                    Serial.printf("\n\n*** File system could not initalize ***\n");
                    abort();
                }
            }

            virtual void boot(JsonObject & config)
            {
                this->_mqtt = (MQTTModule *) this->_application->getModule("MQTT");
                this->_mqtt->registerCallback(this);
            }

            virtual void setup(void)
            {
            }

            void callback(char * topic, unsigned char * payload, unsigned int length)
            {
            }

            void store(const char * module_name, const char * data)
            {
                char filename[255];

                if(strlen(module_name) > 255)
                {
                    Serial.printf("\n*** ABORTING - Filename too long for module %s\n", module_name);
                    abort();
                }

                sprintf(filename, "/module_%s.json", module_name);
                File file = SPIFFS.open(filename, "w");
                if (!file) 
                {
                    Serial.printf("Could not open file %s\n", filename);
                }

                Serial.printf("Writing on %s the following data:\n%s\n", filename, data);

                file.println(data);
                file.close();
            }

            const char * read(const char * module_name)
            {
                char filename[255];
                if(strlen(module_name) > 255)
                {
                    Serial.printf("    ABORTING - Filename too long for module %s\n", module_name);
                    abort();
                }

                sprintf(filename, "/module_%s.json", module_name);
                if(SPIFFS.exists(filename) == false)
                {
                    Serial.printf("File does not exist %s\n", filename);
                    return "";
                }
                
                File file = SPIFFS.open(filename, "r");
                if (!file) 
                {
                    Serial.printf("Could not open file %s\n", filename);
                }

                String line = file.readStringUntil('\n');
                file.close();

                const char * data = line.c_str();
                Serial.printf("Read from %s the following data:\n%s\n", filename, data);

                return data;
            }

            virtual void loop(void)
            {
            }
        protected:
    };
#endif
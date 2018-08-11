#ifndef CONFIGMODULE_H
    #define CONFIGMODULE_H
    #include <Arduino.h>
    #include <ArduinoJson.h>
    #include <FS.h>

    #include "Module.h"
    #include "MQTTModule.h"

    class StorageModule: public Module 
    {
        public:
            StorageModule()
            {
                this->_name = "storage";
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

            void write(const char * filename, const char * module_data)
            {
                File file = SPIFFS.open(filename, "w");
                if (!file) 
                {
                    Serial.printf("Could not open file %s\n", filename);
                }

                //Serial.printf("Writing on %s the following data:\n%s\n", filename, module_data);

                file.println(module_data);
                file.close();
            }

            String read(const char * filename)
            {
                String data;
                File file = SPIFFS.open(filename, "r");
                if(SPIFFS.exists(filename) == false || file == false)
                {
                    Serial.printf("File does not exist %s\n", filename);
                }
                else
                {
                    File file = SPIFFS.open(filename, "r");
                    if (!file) 
                    {
                        Serial.printf("Could not open file %s\n", filename);
                    }
                    else
                    {
                        data = file.readString();
                        file.close();
                    }
                }
                //Serial.printf("Read from %s the following data:\n%s\n", filename, data.c_str());

                return data;
            }
    };
#endif
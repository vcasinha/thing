#ifndef CONFIGMODULE_H
    #define CONFIGMODULE_H
    #include <Arduino.h>
    #include <ArduinoJson.h>
    #include <FS.h>

    #include "Module.h"

    class StorageModule: public Module 
    {
        public:
            StorageModule()
            {
                this->_name = "storage";
                if (SPIFFS.begin() == false)
                {
                    Serial.printf("(Storage) ** Error initializing filesystem");
                }
            }

            bool write(const char * filename, const char * module_data)
            {
                Serial.printf("(Storage) Open file '%s'\n", filename);
                File file = SPIFFS.open(filename, "w");
                if (!file)
                {
                    Serial.printf("(Storage) Could not open '%s'\n", filename);
                    return false;
                }

                //Serial.printf("Writing on %s the following data:\n%s\n", filename, module_data);

                file.println(module_data);
                file.close();

                return true;
            }

            File getFile(const char * filename, const char * mode)
            {
                Serial.printf("(Storage) Get file '%s'\n", filename);
                return SPIFFS.open(filename, mode);
            }

            bool exists(const char * filename)
            {
                if (SPIFFS.exists(filename) == false)
                {
                    Serial.printf("File does not exist %s\n", filename);
                    return false;
                }

                return true;
            }

            String read(const char * filename)
            {
                String data;

                Serial.printf("(Storage) Read file '%s'\n", filename);
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

            Dir opendir(const char * path)
            {
                Serial.printf("(Storage) Open folder '%s'\n", path);
                return SPIFFS.openDir(path);
            }
    };
#endif
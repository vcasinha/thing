#ifndef CONFIGMODULE_H
#define CONFIGMODULE_H
#include <Arduino.h>
#include <ArduinoLog.h>
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
                Log.error("(storage.construct) Error initializing SPIFFS filesystem");
            }
        }

        File getFile(const char *filename, const char *mode)
        {
            Log.trace("(storage.getFile) Get file '%s'", filename);
            return SPIFFS.open(filename, mode);
        }

        bool append(const char * filename, const char * data)
        {
            Log.trace("(storage.write) Open file '%s'", filename);
            File file = SPIFFS.open(filename, "a");
            if (!file)
            {
                Log.error("(storage.write) Could not open '%s'", filename);
                return false;
            }

            //Serial.printf("Writing on %s the following data:\n%s\n", filename, module_data);

            file.println(data);
            file.close();

            return true;
        }

        bool write(const char * filename, const char * module_data)
        {
            Log.trace("(storage.write) Open file '%s'", filename);
            File file = SPIFFS.open(filename, "w");
            if (!file)
            {
                Log.error("(storage.write) Could not open '%s'", filename);
                return false;
            }

            //Serial.printf("Writing on %s the following data:\n%s\n", filename, module_data);

            file.println(module_data);
            file.close();

            return true;
        }

        bool exists(const char * filename)
        {
            if (SPIFFS.exists(filename) == false)
            {
                return false;
            }

            return true;
        }

        bool remove(const char *path)
        {
            return SPIFFS.remove(path);
        }

        String read(const char * filename)
        {
            String data;

            Log.trace("(storage.read) Read file '%s'", filename);
            File file = SPIFFS.open(filename, "r");
            if(SPIFFS.exists(filename) == false || file == false)
            {
                Log.error("(storage.read) File does not exist %s", filename);
            }
            else
            {
                File file = SPIFFS.open(filename, "r");
                if (!file)
                {
                    Log.error("(storage.read) Could not open file %s", filename);
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
            Log.trace("(storage.opendir) Open folder '%s'", path);
            return SPIFFS.openDir(path);
        }

        bool fsInfo(FSInfo & info)
        {
            return SPIFFS.info(info);
        }
};
#endif
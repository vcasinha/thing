#include <Arduino.h>
#include <ArduinoJson.h>

#include "Application.h"
#include "StorageModule.h"
#include "StorageModule.h"
#include "WiFiModule.h"
#include "MQTTModule.h"
#include "DeviceModule.h"
#include "ServerModule.h"
#include "DeviceManagerModule.h"

Application::Application(const char * id)
{
    this->_id = id;
    Serial.printf("\n\n%s booting\n", this->_id);
    uint32_t realSize = ESP.getFlashChipRealSize();
    uint32_t ideSize = ESP.getFlashChipSize();
    FlashMode_t ideMode = ESP.getFlashChipMode();

    Serial.printf("Flash real id: %08X\n", ESP.getFlashChipId());
    Serial.printf("Flash real size: %u bytes\n\n", realSize);
    Serial.printf("Flash ide  size: %u bytes\n", ideSize);
    Serial.printf("Flash ide speed: %u Hz\n", ESP.getFlashChipSpeed());
    Serial.printf("Flash ide mode:  %s\n", (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));

    if (ideSize != realSize) 
    {
        Serial.println("Flash Chip configuration wrong!\n");
    } 
    else 
    {
        Serial.println("Flash Chip configuration ok.\n");
    }

    delay(1000);
    Serial.printf("Load system modules\n");
    this->loadModule(new DeviceModule());
    this->loadModule(new WiFiModule());
    this->loadModule(new MQTTModule());
    this->loadModule(new StorageModule());
    this->loadModule(new ServerModule());
    this->loadModule(new DeviceManagerModule());
}

void Application::loadModule(Module * module)
{
    Serial.printf("Load module %s\n", module->_name);
    module->setApplication(this);
    this->_modules.push(module);
}

Module * Application::getModule(const char * name)
{
    //Serial.printf("Get module %s\n", name);
    for(unsigned int c = 0;c < this->_modules.size();c++)
    {
        Module * m = this->_modules[c];
        if(strcmp(m->_name, name) == 0)
        {
            return m;
        }
    }
    Serial.printf("\n\n *** Module not found '%s' *** \n\n", name);
    abort();
}

void Application::setup(void)
{
    DynamicJsonBuffer buffer;
    char filename[100];
    File file;
    String config_json, default_config = "{\"mqtt\":{\"hostname\":\"petitmaison.duckdns.org\",\"username\":\"mqtt\",\"password\":\"mqtt\",\"root_topic\":\"home\"}}";

    StorageModule * storage = (StorageModule *) this->getModule("storage");

    sprintf(filename, "/%s_configuration.json", this->_id);
    config_json = storage->read(filename);
    
    if(config_json.equals(""))
    {
        Serial.printf("Using default configuration\n");
        config_json = default_config;
    }
    //Serial.printf("Parsing JSON configuration. \n%s\n", config_json.c_str());
    JsonObject & modules_configuration = buffer.parseObject(config_json.c_str());
    if (!modules_configuration.success())
    {
        Serial.printf("Failed to parse JSON configuration. \n%s\n", config_json.c_str());
        abort();
    }

    Serial.printf("Boot modules...\n");
    for(unsigned int i = 0;i < this->_modules.size();i++)
    {
        //Module * module = this->getModule(name);
        Module * module = this->_modules[i];
        if(module == NULL)
        {
            continue;
        }

        Serial.printf("Boot module %s\n", module->_name);

        JsonObject &module_config = modules_configuration[module->_name];
        bool disabled = module_config["disable"] | false;

        module->_loop_period_ms = module_config["update_period"] | module->_loop_period_ms;

        if (disabled == false)
        {
            module_config.prettyPrintTo(Serial);
            Serial.printf("\n");
            
            //Boot module
            module->boot(module_config);
        }
        else
        {
            Serial.printf("* Module '%s' disabled\n", module->_name);
            module->_enabled = false;
        }
    }

    config_json = "";
    modules_configuration.printTo(config_json);
    storage->write(filename, config_json.c_str());
    Serial.printf("Setup modules...\n");
    for(unsigned int c = 0;c < this->_modules.size();c++)
    {
        Module * module = this->_modules[c];
        if(module->_enabled == true)
        {
            Serial.printf("* Setting up module '%s' * \n", module->_name);
            module->setup();
            delay(100);
        }
    }
    Serial.printf("Modules initialized\n");
}

void Application::loop(void)
{
    for(unsigned int c = 0;c < this->_modules.size();c++)
    {
        Module * module = (Module *) this->_modules[c];
        
        if(module->_enabled == true)
        {
            unsigned long current_time = millis() + module->_loop_period_ms;

            if (module->_loop_period_ms == 0 || (current_time - module->_loop_time) > module->_loop_period_ms)
            {
                module->loop();
                delay(10);
                module->_loop_time = current_time;
            }
        }
    }
}
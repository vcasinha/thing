#include <Arduino.h>
#include <ArduinoLog.h>
#include <ArduinoJson.h>

#include "Application.h"
#include "MQTTModule.h"
#include "DeviceModule.h"
#include "StorageModule.h"
#include "RFModule.h"
#include "PersWiFiManager.h"
#include "ServerModule.h"
#include "TimeModule.h"
#include "UnitManagerModule.h"

Application::Application(const char * id)
{
    this->_id = id;
    Log.notice("(application.construct) Booting application '%s' ", this->_id);

    Log.notice("(application.construct) Flash ID %08X", ESP.getFlashChipId());
    uint32_t realSize = ESP.getFlashChipRealSize();
    Log.notice("(application.construct) Flash size %l bytes", realSize);
    uint32_t ideSize = ESP.getFlashChipSize();
    Log.notice("(application.construct) Flash IDE size %l bytes", ideSize);
    Log.notice("(application.construct) Flash IDE speed %lHz", ESP.getFlashChipSpeed());
    Log.notice("(application.construct) CPU Frequency %lMHz", ESP.getCpuFreqMHz());
    FlashMode_t ideMode = ESP.getFlashChipMode();
    Log.notice("(application.construct) Flash IDE mode %s", (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));
    delay(3000);
    if (ideSize != realSize)
    {
        Log.notice("(application.construct) Flash Chip configuration wrong! ");
    }
    else
    {
        Log.notice("(application.construct) Flash Chip configuration ok. ");
    }

    Log.notice("(application.construct) Load application modules ");

    this->loadModule(new DeviceModule());
    this->loadModule(new StorageModule());
    this->loadModule(new WiFiModule());
    this->loadModule(new TimeModule());
    this->loadModule(new ServerModule());
    this->loadModule(new MQTTModule());
    this->loadModule(new RFModule());
    this->loadModule(new UnitManagerModule());
}

void Application::loadModule(Module * module)
{
    Log.notice("(application.loadModule) Loading %s ", module->_name);
    module->setApplication(this);
    this->_modules.push(module);
}

Module * Application::getModule(const char * name)
{
    //Log.notice("Get module %s ", name);
    for(unsigned int c = 0;c < this->_modules.size();c++)
    {
        Module * module = this->_modules[c];
        if (strcmp(module->_name, name) == 0)
        {
            return module;
        }
    }

    Log.fatal("(application.getModule) Module not found '%s'", name);
    Log.fatal("(application.getModule) Abort");
    abort();
}

void Application::setup(void)
{
    DynamicJsonDocument JSONDoc(2048);
    char filename[100];
    File file;
    String config_json, default_config = "{\"mqtt\":{\"hostname\":\"petitmaison.duckdns.org\",\"username\":\"mqtt\",\"password\":\"mqtt\",\"root_topic\":\"home\"}}";

    Log.notice("(application.setup) Setup start ");

    StorageModule * storage = (StorageModule *) this->getModule("storage");

    sprintf(filename, "/configuration.json");
    Log.notice("(application.setup) Reading configuration file %s ", filename);
    config_json = storage->read(filename);

    //Log.notice("(application.) Configuration: %s ", config_json.c_str());

    if(config_json.equals(""))
    {
        Log.warning("(application.setup) Using default configuration ");
        config_json = default_config;
    }

    auto error = deserializeJson(JSONDoc, config_json);

    if (error)
    {
        Log.error("(application.setup) Failed to parse JSON configuration.  %s ", config_json.c_str());
        Log.error(error.c_str());
        abort();
    }

    //Log.notice("(application.setup)### Configuration debug: ");
    //serializeJson(JSONDoc, Serial);
    //Log.notice("(application.setup)### END DEBUG ");

    //JsonObject modules_configuration = JSONDoc.to<JsonObject>();

    Log.notice("(application.setup) Boot modules... ");
    for(unsigned int i = 0;i < this->_modules.size();i++)
    {
        //Module * module = this->getModule(name);
        Module * module = this->_modules[i];
        bool disabled = false;
        if(module == NULL)
        {
            continue;
        }

        Log.trace("(application.setup) Boot module %s ", module->_name);

        if (JSONDoc.containsKey(module->_name) == false)
        {
            Log.warning("(application.setup) * Module '%s' configuration not found ", module->_name);
        }

        JsonVariant module_config = JSONDoc[module->_name];
        if(module_config.isNull())
        {
            Log.warning("(application.setup) * Module '%s' configuration is empty ", module->_name);
        }
        else
        {
            disabled = module_config["disable"] | false;
            module->_loop_period_ms = module_config["loop_period_ms"] | module->_loop_period_ms;
        }

        if (disabled == false)
        {
            String json_config = "";
            serializeJson(module_config.as<JsonObject>(), json_config);
            Log.notice("(application.setup) * Booting module '%s' with (%s)", module->_name, json_config.c_str());
            JsonObject json = module_config.as<JsonObject>();
            module->boot(json);
        }
        else
        {
            Log.warning("(application.setup) * Module '%s' disabled ", module->_name);
            module->_enabled = false;
        }
    }

    Log.notice("(application.setup) Initialize modules... ");
    for(unsigned int c = 0;c < this->_modules.size();c++)
    {
        Module * module = this->_modules[c];
        if(module->_enabled == true)
        {
            Log.notice("(application.setup) * Setting up module '%s' *  ", module->_name);
            module->setup();
            module->ready();
            delay(100);
        }
        else
        {
            Log.warning("(application.setup) * Module disabled '%s' *  ", module->_name);
        }
    }
    String ip_address = WiFi.localIP().toString();
    DeviceModule *device = (DeviceModule *)this->getModule("device");

    Log.notice("(application.setup) #################################################################################");
    Log.notice("(application.setup) ### Application ready and running");

    if(WiFi.getMode() == WIFI_STA)
    {
        Log.notice("(application.setup) ### Connected to SSID %s", WiFi.SSID().c_str());
    }
    else
    {
        Log.notice("(application.setup) ### Started %s AP named %s", device->_secure ? "secure" : "public", device->_hostname.c_str());
    }

    Log.notice("(application.setup) ### HTTP on http://%s.local (%s)", device->_hostname.c_str(), ip_address.c_str());
    Log.notice("(application.setup) ### WiFi on http://%s.local/hotspot-detect.html", device->_hostname.c_str());
    Log.notice("(application.setup) ###");
    Log.notice("(application.setup) #################################################################################");
}


#include <UnitManagerModule.h>



UnitManagerModule::UnitManagerModule()
{
    this->init("unit_manager", 100);
}

void UnitManagerModule::makeUnit(String unitID, JsonObject config)
{
    String unit_type = config["type"].as<String>();
    //String json_config = "";
    //serializeJson(config, json_config);
    //Log.notice("(unitManager.makeDevice) Init '%s' device (%s)", unit_type.c_str(), json_config.c_str());

    UnitFactory *factory = UnitManagerModule::getFactory(unit_type);
    if(factory)
    {
        Unit * unit = factory->make();
        unit->boot(unitID, config, this->_application);
        this->_units.push(unit);
    }
    else
    {
        Log.error("(unitManager.makeDevice) ERROR Unknown device type '%s'.", unit_type.c_str());
    }
}

void UnitManagerModule::config(JsonObject &config)
{
    Log.notice("(UnitManager.config) Set units configuration");
    serializeJson(config, Serial);
    if (config.containsKey("units"))
    {
        for (unsigned int i = 0; i < this->_units.size(); i++)
        {
            Unit *unit = this->_units[i];
            if (config["units"][unit->_id])
            {
                JsonObject unit_config = config["units"][unit->_id];
                String json_config = "";
                serializeJson(unit_config, json_config);
                Log.notice("(unitsManager.config) Update unit configuration '%s' ()", unit->_id.c_str(), json_config.c_str());
                this->_units[i]->unitConfig(unit_config);
            }
        }
    }
}

void UnitManagerModule::boot(JsonObject & config)
{
    if (config.size() == 0)
    {
        Log.warning("(MQTT) Empty configuration, disabling Device Manager");
        this->disable();
        return;
    }

    this->_mqtt = (MQTTModule *)this->_application->getModule("mqtt");
    this->_mqtt->registerCallback(this);

    this->_time = (TimeModule *)this->_application->getModule("time");
    this->_server = (ServerModule * )this->_application->getModule("server");

    if (config.containsKey("units"))
    {
        JsonObject root = config["units"].as<JsonObject>();
        Log.notice("(unitsManager.boot) Booting %d units", config["units"].size());
        for (JsonObject::iterator it = root.begin(); it != root.end(); ++it)
        {
            //config.prettyPrintTo(Serial);
            this->makeUnit(it->key().c_str(), it->value().as<JsonObject>());
        }
    }
}

void UnitManagerModule::setup(void)
{
    this->_server->_webServer->on("/unit", HTTP_GET, [&]() {
        if (!this->_server->isAuthenticated())
        {
            return;
        }

        const char *content_type = "application/json";

        if (!this->_server->_webServer->hasArg("id"))
        {
            Log.error("(web) 'id' is required");
            this->_server->_webServer->send(500, content_type, "{\"message\":\"'id' is required\"}");
            return;
        }

        String unitID = this->_server->_webServer->arg("id");
        Unit *u = this->getUnitByID(unitID);

        if (!u)
        {
            this->_server->_webServer->send(500, content_type, "{\"message\":\"Invalid unit ID\"}");
            return;
        }

        DynamicJsonDocument buffer(1024);
        JsonObject o = buffer.to<JsonObject>();
        u->getStatus(o);
        String json;
        serializeJson(buffer, json);
        this->_server->_webServer->send(200, content_type, json.c_str());
    });

    this->_server->_webServer->on("/unit", HTTP_POST, [&]() {
        if (!this->_server->isAuthenticated())
        {
            return;
        }

        const char *content_type = "application/json";

        if (!this->_server->_webServer->hasArg("id"))
        {
            Log.error("(web) 'id' is required");
            this->_server->_webServer->send(500, content_type, "{\"message\":\"'id' is required\"}");
            return;
        }

            if (!this->_server->_webServer->hasArg("plain"))
            {
                this->_server->_webServer->send(500, "application/json", "{\"message\":\"Body missing\"}");
                return;
            }

        String body = this->_server->_webServer->arg("plain");
        DynamicJsonDocument JSONDoc(1024);
        auto error = deserializeJson(JSONDoc, body);
        JsonObject json = JSONDoc.as<JsonObject>();

        String unitID = this->_server->_webServer->arg("id");
        Unit *unit = this->getUnitByID(unitID);

        if(unit)
        {
            unit->config(json);
        }
        else
        {
            this->makeUnit(unitID, json);
        }

        this->_server->_webServer->send(200, content_type, "{}");
    });

    //Log.notice("Setup devices");
    for (unsigned int i = 0; i < this->_units.size(); i++)
    {
        Unit *unit = this->_units[i];
        unit->ready();
    }

        //Log.notice("Setup devices");
    for (unsigned int i = 0; i < this->_units.size(); i++)
    {
        Unit *unit = this->_units[i];
        unit->setMQTT(this->_mqtt);
        unit->ready();
    }
}

void UnitManagerModule::callback(char *topic, unsigned char *payload, unsigned int length)
{
    String topic_string = topic;
    payload[length] = '\0';
    String data = (char *)payload;
    data.trim();

    Log.trace("(unitsManager.callback) Unit callback on topic '%s' Data: %s", topic_string.c_str(), data.c_str());
    for (unsigned int i = 0; i < this->_units.size(); i++)
    {
        this->_units[i]->callback(topic_string, data);
    }
}

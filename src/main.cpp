#include <Arduino.h>
#include <ArduinoLog.h>

#include "Application.h"
#include "RFModule.h"
#include "DHTModule.h"
#include "StorageModule.h"
#include "WiFiModule.h"
#include "MQTTModule.h"
#include "DeviceModule.h"
#include "ServerModule.h"
#include "SwitchModule.h"
#include "user_settings.h"
#include "ServerModule.h"

Application * app;

void setup() 
{
    Serial.begin(serial_port_baud_rate);
    //ESP.reset();
    //Serial.setDebugOutput(true);
    delay(1000);
    app = new Application(name);
    Serial.printf("Application boot\n");
    delay(1000);
    Serial.printf("Load modules\n");
    app->loadModule(new DeviceModule());
    app->loadModule(new WiFiModule());
    app->loadModule(new MQTTModule());
    app->loadModule(new StorageModule());
    app->loadModule(new ServerModule());
    //app->loadModule(new DHTModule());
    //app->loadModule(new RFModule());
    app->loadModule(new SwitchModule());

    delay(1000);
    app->setup();
    delay(1000);
    Serial.printf("Application ready\n");
}

void loop(void)
{
    app->loop();
    delay(10);
}
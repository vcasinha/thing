#include <Arduino.h>
#include <ArduinoLog.h>

#include "Application.h"
#include "DeviceManagerModule.h"
#include "DHTFactory.h"
#include "SwitchFactory.h"
//#include "RFModule.h"
#include "user_settings.h"

Application * app;

void setup()
{
    Serial.begin(serial_port_baud_rate);
    //ESP.reset();
    //Serial.setDebugOutput(true);
    delay(5000);
    Serial.printf("\n\n%s v%s\n\n", application_name, application_version);
    app = new Application(application_name);
    delay(1000);
    //Serial.printf("Load modules\n");
    //app->loadModule(new DHTLoader());
    //app->loadModule(new RFModule());
    //app->loadModule(new SwitchModule());
    app->setup();
}

void loop(void)
{
    app->loop();
    delay(10);
}
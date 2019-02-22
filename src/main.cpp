#include <Arduino.h>
#include <ArduinoLog.h>

#include "Application.h"
#include "DeviceManagerModule.h"
#include "DHTFactory.h"
#include "SwitchFactory.h"
//#include "RFModule.h"
#include "user_settings.h"

Application *app;

void setup()
{
    Serial.begin(serial_port_baud_rate);
    //ESP.reset();
    //Serial.setDebugOutput(true);
    delay(1000);
    Serial.printf("\n\n%s v%s\n", NAME, FW_VERSION);
    app = new Application("robot");
    delay(1000);
    Serial.printf("Load modules\n");
    //app->loadModule(new DHTLoader());
    //app->loadModule(new RFModule());
    //app->loadModule(new SwitchModule());

    delay(1000);
    app->setup();
    Serial.printf("Application ready\n");
}

void loop(void)
{
    app->loop();
    delay(10);
}
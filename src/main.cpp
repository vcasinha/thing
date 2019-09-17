#include <Arduino.h>
#include <ArduinoLog.h>

#include "Application.h"
#include "DHTFactory.h"
#include "SwitchFactory.h"
#include "UnitManagerModule.h"
#include "user_settings.h"

Application * app;

void printTimestamp(Print *_logOutput)
{
    char c[12];
    sprintf(c, "%10lu ", millis());
    _logOutput->print(c);
}

void printNewline(Print *_logOutput)
{
    _logOutput->print('\n');
}

void setup()
{
    //Initialize serial port output
    Serial.begin(serial_port_baud_rate);
    Serial.print("\n\n");
    //Log configuration
    Log.begin(APP_LOG_LEVEL, &Serial, true);
    Log.setPrefix(printTimestamp); // Uncomment to get timestamps as prefix
    Log.setSuffix(printNewline); // Uncomment to get newline as suffix
    //delay(5000);
    delay(1000);
    Log.notice("Booting %s v%s", application_name, application_version);

    delay(4000);
    app = new Application(application_name);
    UnitManagerModule * manager = (UnitManagerModule *) app->getModule("unit_manager");
    manager->addFactory(new DHTFactory());
    manager->addFactory(new SwitchFactory());
    app->setup();
}

void loop(void)
{
    app->loop();
    delay(10);
}

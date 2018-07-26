#include <Arduino.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

#include "Application.h"
#include "MQTTModule.h"
#include "WiFiModule.h"

#include "user_settings.h"

Application app;
WiFiModule wifi;
MQTTModule mqtt;
WiFiServer server(80);

void setup() 
{
    Serial.printf("Application start\n");
    delay(1000);
    Serial.printf("Load modules\n");
    app.loadModule(&wifi);
    app.loadModule(&mqtt);
    delay(1000);
    Serial.printf("Modules boot initiated\n");
    app.setup();
    delay(1000);
    mqtt._client.publish("test", "something");
    Serial.printf("Modules booted\n");
    delay(1000);
}

void loop() {
    app.loop();
}
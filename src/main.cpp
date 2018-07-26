#include <Arduino.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

#include "Application.h"
#include "MQTTModule.h"
#include "WiFiModule.h"
#include "M1.h"

#include "user_settings.h"

Application app = Application(serial_port_baud_rate);
WiFiModule wifi;
MQTTModule mqtt = MQTTModule(mqtt_hostname, mqtt_username, mqtt_password);
M1 m1;

void callback(char * topic, unsigned char * payload, unsigned int length)
{
    Serial.printf("Me too\n");
}

void setup() 
{
    Serial.printf("Application start\n");
    delay(1000);
    Serial.printf("Load modules\n");
    app.loadModule(&wifi);
    app.loadModule(&mqtt);
    app.loadModule(&m1);
    delay(1000);
    app.setup();
    delay(1000);
}

void loop() {
    app.loop();
    delay(100);
}
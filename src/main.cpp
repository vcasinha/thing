#include <Arduino.h>

#include "Application.h"
#include "MQTTModule.h"
#include "WiFiModule.h"
#include "RFModule.h"
#include "DHTModule.h"
#include "ConfigModule.h"

#include "user_settings.h"

Application app = Application("remembot", serial_port_baud_rate);
WiFiModule wifi;
MQTTModule mqtt = MQTTModule(mqtt_hostname, mqtt_username, mqtt_password);
RFModule rf = RFModule(D5, D6);
DHTModule dht = DHTModule(D4);
ConfigModule config;
void callback(char * topic, unsigned char * payload, unsigned int length)
{
    Serial.printf("I'm outside and I've heard '%s'\n", topic);
}

void setup() 
{
    Serial.printf("Application boot\n");
    delay(1000);
    Serial.printf("Load modules\n");
    app.loadModule(&wifi);
    app.loadModule(&mqtt);
    app.loadModule(&rf);
    app.loadModule(&dht);
    app.loadModule(&config);
    delay(1000);
    app.setup();
    delay(1000);
    Serial.printf("Application ready\n");
}

void loop(void)
{
    app.loop();
    delay(10);
}
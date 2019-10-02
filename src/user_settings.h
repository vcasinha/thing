#ifndef USER_SETTINGS
#define DEBUG_ESP_MDNS
#define DEBUG_ESP_MDNS_RX
#define DEBUG_ESP_MDNS_TX
#define USER_SETTINGS
#define APP_LOG_LEVEL LOG_LEVEL_VERBOSE
#define WIFI_CONNECT_TIMEOUT 30
#define DEBUG_ESP_PORT Serial
#define RFMODULE_SUPPORT

//#define DISABLE_LOGGING

const char *application_name = "Units";
const char *application_version = "1.0rc";
unsigned int serial_port_baud_rate = 115200;

#endif
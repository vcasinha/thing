#ifndef APPLICATION_H
#define APPLICATION_H

    #include <Arduino.h>
    #include <PubSubClient.h>
    #include "Vector.h"

    #include "Module.h"

    class Module;
    class Application
    {
        public:
            PubSubClient _client;
            const char * _id;
            
            Application(const char * id, unsigned int baud_rate)
            {
                Serial.begin(baud_rate);
                //Serial.setDebugOutput(true);
                delay(1000);
                _id = id;
                Serial.printf("\n\n%s booting\n", _id);
                uint32_t realSize = ESP.getFlashChipRealSize();
                uint32_t ideSize = ESP.getFlashChipSize();
                FlashMode_t ideMode = ESP.getFlashChipMode();

                Serial.printf("    Flash real id:   %08X\n", ESP.getFlashChipId());
                Serial.printf("    Flash real size: %u bytes\n\n", realSize);
                Serial.printf("    Flash ide  size: %u bytes\n", ideSize);
                Serial.printf("    Flash ide speed: %u Hz\n", ESP.getFlashChipSpeed());
                Serial.printf("    Flash ide mode:  %s\n", (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));

                if (ideSize != realSize) {
                    Serial.println("    Flash Chip configuration wrong!\n");
                } else {
                    Serial.println("    Flash Chip configuration ok.\n");
                }

                delay(5000);
            }

            void loadModule(Module * module);
            Module * getModule(const char * name);
            void setup(void);
            void loop(void);
        private:
            Vector<const char *> _modulesNames;
            Vector<Module *> _modules;
    };

#endif
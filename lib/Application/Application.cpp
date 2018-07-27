#include "Application.h"
#include "ArduinoJson.h"

void Application::loadModule(Module * module)
{
    module->setApplication(this);
    _modules.push(module);
    _modulesNames.push(module->_name);
}

Module * Application::getModule(const char * name)
{
    //Serial.printf("Get module %s\n", name);
    delay(10);
    for(unsigned int c = 0;c < this->_modules.size();c++)
    {
        Module * m = this->_modules[c];
        if(strcmp(m->_name, name) == 0)
        {
            return m;
        }
    }
    Serial.printf("\n\n *** Module not found '%s' *** \n\n", name);
    abort();
}

void Application::setup(void)
{
    Serial.printf("Initializing %d modules\n", this->_modules.size());
    for(unsigned int c = 0;c < this->_modules.size();c++)
    {
        StaticJsonBuffer<100> buffer;
        JsonObject & config = buffer.createObject();
        if (!config.success())
        {
            Serial.println("Failed to parse JSON configuration.\n");
        }
        
        Serial.printf("\n    * Booting module '%s' * \n", this->_modules[c]->_name);
        this->_modules[c]->boot(config);
        delay(100);
    }
    

    for(unsigned int c = 0;c<this->_modules.size();c++)
    {
        Serial.printf("\n    * Setting up module '%s' * \n", this->_modules[c]->_name);
        this->_modules[c]->setup();
        delay(100);
    }
    Serial.printf("Modules initialized\n");
}

void Application::loop(void)
{
    for(unsigned int c = 0;c < this->_modules.size();c++)
    {
        Module * m =(Module *) this->_modules[c];

        unsigned long current_time = millis() + m->_update_period;
        
        if((current_time - m->_last_update) > m->_update_period)
        {
            m->loop();
            delay(10);
            m->_last_update = current_time;
        }
    }
}
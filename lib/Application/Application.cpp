#include "Application.h"

Application::Application()
{
    Serial.begin(SERIAL_MONITOR_BAUD);
}

void Application::loadModule(Module * module)
{
    module->setApplication(this);
    _modules.push(module);
    _modulesNames.push(module->_name);
}

Module * Application::getModule(const char * name)
{
    Serial.printf("Get module %s\n", name);
    delay(10);
    for(unsigned int c = 0;c < this->_modules.size();c++)
    {
        Serial.printf("Iterating module %d\n", c);
        Module * m = this->_modules[c];
        if(strcmp(m->_name, name) == 0)
        {
            Serial.printf("Found module %s\n", m->_name);
            return m;
        }
    }
    Serial.printf("\n\n *** Module not found '%s' *** \n\n", name);
    abort();
}

void Application::setup(void)
{
    for(unsigned int c = 0;c < this->_modules.size();c++)
    {
        this->_modules[c]->boot();
    }

    for(unsigned int c = 0;c<this->_modules.size();c++)
    {
        this->_modules[c]->setup();
    }
}

void Application::loop(void)
{
    for(unsigned int c = 0;c < this->_modules.size();c++)
    {
        this->_modules[c]->loop();
        delay(10);
    }
    delay(100);
}
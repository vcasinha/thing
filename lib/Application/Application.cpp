#include "Application.h"

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
        Serial.printf("\n    * Booting module '%s' * \n", this->_modules[c]->_name);
        this->_modules[c]->boot();
    }
    delay(100);

    for(unsigned int c = 0;c<this->_modules.size();c++)
    {
        Serial.printf("\n    * Setting up module '%s' * \n", this->_modules[c]->_name);
        this->_modules[c]->setup();
    }
    Serial.printf("Modules initialized\n");
}

void Application::loop(void)
{
    for(unsigned int c = 0;c < this->_modules.size();c++)
    {
        this->_modules[c]->loop();
        delay(10);
    }
    
}
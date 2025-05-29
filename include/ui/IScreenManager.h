#ifndef __ISCREEN_MANAGER_H
#define __ISCREEN_MANAGER_H

#include <Arduino.h>

class IScreenManager {

  public:
    virtual bool switchToScreen(const String &screenName, void *params = nullptr) = 0;

    virtual bool goBack() = 0;
};

#endif // __ISCREEN_MANAGER_H

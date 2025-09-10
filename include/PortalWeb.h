#ifndef PORTAL_WEB_H
#define PORTAL_WEB_H

#include <ESPAsyncWebServer.h>
#include "PositioningManager.h"

class PortalWeb {
public:
    PortalWeb(const char* ssid, const char* password);
    void begin(String mac, PositioningManager& manager);

private:
    AsyncWebServer _server;
    PositioningManager* _manager;
    const char* _ssid;
    const char* _password;
};

#endif // PORTAL_WEB_H

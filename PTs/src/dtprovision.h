// Library #############################################################
// Autoprovisioning of Digital Twins ###################################
// Masterthesis V 0.1.0 ################################################
// by Joel Lehmann #####################################################
// 12.07.2021 ##########################################################

#ifndef DT
#define DT

#include <Arduino.h>
#include <ESP8266HTTPClient.h>

class DigitalTwin
{
    private:

    WiFiClient* _client;
    int ms;
    char devRegURL[64];
    char dittoURL[64];
    char srv[128];
    char honoTenant[16];
    char honoDevice[16];
    char honoNamespace[16];
    char dittoUser[16];
    char dittoPass[16];

    public: 

    DigitalTwin();
    bool init (WiFiClient &client, const char * ip, const char * devReg, const char * ditto, int zeit);
    int createHonoTenant (const char * tenant);
    int createHonoDevice (const char * ns, const char * device);
    int createHonoCredentials (const char * pass);
    int createDittoPiggyback (const char * user, const char * pass, char * json);
    int createDittoPolicy (const char * user, const char * pass, char * json);
    int createDittoThing (char * json);
    int createDittoFeatures (char * json);
};

#endif

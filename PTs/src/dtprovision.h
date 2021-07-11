// Library #############################################################
// Autoprovisioning of Digital Twins ###################################
// Masterthesis ########################################################
// by Joel Lehmann #####################################################
// 08.07.2021 ##########################################################

#ifndef DT
#define DT

#include <Arduino.h>
#include <ESP8266HTTPClient.h>

class DigitalTwin
{
    private:

    WiFiClient* _client;
    String serverIP;
    int telemetryPort;
    int devRegPort;
    int dittoPort;
    int ms;
    String telemetryURL;
    String devRegURL;
    String dittoURL;
    String honoTenant;
    String honoDevice;
    String honoNamespace;
    String honoDevicePassword;
    String nodeRedToken;

    public: 

    DigitalTwin();
    bool init (WiFiClient &client, String ip, int telemetry, int devReg, int ditto, int zeit);
    int createHonoTenant (String tenant);
    int createHonoDevice (String ns, String device);
    int createHonoCredentials (String pass);
    int createDittoPiggyback (char * json);
    int createDittoPolicy (char * json);
    int createDittoThing (char * json);
    int createDittoFeatures (char * json);
    int createNodeRedDashboard (String json);
};

#endif

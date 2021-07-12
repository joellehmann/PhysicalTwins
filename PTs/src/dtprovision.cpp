// Library #############################################################
// Autoprovisioning of Digital Twins ###################################
// Masterthesis V 0.1.0 ################################################
// by Joel Lehmann #####################################################
// 12.07.2021 ##########################################################

#include "dtprovision.h"
#include <Arduino.h>
#include <ESP8266HTTPClient.h>

DigitalTwin::DigitalTwin()
{

}

// Maybe including Credentials 
bool DigitalTwin::init (WiFiClient &client, const char * ip, const char * devReg, const char * ditto, int zeit)
{
    _client = &client;
    ms = zeit;
    char tmp[32];

    strcpy(devRegURL, ip);
    strcat(devRegURL,":");
    strcat(devRegURL,devReg);

    strcpy(dittoURL, ip);
    strcat(dittoURL,":");
    strcat(dittoURL,ditto);

    return true;
}

int DigitalTwin::createHonoTenant (const char * tenant)
{
    HTTPClient http;
    strcat(honoTenant,tenant);
    
    strcpy(srv, devRegURL);
    strcat(srv,"/v1/tenants/");
    strcat(srv,honoTenant);

    http.begin(*_client,srv);

    int response = http.POST("");
    Serial.println();
    Serial.println("###   Hono Tenant Provision   ######################################"); 
    Serial.print("POST to Server: ");  
    Serial.println(srv);  
    Serial.print("HTTP Response code: ");
    Serial.println(response);

    http.end();
    memset(srv, NULL, 128);
    delay(ms);
    return response;
}

int DigitalTwin::createHonoDevice (const char * ns, const char * device)
{
    HTTPClient http;
    strcpy(honoNamespace, ns);
    strcpy(honoDevice, device);

    strcpy(srv, devRegURL);
    strcat(srv,"/v1/devices/");
    strcat(srv,honoTenant);
    strcat(srv,"/");
    strcat(srv,honoNamespace);
    strcat(srv,":");
    strcat(srv,honoDevice);

    http.begin(*_client,srv);

    int response = http.POST("");
    Serial.println();
    Serial.println("###   Hono Device Provision   ######################################");  
    Serial.print("POST to Server: ");  
    Serial.println(srv); 
    Serial.print("HTTP Response code: ");
    Serial.println(response);

    http.end();
    memset(srv, NULL, 128);
    delay(ms);
    return response;
}

int DigitalTwin::createHonoCredentials (const char * pass)
{
    HTTPClient http;

    strcpy(srv, devRegURL);
    strcat(srv,"/v1/credentials/");
    strcat(srv,honoTenant);
    strcat(srv,"/");
    strcat(srv,honoNamespace);
    strcat(srv,":");
    strcat(srv,honoDevice);

    http.begin(*_client,srv);
    http.addHeader("Content-Type", "application/json");

    char json[150];
    strcpy(json,"[{\"type\":\"hashed-password\",\"auth-id\":\"");
    strcat(json,honoDevice);
    strcat(json,"\",\"secrets\":[{\"pwd-plain\":\"");
    strcat(json,pass);
    strcat(json,"\"}]}]");
    
    int response = http.PUT(json);
    Serial.println();
    Serial.println("###   Hono Credential Provision   ##################################"); 
    Serial.print("PUT to Server: ");  
    Serial.println(srv);  
    Serial.print("HTTP Response code: ");
    Serial.println(response);

    http.end();
    memset(json, NULL, 150);
    memset(srv, NULL, 128);
    delay(ms);
    return response;
}

int DigitalTwin::createDittoPiggyback (const char * user, const char * pass, char * json)
{
    HTTPClient http;

    strcpy(srv, dittoURL);
    strcat(srv,"/devops/piggyback/connectivity");

    http.begin(*_client,srv);
    http.addHeader("Content-Type", "application/json");
    http.setAuthorization(user, pass);

    int response = http.POST(json);
    Serial.println();
    Serial.println("###   Ditto Piggyback Provision   ##################################"); 
    Serial.print("POST to Server: ");  
    Serial.println(srv);  
    Serial.print("HTTP Response code: ");
    Serial.println(response);

    http.end();
    memset(srv, NULL, 128);
    delay(ms);
    return response;
}

int DigitalTwin::createDittoPolicy (const char * user, const char * pass, char * json)
{
    HTTPClient http;

    strcpy(dittoUser, user);
    strcpy(dittoPass, pass);

    strcpy(srv, dittoURL);
    strcat(srv,"/api/2/policies/");
    strcat(srv,honoNamespace);
    strcat(srv,":");
    strcat(srv,honoDevice);

    http.begin(*_client,srv);
    http.addHeader("Content-Type", "application/json");
    http.setAuthorization(dittoUser, dittoPass);

    int response = http.PUT(json);
    Serial.println();
    Serial.println("###   Ditto Policy Provision   #####################################"); 
    Serial.print("PUT to Server: ");  
    Serial.println(srv);  
    Serial.print("HTTP Response code: ");
    Serial.println(response);

    http.end();
    memset(srv, NULL, 128);
    delay(ms);
    return response;
}

int DigitalTwin::createDittoThing (char * json)
{
    HTTPClient http;

    strcpy(srv, dittoURL);
    strcat(srv,"/api/2/things/");
    strcat(srv,honoNamespace);
    strcat(srv,":");
    strcat(srv,honoDevice);

    http.begin(*_client,srv);
    http.addHeader("Content-Type", "application/json");
    http.setAuthorization(dittoUser, dittoPass);

    int response = http.PUT(json);
    Serial.println();
    Serial.println("###   Ditto Thing Provision   ######################################");
    Serial.print("PUT to Server: ");  
    Serial.println(srv);  
    Serial.print("HTTP Response code: ");
    Serial.println(response);

    http.end();
    memset(srv, NULL, 128);
    delay(ms);
    return response;
}

int DigitalTwin::createDittoFeatures (char * json)
{
    HTTPClient http;

    strcpy(srv, dittoURL);
    strcat(srv,"/api/2/things/");
    strcat(srv,honoNamespace);
    strcat(srv,":");
    strcat(srv,honoDevice);
    strcat(srv,"/features");

    http.begin(*_client,srv);
    http.addHeader("Content-Type", "application/json");
    http.setAuthorization(dittoUser, dittoPass);

    int response = http.PUT(json);
    Serial.println();
    Serial.println("###   Ditto Features Provision   ###################################");
    Serial.print("PUT to Server: ");  
    Serial.println(srv);  
    Serial.print("HTTP Response code: ");
    Serial.println(response);

    http.end();
    memset(srv, NULL, 128);
    memset(devRegURL, NULL,64);
    memset(dittoURL, NULL, 64);
    memset(honoTenant, NULL, 16);
    memset(honoDevice, NULL, 16);
    memset(honoNamespace, NULL, 16);
    memset(dittoUser, NULL, 16);
    memset(dittoPass, NULL, 16);
    delay(ms);
    return response;
}




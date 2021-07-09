// Library #############################################################
// Autoprovisioning of Digital Twins ###################################
// Masterthesis ########################################################
// by Joel Lehmann #####################################################
// 08.07.2021 ##########################################################

#include "dtprovision.h"
#include <Arduino.h>
#include <ESP8266HTTPClient.h>

DigitalTwin::DigitalTwin()
{

}

// Maybe including Credentials 
bool DigitalTwin::init (WiFiClient &client, String ip, int telemetry, int devReg, int ditto, int zeit)
{
    _client = &client;
    serverIP = ip;
    telemetryPort = telemetry;
    devRegPort = devReg;
    dittoPort = ditto;
    ms = zeit;

    telemetryURL = ip + ":" + telemetryPort;
    devRegURL = ip + ":" + devRegPort;
    dittoURL = ip + ":" + dittoPort;

    return true;
}

int DigitalTwin::createHonoTenant (String tenant)
{
    HTTPClient http;
    honoTenant = tenant;
    String srv = devRegURL + "/v1/tenants/" + honoTenant;
    http.begin(*_client,srv);

    int response = http.POST("");
    Serial.println();
    Serial.println("###   Hono Tenant Provision   ######################################"); 
    Serial.println("POST to Server: "+srv);  
    Serial.print("HTTP Response code: ");
    Serial.println(response);

    http.end();
    delay(ms);
    return response;
}

int DigitalTwin::createHonoDevice (String ns, String device)
{
    HTTPClient http;
    honoNamespace = ns;
    honoDevice = device;
    String srv = devRegURL + "/v1/devices/" + honoTenant + "/" + honoNamespace + ":" + honoDevice;
    http.begin(*_client,srv);

    int response = http.POST("");
    Serial.println();
    Serial.println("###   Hono Device Provision   ######################################");  
    Serial.println("POST to Server: "+srv);  
    Serial.print("HTTP Response code: ");
    Serial.println(response);

    http.end();
    delay(ms);
    return response;
}

int DigitalTwin::createHonoCredentials (String pass)
{
    HTTPClient http;
    honoDevicePassword = pass;
    String srv = devRegURL + "/v1/credentials/" + honoTenant + "/" + honoNamespace + ":" + honoDevice;
    http.begin(*_client,srv);
    http.addHeader("Content-Type", "application/json");

    String jsonParse = "[{\"type\":\"hashed-password\",\"auth-id\":\"" + honoDevice + "\",\"secrets\":[{\"pwd-plain\":\"" + honoDevicePassword + "\"}]}]";

    int response = http.PUT(jsonParse);
    Serial.println();
    Serial.println("###   Hono Credential Provision   ##################################"); 
    Serial.println("PUT to Server: "+srv);  
    Serial.print("HTTP Response code: ");
    Serial.println(response);

    http.end();
    delay(ms);
    return response;
}

int DigitalTwin::createDittoPiggyback (String json)
{
    HTTPClient http;
    String srv = dittoURL + "/devops/piggyback/connectivity";
    http.begin(*_client,srv);
    http.addHeader("Content-Type", "application/json");
    http.setAuthorization("devops", "foobar");

    String jsonParse = json;

    int response = http.POST(jsonParse);
    Serial.println();
    Serial.println("###   Ditto Piggyback Provision   ##################################"); 
    Serial.println("POST to Server: "+srv);  
    Serial.print("HTTP Response code: ");
    Serial.println(response);

    http.end();
    delay(ms);
    return response;
}

int DigitalTwin::createDittoPolicy (String json)
{
    HTTPClient http;
    String srv = dittoURL + "/api/2/policies/" + honoNamespace + ":" + honoDevice;
    http.begin(*_client,srv);
    http.addHeader("Content-Type", "application/json");
    http.setAuthorization("ditto", "ditto");

    String jsonParse = json;

    int response = http.PUT(jsonParse);
    Serial.println();
    Serial.println("###   Ditto Policy Provision   #####################################"); 
    Serial.println("PUT to Server: "+srv);  
    Serial.print("HTTP Response code: ");
    Serial.println(response);

    http.end();
    delay(ms);
    return response;
}

int DigitalTwin::createDittoThing (String json)
{
    HTTPClient http;
    String srv = dittoURL + "/api/2/things/" + honoNamespace + ":" + honoDevice;
    http.begin(*_client,srv);
    http.addHeader("Content-Type", "application/json");
    http.setAuthorization("ditto", "ditto");

    String jsonParse = json;

    int response = http.PUT(jsonParse);
    Serial.println();
    Serial.println("###   Ditto Thing Provision   ######################################");
    Serial.println("PUT to Server: "+srv);  
    Serial.print("HTTP Response code: ");
    Serial.println(response);

    http.end();
    delay(ms);
    return response;
}

int DigitalTwin::createDittoFeatures (String json)
{
    HTTPClient http;
    String srv = dittoURL + "/api/2/things/" + honoNamespace + ":" + honoDevice + "/features";
    http.begin(*_client,srv);
    http.addHeader("Content-Type", "application/json");
    http.setAuthorization("ditto", "ditto");

    String jsonParse = json;

    int response = http.PUT(jsonParse);
    Serial.println();
    Serial.println("###   Ditto Features Provision   ###################################");
    Serial.println("PUT to Server: "+srv);  
    Serial.print("HTTP Response code: ");
    Serial.println(response);

    http.end();
    delay(ms);
    return response;
}

int DigitalTwin::createNodeRedDashboard (String json)
{
    HTTPClient http;
    String srv = "http://jreichwald.de:1880/auth/token";
    http.begin(*_client,srv);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    int response = http.POST("client_id=node-red-admin&grant_type=password&scope=*&username=admin&password=admin1234");

    Serial.println();
    Serial.println("###   Ditto Features Provision   ###################################");
    Serial.println("POST to Server: "+srv);  
    Serial.print("HTTP Response code: ");
    Serial.println(response);

    nodeRedToken = (http.getString()).substring(17,189);

    http.end();

    HTTPClient http2;
    srv = "http://jreichwald.de:1880/flow";
    http2.begin(*_client,srv);
    http2.addHeader("Authorization", "Bearer " + nodeRedToken);
    http2.addHeader("Content-Type", "application/json");

    response = http2.POST(json);
    Serial.println();
    Serial.println("###   NodeRed Dashboard Provision   ###################################");
    Serial.println("POST to Server: "+srv);  
    Serial.print("HTTP Response code: ");
    Serial.println(response);

    http2.end();
    
    delay(ms);
    return response;
    
}



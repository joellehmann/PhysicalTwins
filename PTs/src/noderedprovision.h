// Library #############################################################
// Provision of Nodered Dashboards #####################################
// Masterthesis V0.0.1 #################################################
// by Joel Lehmann #####################################################
// 09.07.2021 ##########################################################

#ifndef noderedprovision
#define noderedprovision

#include <Arduino.h>
#include <ESP8266HTTPClient.h>

class NodeRed
{
    private:

    WiFiClient* _client;
    String serverIP;
    int ms;
    String username;
    String password;
    String nodeRedServer;
    String jsonDashboard;
    int ypos = 50;
    String uiGroupLeftID;
    String uiGroupMidID;
    String uiGroupRightID;
    int orderLeft = 1;
    int orderMid = 1;
    int orderRight = 1;
    String uiTabID;
    String zID;
    String pollingTime;
    String nodeRedToken;
    String id;

    public: 

    NodeRed();
    bool init (WiFiClient &client, String nRServer, String deviceID, String pollingTime);
    void addGauge(String address, String name, String unit, int max, int min, int deadband);
    void addChart(String address, String name, int max, int min, int deadband, int archiveHours);
    void addSwitch(String onAddress, String offAddress, String name);
    void addButton(String address, String name);
    void createNodeRedDashboard ();
};

#endif

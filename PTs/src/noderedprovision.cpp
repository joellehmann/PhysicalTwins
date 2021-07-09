// Library #############################################################
// Provision of Nodered Dashboards #####################################
// Masterthesis V0.0.1 #################################################
// by Joel Lehmann #####################################################
// 09.07.2021 ##########################################################

#include "noderedprovision.h"
#include <Arduino.h>
#include <ESP8266HTTPClient.h>

NodeRed::NodeRed()
{

}

// Credentials implementieren
bool NodeRed::init (WiFiClient &client, String nRServer, String deviceID, String _pollingTime)
{
    randomSeed(analogRead(0));
    id = deviceID;
    uiGroupLeftID = id + "." +String(random(99))+String(random(99))+String(random(99));
    uiGroupMidID = id + "." +String(random(99))+String(random(99))+String(random(99));
    uiGroupRightID = id + "." +String(random(99))+String(random(99))+String(random(99));
    uiTabID = id; //+ "." +String(random(99))+String(random(99))+String(random(99));
    String flowID = String(random(99))+String(random(99))+String(random(99));
    zID = id + "." + flowID;
    pollingTime = _pollingTime;
    
    nodeRedServer = nRServer;

    _client = &client;
    HTTPClient http;
    String srv = nodeRedServer+"/auth/token";
    http.begin(*_client,srv);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    int response = http.POST("client_id=node-red-admin&grant_type=password&scope=*&username=admin&password=admin1234");

    Serial.println();
    Serial.println("###   Nodered Authentification   ###################################");
    Serial.println("POST to Server: "+srv);  
    Serial.print("HTTP Response code: ");
    Serial.println(response);

    if (response > 199) // Errorhandler
    {
        nodeRedToken = (http.getString()).substring(17,189);
        //Serial.println(http.getString());
    }

    Serial.println("TOKEN: " + nodeRedToken);

    http.end();
    return true;

}

void NodeRed::addGauge(String address, String name, String unit, int max, int min, int deadband)
{
    String gaugeID0 = String(random(99))+String(random(99))+String(random(99));
    String gaugeID1 = String(random(99))+String(random(99))+String(random(99));
    String gaugeID2 = String(random(99))+String(random(99))+String(random(99));
    String gaugeID3 = String(random(99))+String(random(99))+String(random(99));
    
    jsonDashboard = jsonDashboard + "{\"id\":\""+id+"."+gaugeID0+"\",\"type\":\"inject\",\"z\":\""+zID+"\",\"repeat\":\""+pollingTime+"\",\"payloadType\":\"date\",\"x\":100,\"y\":"+ypos+",\"wires\":[[\""+id+"."+gaugeID1+"\"]]},{\"id\":\""+id+"."+gaugeID1+"\",\"type\":\"http request\",\"z\":\""+zID+"\",\"method\":\"GET\",\"url\":\""+address+"\",\"x\":300,\"y\":"+ypos+",\"wires\":[[\""+id+"."+gaugeID2+"\"]]}, {\"id\":\""+id+"."+gaugeID2+"\",\"type\":\"rbe\",\"z\":\""+zID+"\",\"func\":\"deadband\",\"gap\":\""+deadband+"\",\"inout\":\"out\",\"property\":\"payload\",\"x\":500,\"y\":"+ypos+",\"wires\":[[\""+id+"."+gaugeID3+"\"]]},{\"id\":\""+id+"."+gaugeID3+"\",\"type\":\"ui_gauge\",\"z\":\""+zID+"\",\"group\":\""+uiGroupLeftID+"\",\"order\":"+orderLeft+",\"gtype\":\"gage\",\"title\":\""+name+"\",\"label\":\""+unit+"\",\"format\":\"{{value}}\",\"min\":"+String(min)+",\"max\":\""+String(max)+"\",\"colors\":[\"#1605af\",\"#3ebe27\",\"#ca3838\"],\"x\":700,\"y\":"+ypos+"},";
    
    orderLeft += 1;
    ypos += 50;
}

void NodeRed::addChart(String address, String name, int max, int min, int deadband, int archiveHours)
{
    String chartID0 = String(random(99))+String(random(99))+String(random(99));
    String chartID1 = String(random(99))+String(random(99))+String(random(99));
    String chartID2 = String(random(99))+String(random(99))+String(random(99));
    String chartID3 = String(random(99))+String(random(99))+String(random(99));
    
    jsonDashboard = jsonDashboard + "{\"id\":\""+id+"."+chartID0+"\",\"type\":\"inject\",\"z\":\""+zID+"\",\"repeat\":\""+pollingTime+"\",\"payloadType\":\"date\",\"x\":100,\"y\":"+ypos+",\"wires\":[[\""+id+"."+chartID1+"\"]]},{\"id\":\""+id+"."+chartID1+"\",\"type\":\"http request\",\"z\":\""+zID+"\",\"method\":\"GET\",\"url\":\""+address+"\",\"x\":300,\"y\":"+ypos+",\"wires\":[[\""+id+"."+chartID2+"\"]]}, {\"id\":\""+id+"."+chartID2+"\",\"type\":\"rbe\",\"z\":\""+zID+"\",\"func\":\"deadband\",\"gap\":\""+deadband+"\",\"inout\":\"out\",\"property\":\"payload\",\"x\":500,\"y\":"+ypos+",\"wires\":[[\""+id+"."+chartID3+"\"]]},{\"id\":\""+id+"."+chartID3+"\",\"type\":\"ui_chart\",\"z\":\""+zID+"\",\"group\":\""+uiGroupMidID+"\",\"order\":"+orderMid+",\"label\":\""+name+"\",\"interpolate\":\"linear\",\"ymin\":\""+String(min)+"\",\"ymax\":\""+String(max)+"\",\"removeOlder\":\""+archiveHours+"\",\"removeOlderUnit\":\"3600\",\"outputs\":1,\"x\":700,\"y\":"+ypos+"},";

    orderMid += 1;
    ypos += 50;
}

void NodeRed::addSwitch(String onAddress, String offAddress, String name)
{
    String switchID0 = String(random(99))+String(random(99))+String(random(99));
    String switchID1 = String(random(99))+String(random(99))+String(random(99));
    String switchID2 = String(random(99))+String(random(99))+String(random(99));
    String switchID3 = String(random(99))+String(random(99))+String(random(99));
    String switchID4 = String(random(99))+String(random(99))+String(random(99));

    jsonDashboard = jsonDashboard + "{\"id\":\""+id+"."+switchID0+"\",\"type\":\"ui_switch\",\"z\":\""+zID+"\",\"label\":\""+name+"\",\"group\":\""+uiGroupRightID+"\",\"order\":"+orderRight+",\"decouple\":\"false\",\"topic\":\"topic\",\"topicType\":\"msg\",\"onvalue\":\"true\",\"onvalueType\":\"bool\",\"offvalue\":\"false\",\"offvalueType\":\"bool\",\"x\":100,\"y\":"+String(ypos+25)+",\"wires\":[[\""+id+"."+switchID1+"\",\""+id+"."+switchID2+"\"]]},{\"id\":\""+id+"."+switchID1+"\",\"type\":\"switch\",\"z\":\""+zID+"\",\"property\":\"payload\",\"propertyType\":\"msg\",\"rules\":[{\"t\":\"true\"}],\"checkall\":\"true\",\"repair\":false,\"outputs\":1,\"x\":300,\"y\":"+String(ypos)+",\"wires\":[[\""+id+"."+switchID3+"\"]]},{\"id\":\""+id+"."+switchID2+"\",\"type\":\"switch\",\"z\":\""+zID+"\",\"property\":\"payload\",\"propertyType\":\"msg\",\"rules\":[{\"t\":\"false\"}],\"checkall\":\"true\",\"repair\":false,\"outputs\":1,\"x\":300,\"y\":"+String(ypos+50)+",\"wires\":[[\""+id+"."+switchID4+"\"]]},{\"id\":\""+id+"."+switchID3+"\",\"type\":\"http request\",\"z\":\""+zID+"\",\"method\":\"POST\",\"url\":\""+offAddress+"\",\"x\":500,\"y\":"+String(ypos)+"},{\"id\":\""+id+"."+switchID4+"\",\"type\":\"http request\",\"z\":\""+zID+"\",\"method\":\"POST\",\"url\":\""+onAddress+"\",\"x\":500,\"y\":"+String(ypos+50)+"},";

    orderRight += 1;
    ypos += 100;
}

void NodeRed::addButton(String address, String name)
{
    String buttonID0 = String(random(99))+String(random(99))+String(random(99));
    String buttonID1 = String(random(99))+String(random(99))+String(random(99));

    jsonDashboard = jsonDashboard + "{\"id\":\""+id+"."+buttonID0+"\",\"type\":\"ui_button\",\"z\":\""+zID+"\",\"group\":\""+uiGroupRightID+"\",\"order\":"+orderRight+",\"label\":\""+name+"\",\"payloadType\":\"str\",\"topic\":\"topic\",\"topicType\":\"msg\",\"x\":100,\"y\":"+ypos+",\"wires\":[[\""+id+"."+buttonID1+"\"]]},{\"id\":\""+id+"."+buttonID1+"\",\"type\":\"http request\",\"z\":\""+zID+"\",\"method\":\"POST\",\"url\":\""+address+"\",\"x\":300,\"y\":"+ypos+"},";

    orderRight += 1;
    ypos += 50;
}

void NodeRed::createNodeRedDashboard ()
{
    String mainID = id + "." + String(random(99))+String(random(99))+String(random(99));

    jsonDashboard = "{\"id\":\""+mainID+"\",\"label\":\"AUTO FLOW of "+id+"\",\"nodes\":["+jsonDashboard;

    jsonDashboard = jsonDashboard + "{\"id\":\""+uiGroupLeftID+"\",\"type\":\"ui_group\",\"name\":\"TWIN STATUS\",\"tab\":\""+uiTabID+"\",\"order\":1,\"disp\":true,\"width\":\"6\",\"collapse\":false},{\"id\":\""+uiGroupMidID+"\",\"type\":\"ui_group\",\"name\":\"TWIN TREND\",\"tab\":\""+uiTabID+"\",\"order\":2,\"disp\":true,\"width\":\"6\",\"collapse\":false},{\"id\":\""+uiGroupRightID+"\",\"type\":\"ui_group\",\"name\":\"TWIN COMMAND & CONTROL\",\"tab\":\""+uiTabID+"\",\"order\":3,\"disp\":true,\"width\":\"6\",\"collapse\":false},{\"id\":\""+uiTabID+"\",\"type\":\"ui_tab\",\"name\":\"Autonomous Generated Digital Twin Dashboard of "+mainID+"\",\"icon\":\"dashboard\",\"order\":1,\"disabled\":false,\"hidden\":false}" + "],\"configs\":[]}";

    HTTPClient http;
    String srv = nodeRedServer+"/flow";
    http.begin(*_client,srv);
    http.addHeader("Authorization", "Bearer " + nodeRedToken);
    http.addHeader("Content-Type", "application/json");

    int response = http.POST(jsonDashboard);
    Serial.println();
    Serial.println("###   NodeRed Dashboard Provision   #################################");
    Serial.println("POST to Server: "+srv);  
    Serial.print("HTTP Response code: ");
    Serial.println(response);
    Serial.println(http.getString());
    Serial.println();
    Serial.println(jsonDashboard);

    http.end();
    
    delay(ms);
}




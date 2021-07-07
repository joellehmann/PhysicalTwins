#include <Wire.h>
#include <SoftwareSerial.h> 
#include <LiquidCrystal_I2C.h> 
#include "BlueDot_BME280.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

#define RX_PIN D7                                          
#define TX_PIN D6 
#define windowsize 10
#define sensorname "KVE_PT"
#define TOPIC "KVE/LEHMANN/"

// HTTP POST CREATE TENANT
String jsonFile;
String serverName;
int httpResponseCode;
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;
HTTPClient http;
HTTPClient http2;
HTTPClient http3;
HTTPClient http4;
HTTPClient http5;
HTTPClient http6;
HTTPClient http7;
   
const double ppmIncrement = 0.001005; 
const double ppmstop = 1.8;

#define Version "0.6.0"
#define Date "07.07.2021"

char topHUM[30];
char topTEMP[30];
char topCO2[30];
char topPRESS[30];

const char* SSID = "Bates Motel";
const char* PSK = "1338591091304385";
const char* MQTT_BROKER = "141.19.44.65";
const char* mqttUser = "PA28@flugplatz";
const char* mqttPassword = "flugzeug";
const char* clientIds = sensorname;
char wiFiHostname[ ] = sensorname;
String clientId = sensorname;
long lastMsg = 0;
char msg[50];
int value = 0;
int ppm, RAWppm;
int cntppm = 0;
int ppmint;
int ppmausgabe;
double ppmfaktor;
double ppmcal = 0;
bool out;
int i = 0;
int Mtemp, Mhum, Mpress;
char antwort[9];
int avgindex = 0;
int sum = 0;
int lastppm[windowsize] = {0,0,0,0,0,0,0,0,0,0};
char * top;
bool flagMsg;
bool toggleLight = false;
bool wasBad = false;
int zykluszeit = 3000;
int aktStunde;
bool noWifi = false;
int cntWifi;

WiFiClient espClient;
PubSubClient client(espClient);
LiquidCrystal_I2C lcd(0x27, 20, 4);
BlueDot_BME280 bme280 = BlueDot_BME280();
SoftwareSerial co2Serial(D7, D6); // RX, TX

int leseCO2()
{
  byte cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  co2Serial.write(cmd, 9);
  co2Serial.readBytes(antwort, 9);

  int antwortHigh = (int) antwort[2];
  int antwortLow = (int) antwort[3];
  ppmint = (256 * antwortHigh) + antwortLow;
  
  //Serial.println(ppmint);
  //Serial.println(antwort[0],HEX);
  //Serial.println(antwort[1],HEX);
  
  if (antwort[0] != 0xFF || antwort[1] != 0x86 || ppmint < 200 || ppmint > 4000 || Mtemp > 80 || Mhum < 1 || Mpress < 800) {
    i++;
    ppmint = 0;
    if (i>10) {  
      ESP.restart();
    }    
    return ppmint;   
  } else {
    i = 0;
  }
  return ppmint;
}

void setup_wifi() {
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(SSID);
    wifi_station_set_hostname(wiFiHostname);
    WiFi.begin(SSID, PSK);
    while (WiFi.status() != WL_CONNECTED && cntWifi < 40) {
        delay(500);
        Serial.print(".");
        cntWifi++;
        noWifi = false;
    }
    
    if (cntWifi > 35){
      noWifi = true;
      Serial.println("NO WIFI");
    }
    
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void reconnect() {
  if (noWifi == false) {
    while (!client.connected()) {
        Serial.print("Reconnecting...");
        if (!client.connect(clientId.c_str())) {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" retrying in 5 seconds");
            delay(5000);
        }
        client.subscribe("buero/JL/CO2");
    }
  }
}

void setup() {
  
  lcd.init(); 
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  /**
  lcd.print("INP Deutschland GmbH");
  lcd.setCursor(0,1);
  lcd.print("Version:");
  lcd.setCursor(15,1);
  lcd.print(Version);
  lcd.setCursor(0,2);
  lcd.print("Datum:");
  lcd.setCursor(10,2);
  lcd.print(Date);
  lcd.setCursor(0,3);
  lcd.print("by    Joel   Lehmann");
  **/
  strcpy(topHUM,TOPIC);
  strcat(topHUM,"HUM");

  strcpy(topTEMP,TOPIC);
  strcat(topTEMP,"TEMP");

  strcpy(topCO2,TOPIC);
  strcat(topCO2,"CO2");

  strcpy(topPRESS,TOPIC);
  strcat(topPRESS,"PRESS");
  
  Serial.begin(9600);
  co2Serial.begin(9600);
  
  //byte cmdabc[9] = {0xFF, 0x01, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x86}; //ABC off
  //co2Serial.write(cmdabc, 9);
  
   
  bme280.parameter.communication = 0;                  //Choose communication protocol
  bme280.parameter.I2CAddress = 0x76;                  //Choose I2C Address
  bme280.parameter.sensorMode = 0b11;                  //Choose sensor mode
  bme280.parameter.IIRfilter = 0b100;                  //Setup for IIR Filter
  bme280.parameter.humidOversampling = 0b101;          //Setup Humidity Oversampling
  bme280.parameter.tempOversampling = 0b101;           //Setup Temperature Ovesampling
  bme280.parameter.pressOversampling = 0b101;          //Setup Pressure Oversampling 
  bme280.parameter.pressureSeaLevel = 1013.25;         //default value of 1013.25 hPa
  bme280.parameter.tempOutsideCelsius = 15;            //default value of 15°C
  bme280.init();
  setup_wifi();
  client.setServer(MQTT_BROKER, 8883);



  lcd.clear();
  if ( noWifi == true ) {
    lcd.setCursor(0,0);
    lcd.print("inp smart CO2 noWiFi");
  } else {
    lcd.setCursor(0,0);
    lcd.print("inp smart CO2 sensor");
  }
  lcd.backlight();
      
  lcd.setCursor(0,2);
  lcd.print("Temp:");
  lcd.setCursor(0,3);
  lcd.print("rF:");
  lcd.setCursor(0,1);
  lcd.print("CO2:");

  lcd.setCursor(19,2);
  lcd.print("C");
  
  lcd.setCursor(19,3);
  lcd.print("%");
  lcd.setCursor(17,1);
  lcd.print("ppm");

  //######################################################################
  // HTTP POST CREATE TENANT #############################################
  //######################################################################

  serverName = "http://141.19.44.65:28443/v1/tenants/flugplatz";
  http.begin(espClient,serverName);

  httpResponseCode = http.POST("");

  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("DIGITAL TWINS @ KVE");
  lcd.setCursor(0,1);
  lcd.print("HONO TENANT PROV");
  lcd.setCursor(0,3);
  lcd.print("HTTP RESPONSE:");
  lcd.setCursor(16,3);
  lcd.print(httpResponseCode);

  http.end();
  delay(3000);

  //######################################################################
  // HTTP POST CREATE DEVICE #############################################
  //######################################################################

  serverName = "http://141.19.44.65:28443/v1/devices/flugplatz/org.Piper:PA28";
  http2.begin(espClient,serverName);

  httpResponseCode = http2.POST("");

  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("DIGITAL TWINS @ KVE");
  lcd.setCursor(0,1);
  lcd.print("HONO DEVICE PROV");
  lcd.setCursor(0,3);
  lcd.print("HTTP RESPONSE:");
  lcd.setCursor(16,3);
  lcd.print(httpResponseCode);

  http2.end();
  delay(3000);

  //######################################################################
  // HTTP PUT CREDENTIALS ################################################
  //######################################################################

  DynamicJsonDocument configJSON(1024);

  configJSON["type"] = "hashed-password";
  configJSON["auth-id"] = "PA28";
  configJSON["secrets"][0]["pwd-plain"] = "flugzeug";

  serializeJson(configJSON, jsonFile);

  Serial.println(jsonFile);

  /* JSON FILE
  {
    "type": "hashed-password",
    "auth-id": "aircraft",
    "secrets": [{
      "pwd-plain": "flugzeug"
    }]
  }
  */

  serverName = "http://141.19.44.65:28443/v1/credentials/flugplatz/org.Piper:PA28";
  http3.begin(espClient,serverName);
  http3.addHeader("Content-Type", "application/json");

  httpResponseCode = http3.PUT("["+jsonFile+"]");

  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("DIGITAL TWINS @ KVE");
  lcd.setCursor(0,1);
  lcd.print("HONO CREDENTIALS PROV");
  lcd.setCursor(0,3);
  lcd.print("HTTP RESPONSE:");
  lcd.setCursor(16,3);
  lcd.print(httpResponseCode);

  http3.end();
  delay(5000);

  //######################################################################
  // HTTP POST DITTO PIGGYBACK ###########################################
  //######################################################################
  
  String dittoPiggyback = R"=====(
    {
      "targetActorSelection": "/system/sharding/connection",
      "headers": {
        "aggregate": false
      },
      "piggybackCommand": {
        "type": "connectivity.commands:createConnection",
        "connection": {
          "id": "hono-connection-for-flugplatz",
          "connectionType": "amqp-10",
          "connectionStatus": "open",
          "uri": "amqp://consumer%40HONO:verysecret@c2e-dispatch-router-ext:15672",
          "failoverEnabled": true,
          "sources": [
            {
              "addresses": [
                "telemetry/flugplatz",
                "event/flugplatz"
              ],
              "authorizationContext": [
                "pre-authenticated:hono-connection"
              ],
              "enforcement": {
                "input": "{{ header:device_id }}",
                "filters": [
                  "{{ entity:id }}"
                ]
              },
              "headerMapping": {
                "hono-device-id": "{{ header:device_id }}",
                "content-type": "{{ header:content-type }}"
              },
              "replyTarget": {
                "enabled": true,
                "address": "{{ header:reply-to }}",
                "headerMapping": {
                  "to": "command/flugplatz/{{ header:hono-device-id }}",
                  "subject": "{{ header:subject | fn:default(topic:action-subject) | fn:default(topic:criterion) }}-response",
                  "correlation-id": "{{ header:correlation-id }}",
                  "content-type": "{{ header:content-type | fn:default('application/vnd.eclipse.ditto+json') }}"
                },
                "expectedResponseTypes": [
                  "response",
                  "error"
                ]
              },
              "acknowledgementRequests": {
                "includes": [],
                "filter": "fn:filter(header:qos,'ne','0')"
              }
            },
            {
              "addresses": [
                "command_response/flugplatz/replies"
              ],
              "authorizationContext": [
                "pre-authenticated:hono-connection"
              ],
              "headerMapping": {
                "content-type": "{{ header:content-type }}",
                "correlation-id": "{{ header:correlation-id }}",
                "status": "{{ header:status }}"
              },
              "replyTarget": {
                "enabled": false,
                "expectedResponseTypes": [
                  "response",
                  "error"
                ]
              }
            }
          ],
          "targets": [
            {
              "address": "command/flugplatz",
              "authorizationContext": [
                "pre-authenticated:hono-connection"
              ],
              "topics": [
                "_/_/things/live/commands",
                "_/_/things/live/messages"
              ],
              "headerMapping": {
                "to": "command/flugplatz/{{ thing:id }}",
                "subject": "{{ header:subject | fn:default(topic:action-subject) }}",
                "content-type": "{{ header:content-type | fn:default('application/vnd.eclipse.ditto+json') }}",
                "correlation-id": "{{ header:correlation-id }}",
                "reply-to": "{{ fn:default('command_response/flugplatz/replies') | fn:filter(header:response-required,'ne','false') }}"
              }
            },
            {
              "address": "command/flugplatz",
              "authorizationContext": [
                "pre-authenticated:hono-connection"
              ],
              "topics": [
                "_/_/things/twin/events",
                "_/_/things/live/events"
              ],
              "headerMapping": {
                "to": "command/flugplatz/{{ thing:id }}",
                "subject": "{{ header:subject | fn:default(topic:action-subject) }}",
                "content-type": "{{ header:content-type | fn:default('application/vnd.eclipse.ditto+json') }}",
                "correlation-id": "{{ header:correlation-id }}"
              }
            }
          ]
        }
      }
    }
  )=====";

  serverName = "http://141.19.44.65:38443/devops/piggyback/connectivity";
  http4.begin(espClient,serverName);
  http4.setAuthorization("devops", "foobar");
  http4.addHeader("Content-Type", "application/json");
  
  httpResponseCode = http4.POST(dittoPiggyback);
  
  Serial.println(dittoPiggyback);
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);
  Serial.println(http4.getString());

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("DIGITAL TWINS @ KVE");
  lcd.setCursor(0,1);
  lcd.print("DITTO PIGGYBACK PROV");
  lcd.setCursor(0,3);
  lcd.print("HTTP RESPONSE:");
  lcd.setCursor(16,3);
  lcd.print(httpResponseCode);

  http4.end();
  delay(5000);
  

  //######################################################################
  // HTTP CREATE DITTO POLICY ############################################
  //######################################################################
  
  String dittoPolicy = R"=====(
    {
      "entries": {
        "DEFAULT": {
          "subjects": {
            "{{ request:subjectId }}": {
              "type": "Ditto user authenticated via nginx"
            }
          },
          "resources": {
            "thing:/": {
              "grant": ["READ", "WRITE"],
              "revoke": []
            },
            "policy:/": {
              "grant": ["READ", "WRITE"],
              "revoke": []
            },
            "message:/": {
              "grant": ["READ", "WRITE"],
              "revoke": []
            }
          }
        },
        "HONO": {
          "subjects": {
            "pre-authenticated:hono-connection": {
              "type": "Connection to Eclipse Hono"
            }
          },
          "resources": {
            "thing:/": {
              "grant": ["READ", "WRITE"],
              "revoke": []
            },
            "message:/": {
              "grant": ["READ", "WRITE"],
              "revoke": []
            }
          }
        }
      }
    }
  )=====";

  serverName = "http://141.19.44.65:38443/api/2/policies/org.Piper:PA28";
  http5.begin(espClient,serverName);
  Serial.println(serverName);
  http5.setAuthorization("ditto", "ditto");
  http5.addHeader("Content-Type", "application/json");
  
  httpResponseCode = http5.PUT(dittoPolicy);
  
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);
  Serial.println(http5.getString());

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("DIGITAL TWINS @ KVE");
  lcd.setCursor(0,1);
  lcd.print("DITTO POLICY PROV");
  lcd.setCursor(0,3);
  lcd.print("HTTP RESPONSE:");
  lcd.setCursor(16,3);
  lcd.print(httpResponseCode);

  http5.end();
  delay(5000);
  
  //######################################################################
  // HTTP CREATE DITTO THING #############################################
  //######################################################################
  
  String dittoTwin = R"=====(
    {
      "policyId": "org.Piper:PA28",
      "attributes": {
        "location": "Germany",
        "airportID": "EDRO",
        "airportName": "Sonderlandeplatz Schweighofen"
      },
      "features": {
      }
    }
  )=====";

  serverName = "http://141.19.44.65:38443/api/2/things/org.Piper:PA28";
  http6.begin(espClient,serverName);
  Serial.println(serverName);
  http6.setAuthorization("ditto", "ditto");
  http6.addHeader("Content-Type", "application/json");
  
  httpResponseCode = http6.PUT(dittoTwin);
  
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);
  Serial.println(http6.getString());

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("DIGITAL TWINS @ KVE");
  lcd.setCursor(0,1);
  lcd.print("DITTO Thing PROV");
  lcd.setCursor(0,3);
  lcd.print("HTTP RESPONSE:");
  lcd.setCursor(16,3);
  lcd.print(httpResponseCode);

  http6.end();
  delay(5000);
  

  //######################################################################
  // HTTP CREATE DITTO THING FEATURES ####################################
  //######################################################################
  
  String dittoFeatures = R"=====(
    {
      "telemetry": {
        "properties": {
          "JoelTemp": {
            "value": null,
          "unit": "Celsius"
          },
          "JoelHum": {
            "value": null,
          "unit": "Celsius"
          },
          "JoelCO2": {
            "value": null,
          "unit": "ppm"
          }
          
        }
      }
    }
  )=====";

  serverName = "http://141.19.44.65:38443/api/2/things/org.Piper:PA28/features";
  http7.begin(espClient,serverName);
  Serial.println(serverName);
  http7.setAuthorization("ditto", "ditto");
  http7.addHeader("Content-Type", "application/json");
  
  httpResponseCode = http7.PUT(dittoFeatures);
  
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);
  Serial.println(http7.getString());

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("DIGITAL TWINS @ KVE");
  lcd.setCursor(0,1);
  lcd.print("DITTO Thing PROV");
  lcd.setCursor(0,3);
  lcd.print("HTTP RESPONSE:");
  lcd.setCursor(16,3);
  lcd.print(httpResponseCode);

  http7.end();
  delay(5000);

  lcd.clear();
  if ( noWifi == true ) {
    lcd.setCursor(0,0);
    lcd.print("inp smart CO2 noWiFi");
  } else {
    lcd.setCursor(0,0);
    lcd.print("inp smart CO2 sensor");
  }
  lcd.backlight();
      
  lcd.setCursor(0,2);
  lcd.print("Temp:");
  lcd.setCursor(0,3);
  lcd.print("rF:");
  lcd.setCursor(0,1);
  lcd.print("CO2:");

  lcd.setCursor(19,2);
  lcd.print("C");
  
  lcd.setCursor(19,3);
  lcd.print("%");
  lcd.setCursor(17,1);
  lcd.print("ppm");
}

void loop() 
{ 

  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");

     if (client.connect(clientIds, mqttUser, mqttPassword)) {
 
      Serial.println("connected");  
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
 
    }
  }
   client.loop();
   
   Mtemp = bme280.readTempC() - 2;
   Mhum = bme280.readHumidity() + 10;
   Mpress = bme280.readPressure();
   RAWppm = leseCO2();
   
   

     
  ppmausgabe = RAWppm;
  lcd.setCursor(7,1);
  lcd.print("        ");
  lcd.setCursor(10,1);
  lcd.print(ppmausgabe);    

  String TE = "{  \"topic\": \"org.Piper/PA28/things/twin/commands/modify\",  \"headers\": {},  \"path\": \"/features/telemetry/properties/JoelTemp/value\",  \"value\": "+String(Mtemp)+"}";
  
  String HU = "{  \"topic\": \"org.Piper/PA28/things/twin/commands/modify\",  \"headers\": {},  \"path\": \"/features/telemetry/properties/JoelHum/value\",  \"value\": "+String(Mhum)+"}";
  
  String CO = "{  \"topic\": \"org.Piper/PA28/things/twin/commands/modify\",  \"headers\": {},  \"path\": \"/features/telemetry/properties/JoelCO2/value\",  \"value\": "+String(ppmausgabe)+"}";

  client.publish("telemetry", String(TE).c_str(),false);
  client.publish("telemetry", String(HU).c_str(),false);
  client.publish("telemetry", String(CO).c_str(),false);

  lcd.setCursor(10,2);
  lcd.print("      ");
  lcd.setCursor(10,2);
  
  lcd.print(String(Mtemp).c_str());
  lcd.setCursor(10,3);
  lcd.print("      ");
  lcd.setCursor(10,3);
  lcd.print(String(Mhum).c_str());
 
  delay(zykluszeit);   

}

void callback(char* topic, byte* payload, unsigned int length) {

  flagMsg = true;
  top = topic;
    
}
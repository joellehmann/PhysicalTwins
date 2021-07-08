// Autoprovisioning of Digital Twins ###################################
// Masterthesis ########################################################
// by Joel Lehmann #####################################################

#include <Wire.h>
#include <SoftwareSerial.h> 
#include <LiquidCrystal_I2C.h> 
#include "BlueDot_BME280.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <dtprovision.h>
#include "credentials.h"

#define RX_PIN D7                                          
#define TX_PIN D6 
#define sensorname "KVE_PT"
#define defHonoTenant "ceMOS"
#define defHonoNamespace "lehmann"
#define defHonoDevice "smartDTsensor"
#define defHonoDevicePassword "sehrgeheim"
#define defServerIP "http://141.19.44.65"
#define defTelemetryPort 18443
#define defDevRegPort 28443
#define defDittoPort 38443

String serverName;
int httpResponseCode;
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;
int httpResponse;
int httpResonse;
const String honoTenant = defHonoTenant;
const String honoNamespace = defHonoNamespace;
const String honoDevice = defHonoDevice;
const int provDelay = 500;
int counter = 4990;

DigitalTwin DTI;
   
const double ppmIncrement = 0.001005; 
const double ppmstop = 1.8;

#define Version "2.0.0"
#define Date "08.07.2021"

const char* SSID = WLANSSID;
const char* PSK = WLANPSK;
const char* MQTT_BROKER = "141.19.44.65";
String tmpMqttUser = honoDevice + "@" + honoTenant;
const char* mqttUser = tmpMqttUser.c_str();
const char* mqttPassword = defHonoDevicePassword;
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
bool flagMsg;
int zykluszeit = 5000;
bool noWifi = false;
int cntWifi;

WiFiClient espClient;
PubSubClient client(espClient);
LiquidCrystal_I2C lcd(0x27, 20, 4);
BlueDot_BME280 bme280 = BlueDot_BME280();
SoftwareSerial co2Serial(D7, D6); // RX, TX

//######################################################################
// MQTT CALLBACK #######################################################
//######################################################################

void callback(char* topic, byte* payload, unsigned int length) {
 
  //Serial.print("Message arrived in topic: ");
  //Serial.println(topic);
 
  //Serial.print("Message:");
  //for (int i = 0; i < length; i++) {
  //  Serial.print((char)payload[i]);
  //}
  
  if (String(topic) == "command///req//backlightOn") {
    lcd.backlight();
    Serial.println("-----------------------");
    Serial.println("COMMAND FROM DIGITAL TWIN: BACKLIGHT ON");
  }
  if (String(topic) == "command///req//backlightOff") {
    lcd.noBacklight();
    Serial.println("-----------------------");
    Serial.println("COMMAND FROM DIGITAL TWIN: BACKLIGHT OFF");
  }
  if (String(topic) == "command///req//espRestart") {
    Serial.println("-----------------------");
    Serial.println("COMMAND FROM DIGITAL TWIN: Restarting Device in 3 sec.");
    Serial.println("-----------------------");
    delay(3000);
    ESP.restart();
  }

  Serial.println("-----------------------");
 
}

//######################################################################
// READ CO2 ############################################################
//######################################################################

int leseCO2()
{
  byte cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  co2Serial.write(cmd, 9);
  co2Serial.readBytes(antwort, 9);

  int antwortHigh = (int) antwort[2];
  int antwortLow = (int) antwort[3];
  ppmint = (256 * antwortHigh) + antwortLow;
    
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

//######################################################################
// SETUP WiFi ##########################################################
//######################################################################

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

//######################################################################
// SETUP ###############################################################
//######################################################################

void setup() {
  
  lcd.init(); 
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  
  lcd.print("KVE HS-Mannheim");
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
  
  Serial.begin(9600);
  co2Serial.begin(9600);
     
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
  client.setCallback(callback);
  client.subscribe("command/+/+/req/#");

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
  // DTI INIT ############################################################
  //######################################################################

  DTI.init(espClient, defServerIP, defTelemetryPort, defDevRegPort, defDittoPort, provDelay);

  //######################################################################
  // HTTP POST CREATE TENANT #############################################
  //######################################################################

  httpResponse = DTI.createHonoTenant(honoTenant);

  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("HONO TENANT PROV");
  lcd.setCursor(0,3);
  lcd.print("HTTP RESPONSE:");
  lcd.setCursor(16,3);
  lcd.print(httpResponse);

  //######################################################################
  // HTTP POST CREATE DEVICE #############################################
  //######################################################################

  httpResponse = DTI.createHonoDevice(defHonoNamespace, defHonoDevice);

  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("HONO DEVICE PROV");
  lcd.setCursor(0,3);
  lcd.print("HTTP RESPONSE:");
  lcd.setCursor(16,3);
  lcd.print(httpResponse);

  //######################################################################
  // HTTP PUT CREDENTIALS ################################################
  //######################################################################

  httpResponse = DTI.createHonoCredentials(defHonoDevicePassword);

  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("HONO CREDENTIALS PROV");
  lcd.setCursor(0,3);
  lcd.print("HTTP RESPONSE:");
  lcd.setCursor(16,3);
  lcd.print(httpResponse);

  //######################################################################
  // HTTP POST DITTO PIGGYBACK ###########################################
  //######################################################################
  
  httpResponse = DTI.createDittoPiggyback();
  
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("DITTO PIGGYBACK PROV");
  lcd.setCursor(0,3);
  lcd.print("HTTP RESPONSE:");
  lcd.setCursor(16,3);
  lcd.print(httpResponse);

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

  httpResponse = DTI.createDittoPolicy(dittoPolicy);

  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("DITTO POLICY PROV");
  lcd.setCursor(0,3);
  lcd.print("HTTP RESPONSE:");
  lcd.setCursor(16,3);
  lcd.print(httpResponse);

  //######################################################################
  // HTTP CREATE DITTO THING #############################################
  //######################################################################
  
  String dittoTwin = R"=====(
    {
      "policyId": ")=====" + honoNamespace + ":" + honoDevice + R"=====(",
      "attributes": {
        "location": "Mannheim",
        "shortName": "HSMA",
        "longName": "Hochschule Mannheim",
        "institute": "KVE",
        "room": "007"
      },
      "features": {
      }
    }
  )=====";
  
  httpResponse = DTI.createDittoThing(dittoTwin);

  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("DITTO Thing PROV");
  lcd.setCursor(0,3);
  lcd.print("HTTP RESPONSE:");
  lcd.setCursor(16,3);
  lcd.print(httpResponse);
  
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
          "JoelPress": {
            "value": null,
            "unit": "hPa"
          },
          "JoelCO2": {
            "value": null,
            "unit": "ppm"
          }       
        }
      }
    }
  )=====";

  httpResponse = DTI.createDittoFeatures(dittoFeatures);

  //######################################################################
  // LCD ADJUSTMENTS #####################################################
  //######################################################################
  
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("DITTO Thing PROV");
  lcd.setCursor(0,3);
  lcd.print("HTTP RESPONSE:");
  lcd.setCursor(16,3);
  lcd.print(httpResponse);

  lcd.clear();
  if ( noWifi == true ) {
    lcd.setCursor(0,0);
    lcd.print("smartDTsensor noWiFi");
  } else {
    lcd.setCursor(0,0);
    lcd.print("smartDTsensor  by JL");
  }
      
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

//######################################################################
// LOOP ################################################################
//######################################################################
void loop() 
{ 

  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
     if (client.connect(clientIds, mqttUser, mqttPassword)) {
      Serial.println("connected");
      client.subscribe("command/+/+/req/#");  
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      client.subscribe("command/+/+/req/#");
      delay(2000);
    }
  }

  client.loop();

  //######################################################################
  // REFRESH SENSORDATA ##################################################
  //###################################################################### 

if (counter > 5000)
{
    Mtemp = bme280.readTempC() - 2;
    Mhum = bme280.readHumidity() + 10;
    Mpress = bme280.readPressure();
    RAWppm = leseCO2();
   
  //######################################################################
  // PUBLISH JSON SENSORDATA TO HONO #####################################
  //######################################################################     

    String TE = "{  \"topic\": \"" + honoNamespace + "/" + defHonoDevice + "/things/twin/commands/modify\",  \"headers\": {},  \"path\": \"/features/telemetry/properties/JoelTemp/value\",  \"value\": "+String(Mtemp)+"}";
    
    String HU = "{  \"topic\": \"" + honoNamespace + "/" + defHonoDevice + "/things/twin/commands/modify\",  \"headers\": {},  \"path\": \"/features/telemetry/properties/JoelHum/value\",  \"value\": "+String(Mhum)+"}";

    String PR = "{  \"topic\": \"" + honoNamespace + "/" + defHonoDevice + "/things/twin/commands/modify\",  \"headers\": {},  \"path\": \"/features/telemetry/properties/JoelPress/value\",  \"value\": "+String(Mpress)+"}";
    
    String CO = "{  \"topic\": \"" + honoNamespace + "/" + defHonoDevice + "/things/twin/commands/modify\",  \"headers\": {},  \"path\": \"/features/telemetry/properties/JoelCO2/value\",  \"value\": "+String(RAWppm)+"}";

    client.publish("telemetry", String(TE).c_str(),false);
    client.publish("telemetry", String(HU).c_str(),false);
    client.publish("telemetry", String(PR).c_str(),false);
    client.publish("telemetry", String(CO).c_str(),false);

  //######################################################################
  // LCD ADJUSTMENTS #####################################################
  //######################################################################

    lcd.setCursor(10,2);
    lcd.print("      ");
    lcd.setCursor(10,2);
    lcd.print(String(Mtemp).c_str());
    lcd.setCursor(10,3);
    lcd.print("      ");
    lcd.setCursor(10,3);
    lcd.print(String(Mhum).c_str());
    lcd.setCursor(7,1);
    lcd.print("        ");
    lcd.setCursor(10,1);
    lcd.print(RAWppm);
    counter = 0;
} else 
{
  counter++;
}
  delay(1);  

}


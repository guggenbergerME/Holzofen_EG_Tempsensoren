#include <Arduino.h>
#include <PubSubClient.h>  
#include "WiFi.h"
#include <OneWire.h>
#include <DallasTemperature.h>


/////////////////////////////////////////////////////////////////////////// Funktionsprototypen
void loop                             ();
void wifi_setup                       ();
void setup_ds18B20_adressen_ausgeben  ();
void callback                         (char* topic, byte* payload, unsigned int length);
void reconnect                        ();
void temperaturen_messen              ();

/////////////////////////////////////////////////////////////////////////// DS18b20
#define SENSOR_PIN  21 // ESP32 Pin OneWire
float tempC; // temperature in Celsius

OneWire oneWire(SENSOR_PIN);
DallasTemperature DS18B20(&oneWire);

/////////////////////////////////////////////////////////////////////////// Schleifen verwalten
unsigned long previousMillis_temp_messen = 0; // Spannung Messen
unsigned long interval_temp_messen = 10000; 

/////////////////////////////////////////////////////////////////////////// Kartendaten 
const char* kartenID = "Holzofen_EG_Regelung";

/////////////////////////////////////////////////////////////////////////// MQTT 
WiFiClient espClient;
PubSubClient client(espClient);

const char* mqtt_server = "192.168.150.1";

/////////////////////////////////////////////////////////////////////////// SETUP - Wifi
void wifi_setup() {

// WiFi Zugangsdaten
const char* WIFI_SSID = "GuggenbergerLinux";
const char* WIFI_PASS = "Isabelle2014samira";

// Static IP
IPAddress local_IP(192, 168, 5, 29);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 0, 0, 0);  
IPAddress dns(192, 168, 1, 1); 

// Verbindung zu SSID
Serial.print("Verbindung zu SSID - ");
Serial.println(WIFI_SSID); 

// IP zuweisen
if (!WiFi.config(local_IP, gateway, subnet, dns)) {
   Serial.println("STA fehlerhaft!");
  }

// WiFI Modus setzen
WiFi.mode(WIFI_OFF);
WiFi.disconnect();
delay(100);

WiFi.begin(WIFI_SSID, WIFI_PASS);
Serial.println("Verbindung aufbauen ...");

while (WiFi.status() != WL_CONNECTED) {

  if (WiFi.status() == WL_CONNECT_FAILED) {
     Serial.println("Keine Verbindung zum SSID möglich : ");
     Serial.println();
     Serial.print("SSID: ");
     Serial.println(WIFI_SSID);
     Serial.print("Passwort: ");
     Serial.println(WIFI_PASS);
     Serial.println();
    }
  delay(2000);
}
    Serial.println("");
    Serial.println("Mit Wifi verbunden");
    Serial.println("IP Adresse: ");
    Serial.println(WiFi.localIP());

}

/////////////////////////////////////////////////////////////////////////// SETUP - ds18B20 Adressen ausgeben
void setup_ds18B20_adressen_ausgeben(){

}

//****************************************************************************************** VOID mqtt reconnected
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    //Serial.print("Baue Verbindung zum mqtt Server auf. IP: ");
    // Attempt to connect
    if (client.connect(kartenID,"zugang1","43b4134735")) {
      //Serial.println("connected");
      ////////////////////////////////////////////////////////////////////////// SUBSCRIBE Eintraege
      //client.subscribe("relais_licht_wohnzimmer_1_0/IN");

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/////////////////////////////////////////////////////////////////////////// MQTT callback
void callback(char* topic, byte* payload, unsigned int length) {
/*
  /////////////////////////////////////////////////////////////////////////// Relais 0
      if (strcmp(topic,"relais_licht_wohnzimmer_1_0/IN")==0) {

          // ON und OFF Funktion auslesen
          if ((char)payload[0] == 'o' && (char)payload[1] == 'n') {  
                  Serial.println("relais_0 -> AN");

                }

          if ((char)payload[0] == 'o' && (char)payload[1] == 'f' && (char)payload[2] == 'f') {  
                  Serial.println("relais_0 -> AUS");

                }
        } 
  */
}


/////////////////////////////////////////////////////////////////////////// SETUP
void setup() {

  // Serielle Kommunikation starten
  Serial.begin(115200);

  // Wifi setup
  wifi_setup();

  // MQTT Broker
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  //DS18b20 Setup
  DS18B20.begin();

  // Adressen der Sensoren ausgeben
   setup_ds18B20_adressen_ausgeben();

}

/////////////////////////////////////////////////////////////////////////// Temperaturen messen
void temperaturen_messen() {

  DS18B20.requestTemperatures();       // send the command to get temperatures
  tempC = DS18B20.getTempCByIndex(0);  // read temperature in °C

  Serial.print("Temperature : ");
  Serial.print(tempC);    // print the temperature in °C
  Serial.print("°C");

}

/////////////////////////////////////////////////////////////////////////// LOOP
void loop() {


  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ Temperatur Ofen messen
  if (millis() - previousMillis_temp_messen > interval_temp_messen) {
      previousMillis_temp_messen = millis(); 
      // Prüfen der Panelenspannung
      Serial.println("Temperaturen messen");
      temperaturen_messen();
    }


}

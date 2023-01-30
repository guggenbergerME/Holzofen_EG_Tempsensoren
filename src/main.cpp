#include <Arduino.h>
#include <PubSubClient.h>  
#include "WiFi.h"

/////////////////////////////////////////////////////////////////////////// mqtt variable
char msgToPublish[60];
char stgFromFloat[10];

//SCL an GPIO 22 - SDA an GPIO 21

/////////////////////////////////////////////////////////////////////////// Pin Impulsgeber deklarieren
int impulsgeberPIN =  13; // 0,5 Watt pro Impuls
int pulseCount;

/////////////////////////////////////////////////////////////////////////// Funktionsprototypen
void loop                       ();
void wifi_setup                 ();
void callback                   (char* topic, byte* payload, unsigned int length);
void reconnect                  ();
void impulse_auswerten          ();
void ImpulsZaehlen              ();


/////////////////////////////////////////////////////////////////////////// Schleifen verwalten
unsigned long previousMillis_messen = 0; // Spannung Messen
unsigned long interval_messen = 60000; 

/////////////////////////////////////////////////////////////////////////// Kartendaten 
const char* kartenID = "Solarmodul_001_Watt";

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
IPAddress local_IP(192, 168, 13, 52);
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

/////////////////////////////////////////////////////////////////////////// Messen Spannung
void impulse_auswerten() {

  // MQTT Server kontaktieren
  if (!client.connected()) {
  reconnect();
  }
  client.loop();

  // Spannung über Spannungsteiler messen. Maximal U max_V_solar
  Serial.println(pulseCount);

  // Leistung berechnen
  float leistung = pulseCount * 0.5;
  Serial.print("Gemessene Leistung in Watt -> ");
  Serial.println(leistung);

  // mqtt senden
      // mqtt Datensatz senden
    dtostrf(leistung, 4, 2, stgFromFloat);
    sprintf(msgToPublish, "%s", stgFromFloat);
    client.publish("Solarpanel/001/watt", msgToPublish);

  // Pulse wieder löschen
  pulseCount = 0;

}

/////////////////////////////////////////////////////////////////////////// ImpulsZaehlen - INTRRUP
void ImpulsZaehlen() {
  // Impulse Hochzählen
  pulseCount++;

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

// Pin deklarieren
// Panel senkrecht init
pinMode(impulsgeberPIN, INPUT);
attachInterrupt(impulsgeberPIN, ImpulsZaehlen, FALLING);
 
}

/////////////////////////////////////////////////////////////////////////// LOOP
void loop() {



  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ Spannung messen
  if (millis() - previousMillis_messen > interval_messen) {
      previousMillis_messen = millis(); 
      // Prüfen der Panelenspannung
      Serial.println("Impulse auslesen");
      impulse_auswerten();
    }


delay(500);
}

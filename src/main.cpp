#include <Arduino.h>
#include <PubSubClient.h>  
#include "WiFi.h"
#include <OneWire.h>
#include <DallasTemperature.h>


/////////////////////////////////////////////////////////////////////////// Funktionsprototypen
void loop                             ();
void wifi_setup                       ();
void callback                         (char* topic, byte* payload, unsigned int length);
void reconnect                        ();
void temperaturen_messen              ();

/////////////////////////////////////////////////////////////////////////// DS18b20
#define SENSOR_PIN  21 // ESP32 Pin OneWire
float tempC; // temperature in Celsius
char buffer1[10];

OneWire oneWire(SENSOR_PIN);
DallasTemperature DS18B20(&oneWire);

/////////////////////////////////////////////////////////////////////////// Temp. Sensor Var
float wert_temp_sensor_wassertasche;
float wert_temp_sensor_vorlauf;
float wert_temp_sensor_ruecklauf;

/////////////////////////////////////////////////////////////////////////// Sensoradressen onewire
DeviceAddress adr_temp_sensor_wassertasche  = { 0x28, 0x3B, 0xED, 0x81, 0xE3, 0xDD, 0x3C, 0x84 }; 
DeviceAddress adr_temp_sensor_vorlauf       = { 0x28, 0xB7, 0xB3, 0x81, 0xE3, 0x92, 0x3C, 0xA7 };
DeviceAddress adr_temp_sensor_ruecklauf     = { 0x28, 0xA9, 0xDB, 0x81, 0xE3, 0x16, 0x3C, 0xD4 };

/////////////////////////////////////////////////////////////////////////// Schleifen verwalten
unsigned long previousMillis_temp_messen = 0; // Spannung Messen
unsigned long interval_temp_messen = 5000; 

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

}

/////////////////////////////////////////////////////////////////////////// Temperaturen messen
void temperaturen_messen() {

  DS18B20.requestTemperatures();       // send the command to get temperatures

  wert_temp_sensor_wassertasche = DS18B20.getTempC(adr_temp_sensor_wassertasche); 
   if ((wert_temp_sensor_wassertasche == -127)||(wert_temp_sensor_wassertasche == 85))  { 
     Serial.println("Sensor wert_temp_sensor_wassertasche : FEHLER");
     } 
    else 
        { 
          dtostrf(wert_temp_sensor_wassertasche,2, 1, buffer1); 
          Serial.println("Sensor wert_temp_sensor_wassertasche : ");
          Serial.println(buffer1);    
          client.publish("Heizung/HolzofenEG/wassertasche", buffer1);     
        }


  wert_temp_sensor_vorlauf  = DS18B20.getTempC(adr_temp_sensor_vorlauf); 
   if ((wert_temp_sensor_vorlauf == -127)||(wert_temp_sensor_vorlauf == 85))  { 
     Serial.println("Sensor wert_temp_sensor_vorlauf : FEHLER");
     } 
    else 
        { 
          dtostrf(wert_temp_sensor_vorlauf,2, 1, buffer1); 
          Serial.println("Sensor wert_temp_sensor_vorlauf : ");
          Serial.println(buffer1);    
          client.publish("Heizung/HolzofenEG/vorlauf", buffer1);     
        }


  wert_temp_sensor_ruecklauf = DS18B20.getTempC(adr_temp_sensor_ruecklauf); 
   if ((wert_temp_sensor_ruecklauf == -127)||(wert_temp_sensor_ruecklauf== 85))  { 
     Serial.println("Sensor wert_temp_sensor_ruecklauf : FEHLER");
     } 
    else 
        { 
          dtostrf(wert_temp_sensor_ruecklauf,2, 1, buffer1); 
          Serial.println("Sensor wert_temp_sensor_ruecklauf : ");
          Serial.println(buffer1);    
          client.publish("Heizung/HolzofenEG/ruecklauf", buffer1);     
        }



}

/////////////////////////////////////////////////////////////////////////// LOOP
void loop() {


    // mqtt Daten senden     
  if (!client.connected()) {
      reconnect();
    }
    client.loop();  


  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ Temperatur Ofen messen
  if (millis() - previousMillis_temp_messen > interval_temp_messen) {
      previousMillis_temp_messen = millis(); 
      // Prüfen der Panelenspannung
      Serial.println(">>LOOP<< Temperaturen messen");
      temperaturen_messen();
    }


}

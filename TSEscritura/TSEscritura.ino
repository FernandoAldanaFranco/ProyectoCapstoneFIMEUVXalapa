//Proyecto
//Dr. Fernando Aldana Franco, Dr. José Gustavo Leyva Retureta, Dr. Carlos Alberto Mora Barradas.
//Junio 2022
//Universidad Veracruzana.
//Sensores y actuadores
//1 de presión
//ThingSpeak
#include <WiFi.h>
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros

//Wifi
char ssid[] = "D-Link_DAP-1360";      // your network SSID (name)
char pass[] = "12345678Fluidos";   // your network password


int status = WL_IDLE_STATUS;
//WiFiServer server(80);
WiFiClient  client;
String myStatus = "";

//ThingSpeak Write
unsigned long myChannelNumber = 1818373;
const char * myWriteAPIKey = "FOOY9V262106FMHL";

//Sensor presión
float A6Value, kPa;

void setup() {
  
  Serial.begin(9600);
  //Conecta a WIFI
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv != "1.1.0") {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  //server.begin();
  //Coencta a ThingSpeak
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  // you're connected now, so print out the status:
  printWifiStatus();
}

void loop() {
  sensor_presion();
  ThingSpeak.setField(1, kPa);
  // write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
  
  delay(2000); // Wait 1 seconds to update the channel again
  
}


//Función de WIFI
void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

//Función de sensado y acondocionamiento de señal sensores presión
void sensor_presion(void){
  int i;
  A6Value = analogRead(0);
  kPa= (A6Value*(0.00488)/(0.022)+20);  
  Serial.println();
  Serial.print("Sensor "); 
  Serial.print(" ,presión: "); 
  Serial.print(kPa);
}

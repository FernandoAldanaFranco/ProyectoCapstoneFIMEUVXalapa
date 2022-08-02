//Proyecto
//Dr. Fernando Aldana Franco, Dr. José Gustavo Leyva Retureta, Dr. Carlos Alberto Mora Barradas.
//Junio 2022
//Universidad Veracruzana.
//ThingSpeak
#include <WiFi.h>
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros

//Wifi
char ssid[] = "D-Link_DAP-1360";      // your network SSID (name)
char pass[] = "12345678Fluidos";   // your network password


int status = WL_IDLE_STATUS;

WiFiClient  client;
String myStatus = "";



//Think Speak Read
unsigned long myChannelNumberRead = 1818373;
const char * myCounterReadAPIKey = "J236ZIAS1P2CL2D9";
unsigned int counterFieldNumber = 1;  

void setup() {
  pinMode(13, OUTPUT);//Define el pin 13 como salida  
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
  
  actuacion();
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


void actuacion(void){
  int statusCode = 0, indice=0;
  digitalWrite(13, LOW);//Apaga el led
  
  float Resultado = ThingSpeak.readFloatField(myChannelNumberRead, counterFieldNumber, myCounterReadAPIKey);  //Lee primer campo
  Serial.println();
  Serial.println(Resultado);
   // Check the status of the read operation to see if it was successful
  statusCode = ThingSpeak.getLastReadStatus();
  if(statusCode == 200){
    Serial.println("Lectura realizada Relevador 1");
 }
 else{
    Serial.println("Problem reading channel Relevador 1. HTTP error code " + String(statusCode)); 
    Resultado=0;
  }
  statusCode = 0;
  
  //Verifica como activar el led 13
  if(Resultado>10)digitalWrite(13, HIGH);//prende el led
  else digitalWrite(13, LOW);//Apaga el led
   
}

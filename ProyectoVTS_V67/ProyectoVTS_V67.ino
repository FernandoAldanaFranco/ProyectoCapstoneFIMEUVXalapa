//Proyecto
//Dr. Fernando Aldana Franco, Dr. José Gustavo Leyva Retureta, Dr. Carlos Alberto Mora Barradas.
//Junio 2022
//Universidad Veracruzana.
//Sensores y actuadores
//5 de presión
//2 caudalímetros
//2 electroválvulas
//ThingSpeak
#include <WiFi.h>
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros

//Wifi
//Wifi
char ssid[] = "D-Link_DAP-1360";      // your network SSID (name)
char pass[] = "12345678Fluidos";   // your network password

//char ssid[] = "INFINITUMA30D";      // your network SSID (name)
//char pass[] = "Hf5Cy6Jg6a";   // your network password
int status = WL_IDLE_STATUS;
//WiFiServer server(80);
WiFiClient  client;
String myStatus = "";

//ThingSpeak Write
unsigned long myChannelNumber = 1780484;
const char * myWriteAPIKey = "A0UDA6NTNMRGTI7I";


//Think Speak Read
unsigned long myChannelNumberRead = 1784069;
const char * myCounterReadAPIKey = "DFRCDBAFB31MP1TK";
unsigned int counterFieldNumber = 1; 

//actuadores
#define RELEA 14
#define RELEB 15
const int TIEMPO_DE_LLENADO = 2000;
const int TIEMPO_ESPERA = 100;

//Sensores de flujo
volatile int NumPulsosA; //variable para la cantidad de pulsos recibidos
volatile int NumPulsosB; //variable para la cantidad de pulsos recibidos
int frecuenciaA, frecuenciaB;
int PinSensorA = 2;    //Sensor conectado en el pin 2
int PinSensorB = 3;    //Sensor conectado en el pin 2
float factor_conversion=7.5; //para convertir de frecuencia a caudal
float caudal_L_m[2];
float caudal_L_h[2];

//Sensor presión
float A6Value[4], kPa[4];




void setup() {
  
  
  pinMode(PinSensorA, INPUT); 
  pinMode(PinSensorB, INPUT); 
  attachInterrupt(digitalPinToInterrupt(2),ContarPulsos1,RISING); //(Interrupcion 0(Pin2),funcion,Flanco de subida)
  attachInterrupt(digitalPinToInterrupt(3),ContarPulsos2,RISING); //(Interrupcion 0(Pin3),funcion,Flanco de subida)
  // put your setup code here, to run once:
  pinMode(RELEA, OUTPUT);//Define el pin RELE como salida
  pinMode(RELEB, OUTPUT);//Define el pin RELE como salida
  digitalWrite(RELEA, HIGH);//Rele inicia apagado
  digitalWrite(RELEB, HIGH);//Rele inicia apagado
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
  digitalWrite(RELEA, HIGH);//Rele inicia apagado
  digitalWrite(RELEB, HIGH);//Rele inicia apagado
  delay(10);
  sensor_caudal();
  sensor_presion();
  actuacion();
  ThingSpeak.setField(1, caudal_L_m[0]);
  ThingSpeak.setField(2, caudal_L_h[0]);
  ThingSpeak.setField(3, caudal_L_m[1]);
  ThingSpeak.setField(4, caudal_L_h[1]);
  ThingSpeak.setField(5, kPa[0]);
  ThingSpeak.setField(6, kPa[1]);
  ThingSpeak.setField(7, kPa[2]);
  ThingSpeak.setField(8, kPa[3]);
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

//---Función que se ejecuta en interrupción sensor flujo A---------------
void ContarPulsos1 (void)
{ 
  NumPulsosA++;  //incrementamos la variable de pulsos
  //timeCounter = millis();
}

//---Función que se ejecuta en interrupción sensor flujo B---------------
void ContarPulsos2 (void)
{ 
  NumPulsosB++;  //incrementamos la variable de pulsos
  //timeCounter = millis();
}
//---Función para obtener frecuencia de los pulsos sensor flujo--------
void ObtenerFrecuencia() 
{  
  NumPulsosA = 0;   //Ponemos a 0 el número de pulsos
  NumPulsosB = 0;   //Ponemos a 0 el número de pulsos
  //interrupts();    //Habilitamos las interrupciones
  delay(1000);   //muestra de 1 segundo
  //noInterrupts(); //Desabilitamos las interrupciones
  frecuenciaA=NumPulsosA; //Hz(pulsos por segundo)
  frecuenciaB=NumPulsosB; //Hz(pulsos por segundo)
}
//Función de sensado y acondocionamiento de señal sensores flujo
void sensor_caudal(void){
  ObtenerFrecuencia(); //obtenemos la Frecuencia de los pulsos en Hz
  caudal_L_m[0]=frecuenciaA/factor_conversion; //calculamos el caudal en L/m
  caudal_L_m[1]=frecuenciaB/factor_conversion; //calculamos el caudal en L/m
  caudal_L_h[0]=caudal_L_m[0]*60; //calculamos el caudal en L/h
  caudal_L_h[1]=caudal_L_m[1]*60; //calculamos el caudal en L/h
  //-----Enviamos por el puerto serie---------------

  Serial.println();
  Serial.print (caudal_L_m[0],3); 
  Serial.print (" L/m\t, ");
  Serial.println();
  Serial.print (caudal_L_m[1],3); 
  Serial.print (" L/m\t"); 
  Serial.println();
  Serial.println (caudal_L_h[0],3); 
  Serial.print ("L/h, "); 
  Serial.println();
  Serial.print (caudal_L_h[1],3); 
  Serial.print ("L/h"); 
}


//Función de sensado y acondocionamiento de señal sensores presión
void sensor_presion(void){
  int i;
  for(i=0;i<4;i++){
      A6Value[i] = analogRead(i);
      kPa[i] = (A6Value[i]*(0.00488)/(0.022)+20);  
      Serial.println();
      Serial.print("Sensor "); 
      Serial.print(i);
      Serial.print(" ,presion: "); 
      Serial.print(kPa[i]);
    }
}

void actuacion(void){
  int statusCode = 0;
  float Resultado = ThingSpeak.readFloatField(myChannelNumberRead, counterFieldNumber, myCounterReadAPIKey);  
  Serial.println(Resultado);
   // Check the status of the read operation to see if it was successful
  statusCode = ThingSpeak.getLastReadStatus();
  if(statusCode == 200){
    Serial.println("Lectura realizada");
    if(Resultado==1){
      Serial.println("Apagado");
      digitalWrite(RELEA, HIGH);//Enciende el rele
      digitalWrite(RELEB, HIGH);//Enciende el rele
      delay(TIEMPO_DE_LLENADO);//Espera el tiempo de llenado 
      Serial.println("Encendido");
      digitalWrite(RELEA, LOW);//Apaga el rele
      digitalWrite(RELEB, LOW);//Apaga el rele
      delay(TIEMPO_ESPERA);//Tiempo de espera antes de repetir el proceso
      Serial.println("*******************Lectura = 1++++++++++++++" );
   }
   else{
    Serial.println("Sin accion*******");
    digitalWrite(RELEA, HIGH);//Enciende el rele
    digitalWrite(RELEB, HIGH);//Enciende el rele
    delay(TIEMPO_ESPERA);//Tiempo de espera antes de repetir el proceso
   }
 }
 else{
    Serial.println("Problem reading channel. HTTP error code " + String(statusCode)); 
    digitalWrite(RELEA, HIGH);//Enciende el rele
    digitalWrite(RELEB, HIGH);//Enciende el rele
    delay(TIEMPO_ESPERA);//Tiempo de espera antes de repetir el proceso
  }
  
}

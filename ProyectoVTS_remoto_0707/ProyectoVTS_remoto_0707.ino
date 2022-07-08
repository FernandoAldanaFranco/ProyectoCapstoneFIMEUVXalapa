//Proyecto Capsotone.
//Dr. Fernando Aldana Franco, Dr. José Gustavo Leyva Retureta, Dr. Carlos Alberto Mora Barradas.
//Julio 2022.
//Facultad de Ingeniería Mecánica y Eléctrica - Xalapa. Universidad Veracruzana.
//Sensores y actuadores
//2 deepswitch
// Emplea servicio ThingSpeak para conexión.
//Tarjeta ESP8266.
/*Descripción: Este programa permite cambiar el estado de dos campos en un canal de ThingSpeak mediante dos switch conectados a dos pines digitales de la tarjeta ESP8266. La comunicación
es por medio de WIFI. Lo que premite funcionar como un control remoto WIFI para el sistema experimental del fenómeno de golpe de ariete. Estas señales controlan el estado de las
electroválvulas.*/

#include <ESP8266WiFi.h> //Biblioteca para el ESP8266
#include "ThingSpeak.h" // Biblioteca para ThingSpeak de Mathworks basado en MQTT
#ifndef STASSID

/* Definiciones WIFI*/
//datos de la red
#define STASSID "D-Link_DAP-1360" 
#define STAPSK  "12345678Fluidos"
#endif

const char* ssid     = STASSID;
const char* password = STAPSK;
WiFiClient  client;//Para establecer la tarjeta como cliente

/* Definición de los botones de entrada*/
const int buttonPinA = 4; //Canal digital 4
const int buttonPinB = 5; //Canal digital 5
int buttonStateA = 0; //Guarda el estado de los botones..
int buttonStateB = 0; //Por default los pone en bajo

/* Definiciones ThnikSpeak*/
//Canal de escritura de ThingSpeak
unsigned long myChannelNumber = 1784069;  //ID del canal de escritura
const char * myWriteAPIKey = "1PNFWJX18P0JE3LQ"; //APIKey de escritura del canal

/* Función de configuración de la tarjeta*/
void setup() {
  pinMode(buttonPinA, INPUT); //Pines digitales para los switch de control
  pinMode(buttonPinB, INPUT);
  
  Serial.begin(115200); //Canal serial inicializado a 115200bps

  //Inicia protocolo de conexión
  Serial.println();
  Serial.println();
  Serial.print("Conectando a: ");
  Serial.println(ssid);

  //Conecta a red wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  //Verifica si ya fue conectado
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  //Imprime el SSID de la red conectada y la IP asignada
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  //Coencta a ThingSpeak como cliente
  ThingSpeak.begin(client);
}

/* Función principal del código*/
void loop() {
  sensar(); //Función para sensar el estado de los switch.
  escribe_canal(); //Funciuón que escribe al canal de ThingSpeak.
  delay(1000); //Espera un segundo antes de comenzar un nuevo ciclo de funcionamiento.
}

/* Función que lee el estado de los switch*/
void sensar(void){
  buttonStateA = digitalRead(buttonPinA); //Estado del switch 1 
  buttonStateB = digitalRead(buttonPinB); //Estado del switch 2 
}

/* Función que escribe al canal de thingspeak para controlar los relevadores*/
void escribe_canal(void){
  //Actualiza los campos del canal de escritura.
  ThingSpeak.setField(1, buttonStateA); //Switch 1.
  ThingSpeak.setField(2, buttonStateB); //Switch 2.
  //Escribe al canal correspondiente
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  //Verifica si la escritura fue correcta
  if(x == 200){
    Serial.println("Channel update successful."); //En caso de actualización, envía mensaje que lo confirma.
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x)); //En caso contrario, envia una alerta.
  } 
}

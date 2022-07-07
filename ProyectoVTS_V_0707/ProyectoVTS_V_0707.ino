//Proyecto Capstone.
//Dr. Fernando Aldana Franco, Dr. José Gustavo Leyva Retureta, Dr. Carlos Alberto Mora Barradas.
//Julio 2022.
//Facultad de Ingeniería Mecánica y Eléctrica - Xalapa. Universidad Veracruzana.
//Sensores y actuadores
//4 de presión
//2 caudalímetros
//2 electroválvulas
// Emplea servicio ThingSpeak para conexión.
//Tarjeta Arduino ADK y shiel wifi.
/*Descripción: Este programa permite adquirir la señal de 4 sensores de presión MPX10DP, 2 sensores de flujo YF-S201 instalados en dos tuberías de PVC para el estudio del fenómeno
del golpe de ariete. Utiliza dos relevadores JQC3F para controlar el encendido de dos electroválvulas. Se comunica vía wifi a dos canales de thingspeak. El primer canal sirve para
visualizar las variables sensadas.El segundo canal permite activar los relevadores en función de la actualización de dos campos. Esta actualización puede realizarse mediante el 
una dirección web o por medio de otra tarjeta*/

#include <WiFi.h> //Biblioteca para el shield wifi
#include "ThingSpeak.h" // Biblioteca para ThingSpeak de Mathworks basado en MQTT

/* Definiciones WIFI*/
//datos de la red
char ssid[] = "D-Link_DAP-1360";      // SSID de la red
char pass[] = "12345678Fluidos";   // Password

int status = WL_IDLE_STATUS; //Variable apra el estado de la red

WiFiClient  client; //Para establecer la tarjeta como cliente
String myStatus = ""; //Para guardar el estado de conectividad

/* Definiciones ThnikSpeak*/
//Canal de escritura de ThingSpeak
unsigned long myChannelNumber = 1780484; //ID del canal de escritura
const char * myWriteAPIKey = "A0UDA6NTNMRGTI7I"; //APIKey de escritura del canal

//Canal de lectura de ThinkSpeak
unsigned long myChannelNumberRead = 1784069; //ID del canal de lectura
const char * myCounterReadAPIKey = "DFRCDBAFB31MP1TK"; //APIKey de lectura
unsigned int counterFieldNumber = 1; //Campos a leer en el canal
unsigned int counterFieldNumber2 = 2; 

/* Definiciones de actuadores*/
#define RELEA 14 //Pines 14 y 15 para el control de los relevadores
#define RELEB 15 
const int TIEMPO_DE_LLENADO = 2000; //Tiempo de actuación de las electroválvulas
const int TIEMPO_ESPERA = 100; //Tiempo de espera del sistema de actuación

/* Definiciones sensores de flujo de caudal*/
volatile int NumPulsosA; //variable para la cantidad de pulsos recibidos sensor 1. Volatile porque se emplean con interrupciones.
volatile int NumPulsosB; //variable para la cantidad de pulsos recibidos sensor 2.
int frecuenciaA, frecuenciaB; //Variables de frecuencia de los sensores.
int PinSensorA = 2;    //Sensor conectado en el pin 2
int PinSensorB = 3;    //Sensor conectado en el pin 3
float factor_conversion=7.5; //para convertir de frecuencia a caudal
float caudal_L_m[2]; //Arreglo para guardar el flujo en litros por minuto.
float caudal_L_h[2]; //Arreglo para guardar el flujo en litros por hora.

/* Definiciones sensores de presión en las tuberías*/
float A6Value[5], kPa[5];

/* Función de configuración de la tarjeta*/
void setup() {
  pinMode(PinSensorA, INPUT); //Pines digitales para sensores de caudal
  pinMode(PinSensorB, INPUT); //en formato de entrada
  attachInterrupt(digitalPinToInterrupt(2),ContarPulsos1,RISING); //(Interrupcion 0(Pin2),funcion,Flanco de subida)
  attachInterrupt(digitalPinToInterrupt(3),ContarPulsos2,RISING); //(Interrupcion 0(Pin3),funcion,Flanco de subida)
  pinMode(RELEA, OUTPUT);//Define el pin relevador 1 como salida
  pinMode(RELEB, OUTPUT);//Define el pin relevador 2 como salida
  digitalWrite(RELEA, HIGH);//Apaga los relevadores.
  digitalWrite(RELEB, HIGH);//Se activan en forma negada.
  Serial.begin(9600); //Comienza comunicación a 9600bps
  /* Conexión a WIFI*/
  // Verifica si el shield esta conectado
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield no conetcado");//En caso contrario, imprime una alerta.
    while (true);
  }
  //Verifica si el firmware del shiel está actualizado.
  String fv = WiFi.firmwareVersion();
  if (fv != "1.1.0") {
    Serial.println("Actualizar el firmware del shield");//En caso contrario, imprime una alerta.
  }

  //Intenta conectarse a la red WIFI.
  while (status != WL_CONNECTED) {
    Serial.print("Intentando conectarse a la red con SSID: ");
    Serial.println(ssid);
    //Esta función es la que conecta con WIFI.
    status = WiFi.begin(ssid, pass);

    //Espera 10 segundos para establecer la conexicón
    delay(10000);
  }

  //Coencta a ThingSpeak como cliente
  ThingSpeak.begin(client);
  //Imprime el status de conexión a internet.
  printWifiStatus();
  
}

/* Función principal del código*/
void loop() {  
  sensor_caudal(); //lectura de flujo de caudal.
  sensor_presion(); //lectura de sensores de presión.
  actualiza_thingspeak(); //Actualiza los componentes del canal de escritura de ThingSpeak.
  actuacion(); //Función que regula la actuación sobre los relevadores.
  delay(2000); // Espera 2 segundos antes de realizar el proceso nuevamente.
  
}


/* Función de estadísticas de wifi. Tomada de los ejemplos de la biblioteca wifi.h*/
void printWifiStatus() {
  //Imprimer el SSID de la red a la que se conecta el shield.
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  //Imprimer la IP.
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  //Imprime la intensidad de la señal.
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

/* Funciones de los sensores de flujo de caudal*/

/*Función que se ejecuta al generarse una interrupción del sensor flujo A*/
void ContarPulsos1 (void)
{ 
  NumPulsosA++;  //Se incrementa el contador de pulsos.
}

/*Función que se ejecuta al generarse una interrupción del sensor flujo B*/
void ContarPulsos2 (void)
{ 
  NumPulsosB++;  //Se incrementa el contador de pulsos.
}

/*---Función para obtener la frecuencia de los pulsos en ambos sensores de flujo*/
void ObtenerFrecuencia() 
{  
  NumPulsosA = 0;   //Ponemos a 0 el número de pulsos
  NumPulsosB = 0;   //Ponemos a 0 el número de pulsos
  delay(1000);   //Tiempo de muestreo de 1 segundo
  frecuenciaA=NumPulsosA/1; //Pulsos por segundo del sensor 1.
  frecuenciaB=NumPulsosB/1; //Pulsos por segundo del sensor 2.
}
/*Función de sensado y acondocionamiento de señal sensores flujo*/
//Código adaptado de: https://naylampmechatronics.com/blog/47_tutorial-sensor-de-flujo-de-agua.html
void sensor_caudal(void){
  ObtenerFrecuencia(); //Obtiene la frecuencia de los pulsos en una ventana de muestreo de 1 segundo.
  caudal_L_m[0]=frecuenciaA/factor_conversion; //Calcula el caudal en L/m en sensor 1.
  caudal_L_m[1]=frecuenciaB/factor_conversion; //Calcula el caudal en L/m en sensor 2.
  caudal_L_h[0]=caudal_L_m[0]*60; //Calcula el caudal en L/h en sensor 1.
  caudal_L_h[1]=caudal_L_m[1]*60; //Calcula el caudal en L/h en sensor 2.
  //Se impruimen esos datos en el puerto serial
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

/*Función de sensado y acondocionamiento de señal sensores presión*/
void sensor_presion(void){
  int i; //Contador para recorrer el arrgelo
  //accede a la lectura de los cuatro sensores empleados
  for(i=0;i<4;i++){
      A6Value[i] = analogRead(i); //Lectura del canal analógico i-esimo
      kPa[i] = (A6Value[i]*(0.00488)/(0.022)+20);  //Convierte la lectura del ADC a kPa.
      Serial.println(); //Imprime por el canal serial el resultado de cada sensor en kPa.
      Serial.print("Sensor "); 
      Serial.print(i);
      Serial.print(" ,presion: "); 
      Serial.print(kPa[i]);
    }
}

/*Función de actuación del sistema*/
void actuacion(void){
  int statusCode = 0, indice=0; //statusCode sirve para verificar el estado de actualización del canal, indice sirve para seleccionar la acción a realizar.
  digitalWrite(RELEA, HIGH);//Asegura que los relevadores estén apagados.
  digitalWrite(RELEB, HIGH);
  delay(TIEMPO_ESPERA);//Espera 100mS antes de actualizar el sistema.
  float Resultado = ThingSpeak.readFloatField(myChannelNumberRead, counterFieldNumber, myCounterReadAPIKey);  //Lee el primer campo del canal correspondiente
  Serial.println();
  Serial.println(Resultado); //imprime el resultado de la lectura
  //Verifica si la lectrua fue realizada correctamente
  statusCode = ThingSpeak.getLastReadStatus();
  if(statusCode == 200){
    Serial.println("Lectura realizada Relevador 1"); //Si la lectura fue realizada correctamente, imprime un mesaje que lo confirma
 }
 else{
    Serial.println("Problema al leer el canal asociado al Relevador 1. HTTP error code " + String(statusCode));  //En caso contrario, imprime una alterta 
    Resultado=0; //Por default se toma este estado como 0.
  }
  statusCode = 0;//Reinicia la variable de verificación
  float Resultado2 = ThingSpeak.readFloatField(myChannelNumberRead, counterFieldNumber2, myCounterReadAPIKey); //Lee el segundo campo del canal correspondiente
  Serial.println();
  Serial.println(Resultado2);//imprime el resultado de la lectura
  //Verifica si la lectrua fue realizada correctamente
  statusCode = ThingSpeak.getLastReadStatus();
  if(statusCode == 200){
    Serial.println("Lectura realizada Relevador 2"); //Si la lectura fue realizada correctamente, imprime un mesaje que lo confirma
 }
 else{
    Serial.println("Problem reading channel Relevador 2. HTTP error code " + String(statusCode)); //En caso contrario, imprime una alterta 
    Resultado2=0; //Por default se toma este estado como 0.
  }
  //Verifica cual relevador debe activar
  if(Resultado==1){
    if(Resultado2==1)indice=1; //Ambos relevadores activos.
    else indice=2; //Relevador 1 activo, relevador 2 apagado.
   }
   else{
    if(Resultado2==1)indice=3; //Relevador 2 activo, relevador 1 apagado.
    else indice=4; //Ambos relevadores apagados.
   }
   accion(indice); //Función que envia la señal a los relevadores.
}

/*Función que envia la señal de actuación a los relevadores*/
void accion(int seleccion){
  //Emplea un switch para la selección de la acción.
  switch(seleccion){
    case 1:
      Serial.println("*******************Relevador 1 activo *************" );
      Serial.println("*******************Relevador 2 activo *************" );
      digitalWrite(RELEA, LOW);//Enciende el rele 1
      digitalWrite(RELEB, LOW);//Enciende el rele 2
      delay(TIEMPO_DE_LLENADO);//Espera el tiempo de funcionamiento.
      digitalWrite(RELEA, HIGH);//Apaga el rele 1
      digitalWrite(RELEB, HIGH);//Apaga el rele 2
    break;
    case 2:
      Serial.println("*******************Relevador 1 activo *************" );
      Serial.println("*******************Relevador 2 apagado *************" );
      digitalWrite(RELEA, LOW);//Enciende el rele 1
      delay(TIEMPO_DE_LLENADO);//Espera el tiempo de funcionamiento. 
      digitalWrite(RELEA, HIGH);//Apaga el rele 1
    break;
    case 3:
      Serial.println("*******************Relevador 1 apagado *************" );
      Serial.println("*******************Relevador 2 activo *************" );
      digitalWrite(RELEB, LOW);//Enciende el rele 2
      delay(TIEMPO_DE_LLENADO);//Espera el tiempo de funcionamiento.
      digitalWrite(RELEB, HIGH);//Apaga el rele 2
    break;
    case 4:
      Serial.println("*******************Relevador 1 apagado *************" );
      Serial.println("*******************Relevador 2 apagado *************" );    
    break;
    default:
      Serial.println("*******************Error en la secuencia de acción *************" ); //Para casos no previstos.
    break;
  }
}

/*Función que escribe la lectura de los sensores al canal correspondiente*/
void actualiza_thingspeak(void){
  //Actualiza los campos del canal de escritura.
  ThingSpeak.setField(1, caudal_L_m[0]); //Sensor de flujo 1 en L/m.
  ThingSpeak.setField(2, caudal_L_h[0]); //Sensor de flujo 1 en L/h.
  ThingSpeak.setField(3, caudal_L_m[1]); //Sensor de flujo 2 en L/m.
  ThingSpeak.setField(4, caudal_L_h[1]); //Sensor de flujo 2 en L/h.
  ThingSpeak.setField(5, kPa[0]); //Sensor de presión 1.
  ThingSpeak.setField(6, kPa[1]); //Sensor de presión 2.
  ThingSpeak.setField(7, kPa[2]); //Sensor de presión 3.
  ThingSpeak.setField(8, kPa[3]); //Sensor de presión 4.
  //Escribe al canal correspondiente
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  //Verifica si la escritura fue correcta
  if(x == 200){
    Serial.println("Canal de escritura actualizado."); //En caso de actualización, envía mensaje que lo confirma.
  }
  else{
    Serial.println("Problema al actualizar el canal. HTTP error code " + String(x)); //En caso contrario, envia una alerta.
  }  
}


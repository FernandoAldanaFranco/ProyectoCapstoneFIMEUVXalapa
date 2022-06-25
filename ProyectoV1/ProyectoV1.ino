//Proyecto
//Dr. Fernando Aldana Franco, Dr. José Gustavo Leyva Retureta, Dr. Carlos Alberto Mora Barradas.
//Junio 2022
//Universidad Veracruzana.
//Sensores y actuadores
//5 de presión
//2 caudalímetros
//2 electroválvulas
//Relevadores
#include <WiFi.h>

//Wifi
char ssid[] = "INFINITUM2259_2.4";      // your network SSID (name)
char pass[] = "wUCz9Hn1ea";   // your network password
int status = WL_IDLE_STATUS;
WiFiServer server(80);

//actuadores
#define RELEA 9
#define RELEB 10
const int TIEMPO_DE_LLENADO = 2000;
const int TIEMPO_ESPERA = 3000;

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
float A6Value[5], kPa[5];

void setup() {
  // put your setup code here, to run once:
  pinMode(RELEA, OUTPUT);//Define el pin RELE como salida
  digitalWrite(RELEA, HIGH);//Rele inicia apagado
  pinMode(RELEB, OUTPUT);//Define el pin RELE como salida
  digitalWrite(RELEB, HIGH);//Rele inicia apagado
  pinMode(PinSensorA, INPUT); 
  pinMode(PinSensorB, INPUT); 
  attachInterrupt(0,ContarPulsos1,RISING); //(Interrupcion 0(Pin2),funcion,Flanco de subida)
  attachInterrupt(1,ContarPulsos2,RISING); //(Interrupcion 0(Pin3),funcion,Flanco de subida)
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
  //Arranca servidor
  server.begin();
  // you're connected now, so print out the status:
  printWifiStatus();
}

void loop() {
  // listen for incoming clients
  WiFiClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          sensor_caudal();
          sensor_presion();
          actuacion();
          client.print("Sensor flujo 1: ");
          client.print(caudal_L_m[0]);
          client.println("<br />");
          client.print("Sensor flujo 2: ");
          client.print(caudal_L_m[1]);
          client.println("<br />");
          client.print("Sensor presión 1: ");
          client.print(kPa[0]);
          client.println("<br />");
          client.print("Sensor presión 2: ");
          client.print(kPa[0]);
          client.println("<br />");
          client.print("Sensor presión 3: ");
          client.print(kPa[0]);
          client.println("<br />");
          client.print("Sensor presión 4: ");
          client.print(kPa[0]);
          client.println("<br />");
          client.print("Sensor presión 5: ");
          client.print(kPa[0]);
          client.println("<br />");
          // output the value of each analog input pin
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);

    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
  
  delay(100);
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
}

//---Función que se ejecuta en interrupción sensor flujo B---------------
void ContarPulsos2 (void)
{ 
  NumPulsosB++;  //incrementamos la variable de pulsos
}
//---Función para obtener frecuencia de los pulsos sensor flujo--------
void ObtenerFrecuencia() 
{  
  NumPulsosA = 0;   //Ponemos a 0 el número de pulsos
  NumPulsosB = 0;   //Ponemos a 0 el número de pulsos
  interrupts();    //Habilitamos las interrupciones
  delay(1000);   //muestra de 1 segundo
  noInterrupts(); //Desabilitamos las interrupciones
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

  Serial.println (caudal_L_m[0],3); 
  Serial.print (" L/m\t, ");
  Serial.print (caudal_L_m[1],3); 
  Serial.print (" L/m\t"); 
  Serial.println (caudal_L_h[0],3); 
  Serial.print ("L/h, "); 
  Serial.print (caudal_L_h[1],3); 
  Serial.print ("L/h"); 
}

//Función de sensado y acondocionamiento de señal sensores presión
void sensor_presion(void){
  int i;
  for(i=0;i<5;i++){
      A6Value[i] = analogRead(i);
      kPa[i] = (A6Value[i]*(0.00488)/(0.022)+20);  
      Serial.println("Sensor "); 
      Serial.print(i);
      Serial.println(",presión: "); 
      Serial.print(kPa[i]);
    }
}


void actuacion(void){
  // put your main code here, to run repeatedly:
  Serial.println("Apagado");
  digitalWrite(RELEA, LOW);//Enciende el rele
  digitalWrite(RELEB, LOW);//Enciende el rele
  delay(TIEMPO_DE_LLENADO);//Espera el tiempo de llenado 
  Serial.println("Encendido");
  digitalWrite(RELEA, HIGH);//Apaga el rele
  digitalWrite(RELEB, HIGH);//Apaga el rele
  delay(TIEMPO_ESPERA);//Tiempo de espera antes de repetir el proceso
}

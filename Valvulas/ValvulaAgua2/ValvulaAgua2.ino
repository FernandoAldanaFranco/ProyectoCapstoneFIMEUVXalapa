//Manejo de relevadores y electroválvulas.
//Dr. Fernando Aldana Franco, Dr. José Gustavo Leyva Retureta, Dr. Carlos Alberto Mora Barradas.
//Julio 2022.
//Facultad de Ingeniería Mecánica y Eléctrica - Xalapa. Universidad Veracruzana.
/*Descripción: Este programa permite electroválvulas mediante relevadores. Lo que permite la apertura y cerrado de la tubería en el proyecto.*/
#define RELE 9 //Pin 9 digital
const int TIEMPO_DE_LLENADO = 2000; //Tiempo de llenado
const int TIEMPO_ESPERA = 3000; //Tiempo de espera

void setup() {
  pinMode(RELE, OUTPUT);//Define el pin RELE como salida
  digitalWrite(RELE, HIGH);//Rele inicia apagado
  Serial.begin(9600); //Inicia canal serial
}

void loop() {
  Serial.println("Encendido"); //Imprime etiqueta de encendido
  digitalWrite(RELE, LOW);//Enciende el rele
  delay(TIEMPO_DE_LLENADO);//Espera el tiempo de llenado 
  Serial.println("Apagado"); //Etiqueta de apagado
  digitalWrite(RELE, HIGH);//Apaga el rele
  delay(TIEMPO_ESPERA);//Tiempo de espera antes de repetir el proceso
}

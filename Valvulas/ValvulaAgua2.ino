#define RELE 9
const int TIEMPO_DE_LLENADO = 2000;
const int TIEMPO_ESPERA = 3000;

void setup() {
  // put your setup code here, to run once:
  pinMode(RELE, OUTPUT);//Define el pin RELE como salida
  digitalWrite(RELE, HIGH);//Rele inicia apagado
    Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Apagado");
  digitalWrite(RELE, LOW);//Enciende el rele
  delay(TIEMPO_DE_LLENADO);//Espera el tiempo de llenado 
  Serial.println("Encendido");
  digitalWrite(RELE, HIGH);//Apaga el rele
  delay(TIEMPO_ESPERA);//Tiempo de espera antes de repetir el proceso
}

//Programa manejo de sensores de presión.
//Dr. Fernando Aldana Franco, Dr. José Gustavo Leyva Retureta, Dr. Carlos Alberto Mora Barradas.
//Julio 2022.
//Facultad de Ingeniería Mecánica y Eléctrica - Xalapa. Universidad Veracruzana.
/*Descripción: Este programa permite cambiar monitorear el canal analógico-digital 0 conectado a un sensor de presión MPX10DP aplicado a agua.*/

float A6Value, kPa; //Variables del sensor. A6Value guarda el valor del ADC. kPa guarda el valor en kPa.

void setup()
{
  Serial.begin(115200); // Abre el canal serial
  Serial.println("Powered By Gus Leyva - v0.1"); 
}

void loop()
{
    A6Value = analogRead(0); //Lee el canal 0 del ADC
    kPa = (A6Value*(0.00488)/(0.022)+20); //Convierte a kPa por aproximación
    Serial.println(kPa); //Imprime su valor
    delay(300); //Espera 300mS antes de una nueva lectura.
}

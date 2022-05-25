float A6Value, kPa;

void setup()
{
  Serial.begin(115200); // Open serial port
  Serial.println("Powered By Gus Leyva - v0.1"); 
  delay(2000);
}

void loop()
{
    A6Value = analogRead(6);
    kPa = (A6Value*(0.00488)/(0.022)+20);
    Serial.println(kPa);
    delay(300);

}

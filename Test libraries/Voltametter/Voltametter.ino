int Vin, temp;
void setup() 
{
  Serial.begin(9600);
}

void loop() 
{
  Vin = analogRead(A0);
  if (Vin != temp)
  {
    Serial.print("I read ");
    Serial.print(Vin);
    Serial.print(" out of 1024 \n");
    temp = Vin;
    delay(1000);
  }

}

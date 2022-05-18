int incomin = 0;
void setup() 
{
  Serial.begin(9600);
}

void loop() 
{
  if (Serial.available()>0)
  {

    Serial.print("Me receive: ");
    while(Serial.available()>0)
    {
      incomin = Serial.read();
      Serial.write(incomin);
    }
    Serial.print("\n");
  }

}

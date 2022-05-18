int a = 0;
int b = 0;
bool initsys = 1;
void setup() 
{
  Serial.begin(9600);

}

void loop() 
{
  //a = PLLCSR;
//  b = PLLFRQ;
  a = 10;
  b = 20;
  if( initsys == 1)
  {
    if (Serial.available()>0)
    {
     initsys = 0;
     while(Serial.available()>0)
      {
        Serial.print("PLLCSR: ");
      }
     while(Serial.available()>0)
      {
        Serial.write(a);
      }
      while(Serial.available()>0)
      {
        Serial.print("\n");
      }
      while(Serial.available()>0)
      {
        Serial.print("PLLFRQ: ");
      }
      while(Serial.available()>0)
      {
        Serial.write(b);
      }
      Serial.print("\n");
    }
  }
}

int VinH, VinL, Vin;
void setup() 
{
  Serial.begin(9600);
  Vin = analogRead(A3);  
  Vin = analogRead(A6);  
//  delay(100);
  ADMUX = 0x54;
//  ADCSRA |= 0x80;
ADCSRA = 0x87;
//  ADCSRB = 0;
//  DIDR0 |= 0x32;
//  DIDR2 = 0x0;  
}

void loop() 
{
  ADCSRA |= 0x40;//  VinL = ADCL;

  delay(1000);
//  VinH = ADCH;
//  Vin = (VinH << 8) + VinL;
Vin = ADC;

//  MUXAD = ADMUX;

  Serial.print("I read ");
  Serial.print(VinH);
  Serial.print(" ");
  Serial.print(VinL);
  Serial.print(" ");
  Serial.print(Vin);
  Serial.print(" out of 1024 \n");

}

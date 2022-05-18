void setup() 
{
  pinMode(9, OUTPUT);
  pinMode(8, OUTPUT);
  ICR1 = 0x0FFF;
  TCCR1A = 0xA2;
  TCCR1B = 0x19;
  OCR1A = 0x07FF;  
  OCR1B = 0x04FF;
}

void loop() 
{
  

}

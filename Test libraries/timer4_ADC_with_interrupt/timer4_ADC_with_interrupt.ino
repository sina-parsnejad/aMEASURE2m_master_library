int VADC, Vin;
int cntr = 0;
bool flagg = 0;
void setup() 
{
  
  noInterrupts ();
  TCCR4B = 0x04;
  TIMSK4 = 0x04;
  TC4H = 0x01;
  OCR4C = 0xF4;
  interrupts ();

  Serial.begin(9600);
  VADC = analogRead(A3);  
  VADC = analogRead(A6);  
  ADMUX = 0xD6; //ADC6 - ADC1
  ADCSRA = 0x87;

  pinMode(9, OUTPUT);
  pinMode(8, OUTPUT);
  ICR1 = 0x0FFF;
  TCCR1A = 0xA2;
  TCCR1B = 0x19;
  OCR1A = 0x07FF;  
  OCR1B = 0x07FF;

  pinMode ( 13, OUTPUT);
}

ISR (TIMER4_OVF_vect)
{
  ADCSRA |= 0x40;//  VinL = ADCL;

  if (OCR1A++ > 4000) OCR1A =0;

  cntr ++;
  
}

void loop() 
{
  if ( ADCSRA & 0x10 )
  {
    VADC = ADC;
    if (VADC > 0x1FF)
    {
      VADC = -( 0x400- VADC);
    }
    Serial.print("I read "); 
    Serial.print(VADC +512);
    Serial.print("\n");
    ADCSRA |= 0x10;

    if (flagg)
    {
      digitalWrite (13, LOW);
      flagg = 0;
    }
    else
    {
      digitalWrite (13, HIGH);
      flagg = 1;
    }
  }
  
  
  

}



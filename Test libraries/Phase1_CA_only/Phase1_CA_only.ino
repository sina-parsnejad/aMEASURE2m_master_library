int Vout0, Vout1;
int cntr = 0;
bool ovf_flagg = 0;
void setup()
{

  noInterrupts ();
  TCCR4B = 0x04;
  TIMSK4 = 0x04;
  TC4H = 0x01;
  OCR4C = 0xF4;


  Serial.begin(9600);
  Vout0 = analogRead(A3);
  Vout1 = analogRead(A2);
  ADMUX = 0xD6; //ADC6 - ADC1
  ADCSRA = 0x87;

  pinMode(9, OUTPUT);
  pinMode(8, OUTPUT);
  ICR1 = 0x0FFF;
  TCCR1A = 0xA2;
  TCCR1B = 0x19;
  OCR1A = 0x07FF;
  OCR1B = 0x07FF;
  interrupts ();
}

ISR (TIMER4_OVF_vect)
{
  ADMUX = 0xD4; //ADC4 - ADC1 VOUT1
  ADCSRA |= 0x40;// convert
  while (!( ADCSRA & 0x10 )) {} // wait for conversion
  Vout0 = ADC;
  ADCSRA |= 0x10;
  ADMUX = 0xD5; //ADC5 - ADC1 VOUT2
  ADCSRA |= 0x40;// convert
  while (!( ADCSRA & 0x10 )) {} // wait for conversion
  Vout1 = ADC;
  ADCSRA |= 0x10;
  if (OCR1A++ > 4000) OCR1A = 0;
  if (OCR1B-- < 100) OCR1B = 4000;
  ovf_flagg = 1;

}

void loop()
{
  if ( ovf_flagg )
  {
    ovf_flagg = 0;
      if (Vout0 > 0x1FF)
  {
    Vout0 = -( 0x400 - Vout0);
  }
    if (Vout1 > 0x1FF)
  {
    Vout1 = -( 0x400 - Vout1);
  }
    Serial.print("DATA\r");
    Serial.print(Vout0);
    Serial.print("0");
    Serial.print("0");
    Serial.print("\r");
    Serial.print("DATA\r");
    Serial.print(Vout1);
    Serial.print("1");
    Serial.print("0");
    Serial.print("\r");
  }

}



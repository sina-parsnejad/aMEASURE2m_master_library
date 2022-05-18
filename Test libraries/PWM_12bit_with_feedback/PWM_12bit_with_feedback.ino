# include "DAC_structs.h"

int Vin4A0 = 0;
int Vin4A1 = 0;
int Verr = 0;
int Vin = 0;
bool que = 0;
int cntr1 = 0;
int cntr2 = 0;
void setup() 
{
  pinMode(9, OUTPUT);
  pinMode(8, OUTPUT);
//  pinMode(13, OUTPUT);  
  ICR1 = 0x0FFF;
  TCCR1A = 0xA2;
  TCCR1B = 0x19;
  OCR1A = 0x07FF;  
  OCR1B = 0x07FF;

  Serial.begin(9600);

 
  
}

void loop() 
{

//  Vin = cntr2;// sin(2*3.1415*cntr/100);
  
//  if (cntr1++ > 9 )
//  {
//    if (cntr2++ >99) cntr2 = -100;
//    cntr1 = 0;
//  }
  
  Vin4A0 = analogRead(A0);
  Vin4A1 = analogRead(A1);
  Verr += Vin4A1 + Vin - Vin4A0;
  

  OCR1A = ((Vin4A1 + Vin) << 2) + ((Vin4A1 + Vin - Vin4A0) << 0) + (Verr >> 8) ;



}




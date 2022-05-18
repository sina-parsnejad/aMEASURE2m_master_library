
#include "structs.h"

ch_params_t params[2]; // all data regarding the channel is recorded in this struct

enum USB_States {  USB_fresh, USB_idn, USB_serial, USB_channel0, USB_channel1, USB_STOP, USB_STREAM, USB_CV, USB_CA, USB_MPS, USB_ARB, USB_SWV, USB_ERROR }; //all usb states fresh is the base state where the loop is idle when no input command is avalible
enum USB_States current_USB_state = USB_fresh ;

uint8_t incomingByte = 0;
char ascii_char;
String cmd = "";
boolean cmd_flag = false;
boolean programming_success = false;

uint8_t input_channel_for_USB = 0;  //records the input channel when it is transmitted through USB
uint8_t input_counter_for_USB = 0;  //counter for USB when input 

bool ovf_flagg =0; //a flag to let the main loop know that a conversion is performed

bool led = 0;

uint16_t ADC_cntr = 0;
bool ADC_flag = 0;

void setup()
{

  noInterrupts (); // no interrrupts during setup

  current_USB_state = USB_fresh;
  
  //timer 4 configuration for 1ms interrups
  TCCR4B = 0x04;
  TIMSK4 = 0x04;
  //TC4H = 0x01;
  //OCR4C = 0xF4;
  TC4H = 0x00;
  OCR4C = 0x64;

  Serial.begin(115200);

  uint16_t dump;
  dump = analogRead(A0); // these are to take care of setting up most of ADC parameters for all theses ports
  dump = analogRead(A1);
  dump = analogRead(A2);
  dump = analogRead(A3);
  dump = analogRead(A6);
  ADMUX = 0xD6; //ADC6 - ADC1
  ADCSRA = 0x87;

  // these are the automatic gain control and PWM GPIO pins
  pinMode(9, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);

  pinMode(12, OUTPUT);


  // set up Timer 1 for PWM through ports 8 and 9
  ICR1 = 0x0FFF;
  TCCR1A = 0xA2;
  TCCR1B = 0x19;
  OCR1A = 0x07FF; // This is PWM 1 value. The PWM is a 12 bit DAC so the max number is 2^12
  OCR1B = 0x07FF; // This is PWM 2 value. The PWM is a 12 bit DAC so the max number is 2^12

  default_params(params); // reset all params to their defaut values

  // type in temp commands here


  
  interrupts (); // enable global interrupts
}

ISR (TIMER4_OVF_vect) // Timer 4 interrup
{
  /*
   * This bloack is the timer4 interrupt which fires everey 1ms and is the main source of timing int the whole system
   * depending on which channel is on, the corresponding ADC and DAC are read
   */
  if (params[0].OP == STREAM)
  {
    OCR1A = (1024 - params[0].DACv) << 2; //implement DAC values. DACv is 10 bits while OCR1A is 12 bits so be ware
  }
  if (params[1].OP == STREAM)
  {
    OCR1B = (1024 -  params[1].DACv) << 2;
  }
  if (params[0].OP == STREAM)
  {
    run_ch (&params[0]);  //generate DAC value for channel 0
  }
  if (params[1].OP == STREAM)
  {
    run_ch (&params[1]);  //generate DAC value for channel 1
  }
  if (ADC_cntr++ >= 9)
  {
    ADC_flag = 1;
    ADC_cntr = 0;
  }
  
}

void loop()
{
  if ( ADC_flag ) // this flag is up untile we complete this whole command
  {
    ADC_flag = 0;
    
    if (params[0].OP == STREAM)
    {
      ADMUX = 0xD4; //ADC4 - ADC1 VOUT1 --> set up the MUX for ADC4 - ADC1 operation which is A3 or Vout1
      ADCSRA |= 0x40;// convert
      while (!( ADCSRA & 0x10 )) {} // wait for conversion
      params[0].ADCv = ADC; //record results
      ADCSRA |= 0x10; // reset flag
    }

    if (params[1].OP == STREAM)
    {
      ADMUX = 0xD5; //ADC5 - ADC1 VOUT2 --> set up the MUX for ADC4 - ADC1 operation which is A2 or Vout2
      ADCSRA |= 0x40;// convert
      while (!( ADCSRA & 0x10 )) {} // wait for conversion
      params[1].ADCv = ADC;
      ADCSRA |= 0x10;    
    }
    
    if (params[0].OP == STREAM)
    {
      
      if (params[0].ADCv > 0x1FF) //this is just to convert the ADC value we just read. It is super weird 
      {
        params[0].ADCv = -( 0x400 - params[0].ADCv);
      }
      params[0].ADCv = params[0].ADCv - CH0_offset;
      Serial.print("DATA\r");
      Serial.print(((params[0].ADCv) >> 8) & 0xFF);
      Serial.print("\r");
      Serial.print(((params[0].ADCv)) & 0xFF);
      Serial.print("\r0\r");
      Serial.print(params[0].FdbkG);
      Serial.print("\r");
      Serial.print("DAC\r");
      Serial.print((params[0].DACv >> 8) & 0xFF);
      Serial.print("\r");
      Serial.print(((params[0].DACv)) & 0xFF);
      Serial.print("\r");
      dynamic_fdbk (&params[0]); // dynamic feedback. This is done last so that we wouldnt alter the value sent to the host
      
    }
    if (params[1].OP == STREAM)
    {
      
      if (params[1].ADCv > 0x1FF) //this is just to convert the ADC value we just read. It is super weird 
      {
        params[1].ADCv = -( 0x400 - params[1].ADCv);
      }
      params[1].ADCv = params[1].ADCv - CH1_offset;
      Serial.print("DATA\r");
      Serial.print(((params[1].ADCv) >> 8) & 0xFF);
      Serial.print("\r");
      Serial.print(((params[1].ADCv)) & 0xFF);
      Serial.print("\r1\r");
      Serial.print(params[1].FdbkG);
      Serial.print("\r");
      Serial.print("DAC\r");
      Serial.print(((params[1].DACv) >> 8) & 0xFF);
      Serial.print("\r");
      Serial.print(((params[1].DACv)) & 0xFF);
      Serial.print("\r");      
      dynamic_fdbk (&params[1]); // dynamic feedback. This is done last so that we wouldnt alter the value sent to the host
      
    }   
  }

  switch (current_USB_state)
  {
    case USB_fresh:
//      Serial.print("USB_fresh\r");
      if (Serial.available() > 0) //RX buffer holds up to 64 bytes
      {
        incomingByte = Serial.read();
        ascii_char = char(incomingByte);
        receive_USB_char ();
      }
      if (cmd_flag)
      {
        /*
         * this block deciphers the input cmd
         * it also deciphers what should the next stage be USB_serial, USB_channel0, USB_channel1
         */
        
        if (cmd == "*idn?") current_USB_state = USB_idn;
        else if (cmd == "STOP")  current_USB_state = USB_STOP;
        else if (cmd == "STREAM")current_USB_state = USB_STREAM;
        else if (cmd == "CV")    current_USB_state = USB_CV;
        else if (cmd == "CA")    current_USB_state = USB_CA;
        else if (cmd == "MPS")   current_USB_state = USB_MPS;
        else if (cmd == "ARB")   current_USB_state = USB_ARB;
        else if (cmd == "SWV")   current_USB_state = USB_SWV;
        else if (cmd == "*serial?")   current_USB_state = USB_serial;
        else if (cmd == "*channel0?")   current_USB_state = USB_channel0;
        else if (cmd == "*channel1?")   current_USB_state = USB_channel1;
        else                     current_USB_state = USB_ERROR;
        cmd_flag = false;
        cmd ="";        
      }
      break ;
    case USB_ERROR:
      Serial.write("Error\r");
      default_params(params);
      current_USB_state = USB_fresh;
      programming_success = false;
      cmd = "";
      cmd_flag = false; 
      break;
    case USB_idn:
      Serial.write("aMEASURE V2m\r");
      cmd = "";
      cmd_flag = false;      
      current_USB_state = USB_fresh;
      break;
    case USB_serial:
      Serial.write("5\r"); /////////////////////////////////////////////////// these change with every device
      cmd = "";
      cmd_flag = false;      
      current_USB_state = USB_fresh;
      break;
    case USB_channel0:
      Serial.write("4937, 500.0, 49.81, 0.998\r");
      cmd = "";
      cmd_flag = false;      
      current_USB_state = USB_fresh;
      break;
    case USB_channel1:
      Serial.write("4895, 499.3, 49.85, 1.007\r");
      cmd = "";
      cmd_flag = false;      
      current_USB_state = USB_fresh;
      break;/////////////////////////////////////////////////////////////////
    case USB_STOP:
    /*
     * STOP stage is basically the dump stage
     * when we are here, every variable is reset
     */
      default_params(params);
      current_USB_state = USB_fresh;
      programming_success = false;
      cmd = "";
      cmd_flag = false; 
      Serial.write("ACK\r");
      break;
    case USB_STREAM :
    /*
     * here we decide what channel should be on based on the input we receive
     * if ch0 then only cho
     * if ch1 then only ch1
     * if ALL then all
     */
      //Serial.print("USB_STREAM\r");
      if (Serial.available() > 0) //RX buffer holds up to 64 bytes
      {
        // read the input command
        incomingByte = Serial.read();
        ascii_char = char(incomingByte);
        receive_USB_char ();
      }
      if (cmd_flag) // cmd_flag is true when we are done reading and reached \r\n
      {      
        if (cmd == "ch0")
        {
           //Serial.print("ch0");
           params[0].OP = STREAM;
           current_USB_state = USB_fresh;
           cmd = "";
        }
        else if (cmd == "ch1")
        {
           //Serial.print("ch1");
           params[1].OP = STREAM;
           current_USB_state = USB_fresh;
           cmd = "";
        }
        else if (cmd == "ALL")
        {
           //Serial.print("ALL");
           params[0].OP = STREAM;
           params[1].OP = STREAM;
           current_USB_state = USB_fresh;
           cmd = "";
        }
        else  
        {
          current_USB_state = USB_ERROR;
          Serial.write("Error\r");
        } //the stream command received is wrong
        cmd_flag = false;        
      }
      break;
    case USB_CV:
    /*
     * we are CV mode, receive input arguments fololowed by \r\n
     * if not arguments are received, then the default arguments are used
     */
      
      if (Serial.available() > 0) //RX buffer holds up to 64 bytes
      {
        incomingByte = Serial.read();
        switch (input_counter_for_USB)
        {
          /*
           * the input_counter_for_USB is a counter that counts up and basically fills up statues registers by the incoming numbers
           */
          case 0:
            input_channel_for_USB = incomingByte;
            params[input_channel_for_USB].wave = CV;
            input_counter_for_USB++;
            break;
          case 1:
            params[input_channel_for_USB].FdbkG = incomingByte;
            input_counter_for_USB++;
            break;
          case 2:
            params[input_channel_for_USB].InitV = ( incomingByte << 8); //receive the MSB part of the InitV variable
            input_counter_for_USB++;
            break;
          case 3:
            params[input_channel_for_USB].InitV = params[input_channel_for_USB].InitV + incomingByte; //append the LSB part of InitV
            input_counter_for_USB++;
            break;
          case 4:
            params[input_channel_for_USB].HighV = ( incomingByte << 8);
            input_counter_for_USB++;            
            break;
          case 5:
            params[input_channel_for_USB].HighV = params[input_channel_for_USB].HighV + incomingByte;
            input_counter_for_USB++;
            break;
          case 6:
            params[input_channel_for_USB].LoowV = ( incomingByte << 8);
            input_counter_for_USB++;            
            break;
          case 7:
            params[input_channel_for_USB].LoowV = params[input_channel_for_USB].LoowV + incomingByte;
            input_counter_for_USB++;
            break;
          case 8:
            params[input_channel_for_USB].ScanR = ( incomingByte <<8 );
            input_counter_for_USB++;            
            break;
          case 9:
            params[input_channel_for_USB].ScanR = params[input_channel_for_USB].ScanR +  incomingByte;
            input_counter_for_USB++;
            break; 
          default:  
            /*
            * we are done setting the parameters for CV
            * now we are waiting for /r/n
            */
            ascii_char = char(incomingByte);
            check_USB_for_return ();
            set_fdbk (&params[input_channel_for_USB]);
            if (programming_success) // programming_success is set when we successfuly receive /r/n
            {
              params[input_channel_for_USB].wave = CV;
              programming_success = false;
              Serial.write("ACK\r");
              setup_CV (&params[input_channel_for_USB]);
            }
            break;
        }    
      }
      break;
    case USB_CA:
    /*
     * same thing as shown on CV but with less parameters to program
     */
      //Serial.print("USB_CA\r");
      if (Serial.available() > 0) //RX buffer holds up to 64 bytes
      {
        incomingByte = Serial.read();
        switch (input_counter_for_USB)
        {
          case 0:
            input_channel_for_USB = incomingByte;
            params[input_channel_for_USB].wave = CA;
            input_counter_for_USB++;
            break;
          case 1:
            params[input_channel_for_USB].FdbkG = incomingByte;
            input_counter_for_USB++;
            break;
          case 2:
            params[input_channel_for_USB].InitV = ( incomingByte << 8);
            input_counter_for_USB++;            
            break;
          case 3:
            params[input_channel_for_USB].InitV = params[input_channel_for_USB].InitV + incomingByte;
            input_counter_for_USB++;
            break;
          default:
            ascii_char = char(incomingByte);
            check_USB_for_return ();
            set_fdbk (&params[input_channel_for_USB]);
            if (programming_success)
            {
              params[input_channel_for_USB].wave = CA;
              programming_success = false;
              Serial.write("ACK\r");
              params[input_channel_for_USB].DACv = params[input_channel_for_USB].InitV;
            }
            break;
        }     
      }
      break;
    case USB_MPS:
      /*
      * same thing as shown on CV but we have the additional elements of Time1 and Time2
      */
      //Serial.print("USB_MPS\r");
      if (Serial.available() > 0) //RX buffer holds up to 64 bytes
      {
        incomingByte = Serial.read();
        switch (input_counter_for_USB)
        {
          case 0:
            input_channel_for_USB = incomingByte;
            params[input_channel_for_USB].wave = MPS;
            input_counter_for_USB++;
            break;
          case 1:
            params[input_channel_for_USB].FdbkG = incomingByte;
            input_counter_for_USB++;
            break;
          case 2:
            params[input_channel_for_USB].InitV = ( incomingByte << 8) ;
            input_counter_for_USB++;
            break;
          case 3:
            params[input_channel_for_USB].InitV = params[input_channel_for_USB].InitV + incomingByte ;
            input_counter_for_USB++;
            break;
          case 4:
            params[input_channel_for_USB].HighV = ( incomingByte << 8);
            input_counter_for_USB++;
            break;
          case 5:
            params[input_channel_for_USB].HighV = params[input_channel_for_USB].HighV + incomingByte;
            input_counter_for_USB++;
            break;
          case 6:
            params[input_channel_for_USB].LoowV = ( incomingByte << 8);
            input_counter_for_USB++;
            break;
          case 7:
            params[input_channel_for_USB].LoowV = params[input_channel_for_USB].LoowV + incomingByte;
            input_counter_for_USB++;
            break;
          case 8:
            params[input_channel_for_USB].Time1 = ( incomingByte <<8 );
            input_counter_for_USB++;
            break;
          case 9:
            params[input_channel_for_USB].Time1 = params[input_channel_for_USB].Time1 + incomingByte;
            input_counter_for_USB++;
            break; 
          case 10:
            params[input_channel_for_USB].Time2 = ( incomingByte <<8 );
            input_counter_for_USB++;
            break;
          case 11:
            params[input_channel_for_USB].Time2 = params[input_channel_for_USB].Time2 + incomingByte;
            input_counter_for_USB++;
            break; 
          default:
            ascii_char = char(incomingByte);
            check_USB_for_return ();
            set_fdbk (&params[input_channel_for_USB]);
            if (programming_success)
            {
              params[input_channel_for_USB].wave = MPS;
              programming_success = false;
              Serial.write("ACK\r");
              setup_MPS (&params[input_channel_for_USB]);
            }    
            break;
        }         
      }
      break;
      case USB_ARB:
        current_USB_state = USB_STOP;
        break;
        
    case USB_SWV:
      /*
      * same thing as shown on CV but we have the additional elements of Time peak voltage and step voltage
      */
      //Serial.print("USB_MPS\r");
      if (Serial.available() > 0) //RX buffer holds up to 64 bytes
      {
        incomingByte = Serial.read();
        switch (input_counter_for_USB)
        {
          case 0:
            input_channel_for_USB = incomingByte;
            params[input_channel_for_USB].wave = SWV;
            input_counter_for_USB++;
            break;
          case 1:
            params[input_channel_for_USB].FdbkG = incomingByte;
            input_counter_for_USB++;
            break;
          case 2:
            params[input_channel_for_USB].InitV = ( incomingByte << 8) ;
            input_counter_for_USB++;
            break;
          case 3:
            params[input_channel_for_USB].InitV = params[input_channel_for_USB].InitV + incomingByte ;
            input_counter_for_USB++;
            break;
          case 4:
            params[input_channel_for_USB].HighV = ( incomingByte << 8);
            input_counter_for_USB++;
            break;
          case 5:
            params[input_channel_for_USB].HighV = params[input_channel_for_USB].HighV + incomingByte;
            input_counter_for_USB++;
            break;
          case 6:
            params[input_channel_for_USB].SWV_step_const = ( incomingByte << 8);
            input_counter_for_USB++;
            break;
          case 7:
            params[input_channel_for_USB].SWV_step_const = params[input_channel_for_USB].SWV_step_const + incomingByte;
            input_counter_for_USB++;
            break;
          case 8:
            params[input_channel_for_USB].SWV_peak = ( incomingByte <<8 );
            input_counter_for_USB++;
            break;
          case 9:
            params[input_channel_for_USB].SWV_peak = params[input_channel_for_USB].SWV_peak + incomingByte;
            input_counter_for_USB++;
            break; 
          case 10:
            params[input_channel_for_USB].Time1 = ( incomingByte <<8 );
            input_counter_for_USB++;
            break;
          case 11:
            params[input_channel_for_USB].Time1 = params[input_channel_for_USB].Time1 + incomingByte;
            input_counter_for_USB++;
            break; 
          default:
            ascii_char = char(incomingByte);
            check_USB_for_return ();
            set_fdbk (&params[input_channel_for_USB]);
            if (programming_success)
            {
              params[input_channel_for_USB].wave = SWV;
              programming_success = false;
              
              Serial.write("ACK\r");
              setup_SWV (&params[input_channel_for_USB]);
            }    
            break;
        }         
      }
      break;
      default:
        current_USB_state = USB_STOP;
        break;
  }
}

void default_params(ch_params_t params[2]) // reverts every thing to default
{
  input_counter_for_USB = 0;
  
  int i = 0;

  for (i = 0; i < 2; i++)
  {
    // PC command
    params[i].OP = STOP;
    params[i].wave = CA;
    params[i].FdbkG = 0;
    params[i].ChNum = i;
    params[i].HighV = 512;
    params[i].LoowV = 512;
    params[i].InitV = 512;
    params[i].ScanR = 0;
    params[i].Time1 = 0;
    params[i].Time2 = 0;
    params[i].cntr = 0;
    params[i].ADCv = 0;
    params[i].DACv = 512;

    params[i].fnc_cntr = 0;
  
    // CV
    params[i].t_scan = 1;
    params[i].CV_dir = 1;
  
    // MPS
    params[i].MPS_V_flag = 0;
    params[i].MPS_time = 0;
    
    params[i].SWV_V_flag = 0;    // determines if we are in stage 1 or stage 2
    params[i].SWV_step = 0;
    params[i].SWV_step_const = 0;
    params[i].SWV_peak = 0;

    params[i].Gain_adjustment_counter = 0;
  }
  OCR1A = params[0].DACv;
  OCR1B = params[1].DACv;
  set_fdbk (&params[0]); // implement default feedback for channel 0
  set_fdbk (&params[1]); // implement default feedback for channel 1
  
}

void set_fdbk (ch_params_t *params) // set feedback gain
{  
  switch (params->ChNum)  //check for channel number
  {
    case 0:
      switch (params->FdbkG)
      {
        // [SEL1 SEL0] = [D5 D6];
        case 0: digitalWrite(5, LOW) ; digitalWrite(6, LOW) ; break;  // 5M
        case 1: digitalWrite(5, LOW) ; digitalWrite(6, HIGH); break; // 500k
        case 2: digitalWrite(5, HIGH); digitalWrite(6, LOW) ; break; // 50k
        case 3: digitalWrite(5, HIGH); digitalWrite(6, HIGH); break;  // 1k
      }
      break;
    case 1:
      switch (params->FdbkG)
      {
        // [SEL1 SEL0] = [D3 D4];
        case 0: digitalWrite(3, LOW) ; digitalWrite(4, LOW) ; break;  // 5M
        case 1: digitalWrite(3, LOW) ; digitalWrite(4, HIGH); break; // 500k
        case 2: digitalWrite(3, HIGH); digitalWrite(4, LOW) ; break; // 50k
        case 3: digitalWrite(3, HIGH); digitalWrite(4, HIGH); break;  // 1k
      }
      break;    
  }
}

void dynamic_fdbk (ch_params_t *params) // control TIA feedback gain based on the ADC value just read and call set_fdbk function to implement the changes
{
  if (params->OP == STREAM) //are we streaming?
  {    
    if (((params->ADCv < AMPLIFICATION_UPPER_BOUND) && (params->ADCv > AMPLIFICATION_LOWER_BOUND)) ||
        ((params->ADCv > ATTENUATION_UPPER_BOUND) || (params->ADCv < ATTENUATION_LOWER_BOUND)))     // is the ADC result out of bouds?
    {      
      // voltage change about fixed reference (2.5V) too small; needs amplification

      if ((params->ADCv < AMPLIFICATION_UPPER_BOUND) && (params->ADCv > AMPLIFICATION_LOWER_BOUND))
      {      
        
          //params->Gain_adjustment_counter = 0;
          if (params->FdbkG == 0) // already using largest feedback gain
          {
            // error
          }
          else
          {
            if ((params->Gain_adjustment_counter++ > 5 ) && (params->FdbkG < 3))
            {
              params->FdbkG--;  // amplify by 1 stage
              params->Gain_adjustment_counter = 0;              
            }
            else if ((params->Gain_adjustment_counter++ > 50 ) && (params->FdbkG == 3))
            {
              params->Gain_adjustment_counter = 0;
              params->FdbkG--;  // amplify by 1 stage
            }
          }
        
      }
      // output close to saturation (leading to inaccurate estimation); needs attenuation
      if ((params->ADCv > ATTENUATION_UPPER_BOUND) || (params->ADCv < ATTENUATION_LOWER_BOUND))
      {
        if (params->FdbkG == 3) // already using smallest feedback gain
        {
          // error
        }
        else
        {
          params->FdbkG++;  // attenuate by 1 stage
        }
      }    
    set_fdbk(params); // update TIA feedback gain
    }
    else
    {
      params->Gain_adjustment_counter = 0;
    }
  }
}

void setup_CV (ch_params_t *params) // set up CV
{
  params->DACv = params->InitV;
  // determine initial scan direction
  if (params->HighV < params->InitV)
  {
    params->CV_dir = 0;
  }
  else if (params->HighV == params->InitV)
  {
    if (params->HighV > params->LoowV)
    {
      params->CV_dir = 0;
    }
  }

  // for clarity, swap values so that higher potential is in HighV
  if (params->HighV < params->LoowV)
  {
    uint16_t tmp;
    tmp = params->HighV;
    params->HighV = params->LoowV;
    params->LoowV = tmp;
  }
  
  params->t_scan = 48828/(params->ScanR); // calculate delay between samples (in ms)
  if ( (48828 << 1) % (params->ScanR << 1) >= (params->ScanR)) //the division has a tendency to round down. this code helps with determining if we are better off rounding up or down and rounding up if necessary
  {
    params->t_scan++; // round up
  }
  //params->t_scan = params->t_scan * 5;
}

void setup_MPS (ch_params_t *params)  // set up MPS
{
  params->fnc_cntr = 0;  // reset step interval counter
  params->MPS_V_flag = 0;
  params->MPS_time = params->Time1;
  params->DACv = params->InitV;  
}

void setup_SWV (ch_params_t *params)  // set up MPS
{
  params->fnc_cntr = 0;  // reset step interval counter
  params->SWV_V_flag = 0;
  params->DACv = params->InitV;  
}

void gen_CV (ch_params_t *params) // generate CV
{
  params->fnc_cntr++; //
  if (params->fnc_cntr >= params->t_scan)
  {
    params->fnc_cntr = 0;  // reset scan interval counter
    if (params->CV_dir) // scan in +ve direction
    {
      params->DACv++ ;
      if (params->DACv >= params->HighV) // reverse scan direction
      {
        params->CV_dir = 0 ;
      }
    }
    else  // scan in -ve direction
    {
      params->DACv--;
      if (params->DACv <= params->LoowV) // reverse scan direction
      {
        params->CV_dir = 1 ;
      }
    }
  }
}

void gen_MPS (ch_params_t *params)
{
  params->fnc_cntr++;
  if (params->fnc_cntr >= params->MPS_time) // apply step potential
  {
    params->fnc_cntr = 0;  // reset step interval counter
    if (params->MPS_V_flag)  // step potential 1
    {
      params->MPS_V_flag = 0;
      params->MPS_time = params->Time1;
      params->DACv = params->HighV;
    }
    else  // step potential 2
    {
      params->MPS_V_flag = 1;
      params->MPS_time = params->Time2;
      params->DACv = params->LoowV;
    }
  }
}

void gen_SWV (ch_params_t *params)
{
  if (params->SWV_step_const <= ( params->HighV - params->LoowV))
  {
    params->fnc_cntr++;
    if (params->fnc_cntr >= params->Time1) // apply step potential
    {
      params->fnc_cntr = 0;  // reset step interval counter
      if (params->SWV_V_flag)  // step potential 1
      {
        params->SWV_V_flag = 0;
        params->DACv = params->InitV + params->SWV_step - params->SWV_peak;
        params->SWV_step += params->SWV_step_const;
      }
      else  // step potential 2
      {
        params->SWV_V_flag = 1;
        params->DACv = params->InitV + params->SWV_step + params->SWV_peak;
      }
    }
  }
}

void run_ch (ch_params_t *params)
{
  if (params->OP == STREAM) //is the channel on?
  {    
    // generate waveforms
    switch (params->wave)
    {
      case CA : /*CA*/          ; break;
      case CV : gen_CV(params)  ; break;
      case MPS: gen_MPS(params) ; break;
      case ARB: /*Arbitrary*/   ; break;
      case SWV: gen_SWV(params) ; break;
      default:/*nothing*/     ; break;
    }
  }
}

void receive_USB_char ()
{
    switch (ascii_char)
    {
      case '\r':
        //while(Serial.available()<1); //wait till next byte is received (not good! it's blocking)
        incomingByte = Serial.read();
        ascii_char = char(incomingByte);
        if (ascii_char == '\n')
        {
          cmd_flag = true;
        }
        else 
        {
          cmd_flag = false;
          cmd = "";
          Serial.write("aMEASURE V2m received incorrect second terminator!\r");
        }
        break;
      case '\n':
        cmd_flag = false;
        cmd = "";
        Serial.write("aMEASURE V2m received incorrect first terminator!\r");
        break;
      default:
        cmd = cmd + ascii_char;
        break;
    }
}
void check_USB_for_return ()
{
  /*
   * the program is to receive and process the \r\n command at USB
   * It set a flag that lets us know the \r\n are set
   * or it goes to STOP mode and resets the uC if anthing else is received
   */
  switch (ascii_char)
  {
    case '\r':
      //while(Serial.available()<1); //wait till next byte is received (not good! it's blocking)
      incomingByte = Serial.read();
      ascii_char = char(incomingByte);
      if (ascii_char == '\n')
      {
        cmd_flag = false;
        //reset the state to fresh to prepare the program for next set of structiion
        current_USB_state = USB_fresh;  
        input_counter_for_USB = 0;
        programming_success = true;
      }
      else 
      {
        cmd_flag = false;
        current_USB_state = USB_STOP;
        input_counter_for_USB = 0;
        programming_success = false;
        Serial.write("aMEASURE V2m received incorrect second terminator!\r");
      }
      break;
    case '\n':
      cmd_flag = false;
      current_USB_state = USB_STOP;
      input_counter_for_USB = 0;
      programming_success = false;
      Serial.write("aMEASURE V2m received incorrect first terminator!\r");
      break;
  }
  cmd = "";
}




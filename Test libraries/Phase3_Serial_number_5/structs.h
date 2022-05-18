enum     OPERATION {STREAM, STOP};
enum     WAVEFORM {CV, CA, MPS, ARB, SWV};
typedef struct
{
  // PC command
  enum     OPERATION OP;
  enum     WAVEFORM wave;
  //uint8_t  Class; //0xSY where S determines the operation mode and Y determines if the channel is active or not S=1 is CA S=2 is CV S=3 is MPS and S=4 is Arb
  uint8_t  ChNum; // channel number 
  uint8_t  FdbkG; // dynamic gain
  uint16_t HighV; // Higher voltage eshelon
  uint16_t LoowV; // lower voltage eshelon
  uint16_t InitV; // initial voltage
  uint16_t ScanR; // scan rate
  uint16_t Time1; // MPS stage one time
  uint16_t Time2; // MPS stage two time
  uint16_t cntr;  // general counter

  int16_t ADCv; // ADC voltage
  int16_t DACv; // DAC voltage

  uint16_t fnc_cntr;  // function generator counter

  // CV
  uint16_t t_scan;  // CV time delay between each step
  bool CV_dir;  // CV direction

  // MPS
  bool MPS_V_flag;    // determines if we are in stage 1 or stage 2
  uint16_t MPS_time;  //  a timer to have the elapsed time on each stage at hand

  //SWV
  bool SWV_V_flag;    // determines if we are in stage 1 or stage 2
  uint16_t SWV_step;
  uint16_t SWV_step_const;
  uint16_t SWV_peak;

  uint8_t Gain_adjustment_counter;
} ch_params_t;

#define AMPLIFICATION_UPPER_BOUND 10   // 2.651V
#define AMPLIFICATION_LOWER_BOUND -10  // 2.348V
#define ATTENUATION_UPPER_BOUND  480   // 4.794V
#define ATTENUATION_LOWER_BOUND -480   // 0.205V

#define CH0_offset 0;
#define CH1_offset 4;


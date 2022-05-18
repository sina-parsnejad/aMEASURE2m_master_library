///*
// * functions.c
// *
// *  Created on: 9/19/2016
// *      Author: Sina
// */
//
//#include "structs.h"
//
//void default_params(ch_params_t params[2])
//{
//  int i = 0;
//
//  for (i = 0; i < 2; i++)
//  {
//    // PC command
//    params[i].Class = 0;
//    params[i].FdbkG = 0;
//    params[i].ChNum = i;
//    params[i].HighV = 0;
//    params[i].LoowV = 0;
//    params[i].ScanR = 0;
//    params[i].Time1 = 0;
//    params[i].Time2 = 0;
//    params[i].cntr = 0;
//    params[i].ADCv = 0;
//    params[i].DACv = 0;
//  
//    // CV
//    params[i].t_scan = 0;
//    params[i].CV_step = 0;
//    params[i].CV_dir = 0;
//  
//    // MPS
//    params[i].MPS_chgV = 0;
//    params[i].MPS_V_flag = 0;
//
//  }
//}
//
//void set_fdbk (ch_params_t *params)
//{
//  // set TIA feedback gain
//  switch (params->ChNum)
//  {
//    case 0:
//      switch (params->FdbkG)
//      {
//        // [SEL1 SEL0] = [D5 D6];
//        case 0: digitalWrite(5, LOW) ; digitalWrite(6, LOW) ; break;  // 5M
//        case 1: digitalWrite(5, LOW) ; digitalWrite(6, HIGH); break; // 500k
//        case 2: digitalWrite(5, HIGH); digitalWrite(6, LOW) ; break; // 50k
//        case 3: digitalWrite(5, HIGH); digitalWrite(6, HIGH); break;  // 1k
//      }
//      break;
//    case 1:
//      switch (params->FdbkG)
//      {
//        // [SEL1 SEL0] = [D3 D4];
//        case 0: digitalWrite(3, LOW) ; digitalWrite(4, LOW) ; break;  // 5M
//        case 1: digitalWrite(3, LOW) ; digitalWrite(4, HIGH); break; // 500k
//        case 2: digitalWrite(3, HIGH); digitalWrite(4, LOW) ; break; // 50k
//        case 3: digitalWrite(3, HIGH); digitalWrite(4, HIGH); break;  // 1k
//      }
//      break;    
//  }
//}

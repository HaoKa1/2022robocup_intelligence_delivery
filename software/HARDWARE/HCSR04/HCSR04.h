#ifndef __HCSR04_H
#define __HCSR04_H

#include "sys.h"
#include "delay.h"



#define TRIG  PCout(3) 
//#define ECHO  PAin(0)

void HCSR04_TRIG_Send(void);
void HCSR04_TRIG_Init(void);
float Get_Distance(void);

void TIM5_Cap_Init(u16 arr,u16 psc);


#endif

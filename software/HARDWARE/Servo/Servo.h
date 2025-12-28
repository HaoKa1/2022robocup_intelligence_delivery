#ifndef __SERVO_H
#define __SERVO_H
#include "sys.h" 
#include "stm32f10x_tim.h"
#include "string.h"
#include "syn6288.h"

void TIM5_PWM_Init(u16 arr,u16 psc);
void TIM2_PWM_Init(u16 arr,u16 psc);
void Open(void);
void Close(void);
uint16_t QR(void);
void Put(uint16_t Number);
void USART2_Start(void);
void USART2_Stop(void);
	

#endif

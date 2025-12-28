#ifndef __MOTOR_H
#define __MOTOR_H

#include "stm32f10x.h" 


//初始化函数
void Motor_Configure(void);
void Motor(u8 port, s16 speed);
		 				    
#endif

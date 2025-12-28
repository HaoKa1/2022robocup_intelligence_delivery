#ifndef __UART5_H
#define __UART5_H	 

#include "stm32f10x.h"  

void UART5_init(uint32_t bound);				//串口5初始化 
void UART5_Start(void);							//启动串口5测距功能
void UART5_Stop(void);							//关闭串口5测距功能

#endif














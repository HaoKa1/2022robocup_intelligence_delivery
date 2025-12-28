#ifndef __UART4_H
#define __UART4_H	 

#include "stm32f10x.h"  

void UART4_init(uint32_t bound);		//串口4初始化 
void UART4_Start(void);							//启动串口4测距功能
void UART4_Stop(void);							//关闭串口4测距功能
#endif














#ifndef __LASER_H
#define __LASER_H

#include "stm32f10x.h" 	   

#define La	  PBin(11)	


//初始化函数
void Laser_Configure(void);

void Laser_Stop(void);
		 				    
#endif

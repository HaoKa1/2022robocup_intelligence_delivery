#include "Laser.h"
#include "stm32f10x.h"
#include "Motor.h"

void Laser_Configure(void)	       
{	
 	GPIO_InitTypeDef GPIO_InitStructure;
 
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);		//使能端口时钟


	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	//设置成上拉输入
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
}


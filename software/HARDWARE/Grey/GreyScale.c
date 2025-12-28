#include "GreyScale.h"
#include "stm32f10x.h"
/*
*******************************灰度传感器初始化**********************************
采用的传感器为模拟量返回，但是没有进行相应的数模转换，因为数模转换的IO口布局很复杂，
前期考虑时未能考虑周到，强烈建议队伍加入数模转换环节，否则返回的值是数字量的0（白线）、
1（黑线），无法进行精确的PID控制，只有比例控制，这对我们后期的巡线等造成了很大的影响
*/

void GREY_Configure(void)
{
 	GPIO_InitTypeDef GPIO_InitStructure;
 
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);		//使能端口时钟
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_11|GPIO_Pin_12;		//使能12个IO引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	//设置成上拉输入
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);		//使能端口时钟
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;		//使能12个IO引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	//设置成上拉输入
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);		//使能端口时钟
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_14|GPIO_Pin_15;		//使能12个IO引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	//设置成上拉输入
 	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

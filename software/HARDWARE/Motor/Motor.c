#include "Motor.h"
#include "stm32f10x.h"

void Motor_Configure(void){
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4|RCC_APB1Periph_TIM3, ENABLE); 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOA,ENABLE);
	TIM_DeInit(TIM4);
	TIM_DeInit(TIM3);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_6 | GPIO_Pin_7| GPIO_Pin_8| GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIO
	
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_6 | GPIO_Pin_7;	
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIO
	
	TIM_TimeBaseStructure.TIM_Period = 1010-1;
  TIM_TimeBaseStructure.TIM_Prescaler = 6-1;  //17 4K   19 3.6K  533 135HZ
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	
  TIM_OC1Init(TIM4, &TIM_OCInitStructure); 
  TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);//使能的预装载寄存器

  TIM_OC2Init(TIM4, &TIM_OCInitStructure); 
  TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);//使能的预装载寄存器

  TIM_OC3Init(TIM4, &TIM_OCInitStructure); 
  TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);//使能的预装载寄存器

  TIM_OC4Init(TIM4, &TIM_OCInitStructure); 
  TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);//使能的预装载寄存器

/**************************************************************************/
	
  TIM_OC1Init(TIM3, &TIM_OCInitStructure); 
  TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);//使能的预装载寄存器
	
  TIM_OC2Init(TIM3, &TIM_OCInitStructure); 
  TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);//使能的预装载寄存器
	
  TIM_OC3Init(TIM3, &TIM_OCInitStructure); 
  TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);//使能的预装载寄存器
	
  TIM_OC4Init(TIM3, &TIM_OCInitStructure); 
  TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);//使能的预装载寄存器
    
  TIM_ARRPreloadConfig(TIM4, ENABLE);	//使能定时器4
	TIM_ARRPreloadConfig(TIM3, ENABLE);	//使能定时器3
  TIM_Cmd(TIM4, ENABLE);
	TIM_Cmd(TIM3, ENABLE);

}
/*
port选择对应驱动端口，speed调整端口输出的PWM波占空比以调整速度
speed不得大于950，否则驱动将会损坏！！！！！！！
*/
void Motor(u8 port, s16 speed)
{
	if(speed >= 0)
	{
		if(speed>1000) speed=1000;
		switch(port)
		{
			case 1:  
				TIM4->CCR1 =  speed;
				TIM4->CCR2 =  0;   
				break;
			case 2:
				TIM4->CCR3 =  speed;
				TIM4->CCR4 =  0; 
				break;
			case 3:
				TIM3->CCR1 =  speed;
				TIM3->CCR2 =  0; 
				break;
			case 4:
				TIM3->CCR3 =  speed;
				TIM3->CCR4 =  0; 
				break;      
		}
	} 
	else 
	{
		if(speed<-1000) speed=-1000;
		switch(port)
		{
			case 1:
				TIM4->CCR1 =  0;
				TIM4->CCR2 =  -speed;			
				break;
			case 2:
				TIM4->CCR3 =  0;
				TIM4->CCR4 =  -speed; 
				break;
			case 3:
				TIM3->CCR1 =  0;
				TIM3->CCR2 =  -speed; 
				break;
			case 4:
				TIM3->CCR3 =  0;
				TIM3->CCR4 =  -speed; 
				break;   
		}
	}   
}



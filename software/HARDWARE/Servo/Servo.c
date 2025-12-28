#include "Servo.h"
#include "stm32f10x.h"
#include "delay.h"

//TIM2 PWM部分初始化 
//PWM输出初始化
//arr：自动重装值
//psc：时钟预分频数

u8 recv_ok = 0;       //接收完成标志
u8 uart_buf[32]={0};  //用于保存串口数据
u8 uart_cnt=0;        //用于定位串口数据的位置

void TIM5_PWM_Init(u16 arr,u16 psc)
{  
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);	//使能定时器2时钟
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA  | RCC_APB2Periph_AFIO, ENABLE);  //使能GPIO外设和AFIO复用功能模块时钟

	
	
	
   //设置该引脚为输出功能,输出TIM7 CH3、CH4的PWM脉冲波形	GPIOA.2 GPIOA.3
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_0|GPIO_Pin_1; //TIM7_CH3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIO
 
 

   //初始化TIM7
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值 
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
	
	//初始化TIM7 Channel2 PWM模式	 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //选择定时器模式:TIM脉冲宽度调制模式2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	
	TIM_OC1Init(TIM5, &TIM_OCInitStructure);        //根据T指定的参数初始化外设TIM7 OC2
  TIM_OC1PreloadConfig(TIM5, TIM_OCPreload_Enable);// 使能预装载寄存�
	
	TIM_OC2Init(TIM5, &TIM_OCInitStructure);        //根据T指定的参数初始化外设TIM7 OC2
  TIM_OC2PreloadConfig(TIM5, TIM_OCPreload_Enable);// 使能预装载寄存器
	
	TIM_Cmd(TIM5, ENABLE);  //使能TIM7

}


////TIM2 PWM部分初始化 
////PWM输出初始化
////arr：自动重装值
////psc：时钟预分频数
void TIM2_PWM_Init(u16 arr,u16 psc)
{  
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);	//使能定时器2时钟
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB| RCC_APB2Periph_AFIO, ENABLE);  //使能GPIO外设和AFIO复用功能模块时钟
	GPIO_PinRemapConfig(GPIO_FullRemap_TIM2 , ENABLE); //Timer2重映射  TIM2_CH2->PB5  

   //设置该引脚为输出功能,输出TIM7 CH3、CH4的PWM脉冲波形	GPIOA.2 GPIOA.3
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_15; //TIM7_CH3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIO	
	
   //设置该引脚为输出功能,输出TIM7 CH3、CH4的PWM脉冲波形	GPIOA.2 GPIOA.3
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_3|GPIO_Pin_10|GPIO_Pin_11; //TIM7_CH3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIO

	
   //初始化TIM2
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值 
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
	
	//初始化TIM7 Channel2 PWM模式	 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //选择定时器模式:TIM脉冲宽度调制模式2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	
	
	TIM_OC1Init(TIM2, &TIM_OCInitStructure);        //根据T指定的参数初始化外设TIM7 OC1
  TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);// 使能预装载寄存器
	
	TIM_OC2Init(TIM2, &TIM_OCInitStructure);        //根据T指定的参数初始化外设TIM7 OC2
  TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);// 使能预装载寄存器

	TIM_OC3Init(TIM2, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM7 OC3
  TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);// 使能预装载寄存器
	
  TIM_OC4Init(TIM2, &TIM_OCInitStructure);        //根据T指定的参数初始化外设TIM7 O4   
	TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);// 使能预装载寄存器
	
	TIM_Cmd(TIM2, ENABLE);  //使能TIM7

}	
/*
*********************扫码器*********************
利用strstr函数找到返回的字符型数值，再将数值赋值给Number
*/
uint16_t QR(void){
	uint16_t Number;
	if(recv_ok==1)  //接收完成
	{			
		if(strstr((char*)uart_buf,"0")) Number=0;

		if(strstr((char*)uart_buf,"1"))	Number=1;

		if(strstr((char*)uart_buf,"2"))	Number=2;

		if(strstr((char*)uart_buf,"3"))	Number=3;

		if(strstr((char*)uart_buf,"4"))	Number=4;

		if(strstr((char*)uart_buf,"5"))	Number=5;

		uart_cnt = 0;        //最后清零，重新计数
		recv_ok = 0;         //接收完成标志置0
		return Number;
	}
	else{
		return 6;
	}

}
/*
***********************************投球相关函数****************************************
Put：转盘转到对应位置，1560为安装测试后的初始位置，需自行调整，建议后续使用时进行宏定义
Open、Close：开关门，1500和500均为安装测试后位置，需自行调整
*/
void Put(uint16_t Number){
	if(Number<=2){
		TIM_SetCompare2(TIM2,1560+Number*400);
	}
	else{
		TIM_SetCompare2(TIM2,750+(Number-3)*400);
	}
}
void Open(void){
	TIM_SetCompare4(TIM2,1500);	
}
void Close(void){
	TIM_SetCompare4(TIM2,500);	
}

void USART2_Start(void)
{
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);								
}
void USART2_Stop(void)
{
	USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
}

void USART2_IRQHandler(void)      //UASRT2中断服务程序
{
 uint8_t d;

 //检测标志位
 if(USART_GetITStatus(USART2, USART_IT_RXNE)!=RESET)
 {
  //接受数据
  d = USART_ReceiveData(USART2);
     //将接受到的数据依次保存到数组里
  uart_buf[uart_cnt++] = d;  
  //GM65模块发完一组数据后会自动发送一个回车符，所以通过检测是否接受到回车来判断数据是否接受完成
  if(d == 0x0D) 
  {
   recv_ok = 1;  //接受完成
  }

  USART_ClearITPendingBit(USART2,USART_IT_RXNE); //清空标志位
 } 
}

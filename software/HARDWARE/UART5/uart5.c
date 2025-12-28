#include "UART5.h"

uint16_t B_distance=0;	//连接到串口5的测距传感器返回的检测数据

/*UART5中断服务函数*/
void UART5_IRQHandler(void)
{
  static uint8_t state = 0;
	static uint16_t dist_temp = 0;
	uint8_t rev;
  if (USART_GetITStatus(UART5, USART_IT_RXNE) != RESET) //接收到数据
    {
        rev = USART_ReceiveData(UART5);
				switch(state)
				{
					case 0:		if(rev == 'e')		//起始状态0，检测到'e'，继续检测后续是否为':'
												state = 1;
											break;
					case 1:		if(rev == ':')		//检测到':'，继续检测后续是否为'0'
												state =	2;
											else
												state = 0;			//否则退回状态0
											break;
					case 2:		if(rev == '0')		//检测到'0'，表明本条数据帧有效，继续检测
												state = 3;
											else
												state = 0;			//否则退回状态0
											break;
					case 3:		if(rev == 'd')		//检测到'd'，继续检测
												state = 4;
											break;
					case 4:		if(rev == ':')		//检测到':'，表明后续数据为有效数据，继续检测
											{
												dist_temp = 0;	//准备保存新数据
												state = 5;
											}
											else
												state = 3;			//否则退回状态3
											break;
					case 5:		if(rev == ' ')			//遇到'空格'，本轮退出
											break;
										else 
										{
											if((rev >= '0')&&(rev <= '9'))		//检测到'数字'
													dist_temp = dist_temp * 10 + rev - '0';
											else
											{
												if(rev == 'm')	//数字接收正常结束，刷新距离A测量值，并退回状态'0'
												{
													B_distance = dist_temp;
													state = 0;		
												}
												else						//数字非正常结束，本次数值无效，退回状态0
													state = 0;
											}
										}
				}
    }
}




//初始化IO 串口5
//pclk1:PCLK1时钟频率(Mhz)
//bound:波特率
void UART5_init(u32 bound)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;																																																																																														

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD, ENABLE);// GPIOC、GPIOD时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);

	USART_DeInit(UART5);                                //复位串口5
	//UART5_TX   PC.12
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;            //PC.12
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	     //复用推挽输出
	GPIO_Init(GPIOC, &GPIO_InitStructure);               //初始化PC.12

	//UART5_RX	  PD.2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;            //PD.2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //浮空输入
	GPIO_Init(GPIOD, &GPIO_InitStructure);               //初始化PD2

	USART_InitStructure.USART_BaudRate = bound;                                    //一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;                    //字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;                         //一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;                            //无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	               //收发模式
	USART_Init(UART5, &USART_InitStructure); 						//初始化串口5

	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2 ; 	//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;					//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;							//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);															//根据指定的参数初始化VIC寄存器
	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);								//开启接收中断
	
	USART_Cmd(UART5, ENABLE);                                   //使能串口
}

void UART5_Start(void)
{
	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);								
}
void UART5_Stop(void)
{
	USART_ITConfig(UART5, USART_IT_RXNE, DISABLE);								
}

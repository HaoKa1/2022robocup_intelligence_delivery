#include "USART2.h"

void usart_init2(unsigned int baud)
{
    GPIO_InitTypeDef GPIO_Init_Structure;                            //定义GPIO结构体
    USART_InitTypeDef USART_Init_Structure;                          //定义串口结构体
	NVIC_InitTypeDef  NVIC_Init_Structure;														//定义中断结构体
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
    RCC_APB2PeriphClockCmd(USART2_GPIO_CLK,  ENABLE);                 //开启GPIOA时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,  ENABLE);            //开启APB2总线复用时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,  ENABLE);          //开启USART1时钟
    
    //配置PA2 TX
    GPIO_Init_Structure.GPIO_Mode  = GPIO_Mode_AF_PP;                //复用推挽
    GPIO_Init_Structure.GPIO_Pin   = USART2_TX_GPIO_PIN;
    GPIO_Init_Structure.GPIO_Speed = GPIO_Speed_10MHz;
    
    GPIO_Init( USART2_GPIO_PORT, &GPIO_Init_Structure);
    
    //配置PA3 RX
    GPIO_Init_Structure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;                //复用推挽
    GPIO_Init_Structure.GPIO_Pin   = USART2_RX_GPIO_PIN;
    GPIO_Init( USART2_GPIO_PORT, &GPIO_Init_Structure);
	
    USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);	
    USART_Init_Structure.USART_BaudRate = baud;                                          //波特率设置为115200
    USART_Init_Structure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;       //硬件流控制为无
    USART_Init_Structure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;                       //模式设为收和发
    USART_Init_Structure.USART_Parity = USART_Parity_No;                                   //无校验位
    USART_Init_Structure.USART_StopBits = USART_StopBits_1;                                //一位停止位
    USART_Init_Structure.USART_WordLength = USART_WordLength_8b;                           //字长为8位  
    USART_Init(USART2, &USART_Init_Structure);  
    USART_Cmd(USART2, ENABLE);
		
		//中断结构体配置
	NVIC_Init_Structure.NVIC_IRQChannel 			=   USART2_IRQn;
	NVIC_Init_Structure.NVIC_IRQChannelCmd   	=   ENABLE;
	NVIC_Init_Structure.NVIC_IRQChannelPreemptionPriority  =  2;
	NVIC_Init_Structure.NVIC_IRQChannelSubPriority         =  3;
	NVIC_Init(&NVIC_Init_Structure);
}

/**
 * 功能：串口写字节函数
 * 参数1：USARTx ：串口号
 * 参数2：Data   ：需写入的字节
 * 返回值：None
 */
void USART_Send_Byte(USART_TypeDef* USARTx, uint16_t Data)
{
    USART_SendData(USARTx, Data);
    while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE)==RESET);
}
/**
 * 功能：串口写字符串函数
 * 参数1：USARTx ：串口号
 * 参数2：str    ：需写入的字符串
 * 返回值：None
 */
void USART_Send_String(USART_TypeDef* USARTx, char *str)
{
    uint16_t i=0;
    do
    {
        USART_Send_Byte(USARTx,  *(str+i));
        i++;
    }
    while(*(str + i) != '\0');
        
    while(USART_GetFlagStatus(USART1, USART_FLAG_TC)==RESET);
}

/**
 * 功能：重定向
 */
int fgetc(FILE *f)
{
    while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE)==RESET);
    return (int)USART_ReceiveData(USART1);
}

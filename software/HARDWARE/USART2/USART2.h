#ifndef __USART2_H
#define __USART2_H

#include "stm32f10x.h"
#include <stdio.h>

//串口2
#define USART2_GPIO_PORT      GPIOA
#define USART2_GPIO_CLK       RCC_APB2Periph_GPIOA
#define USART2_TX_GPIO_PIN    GPIO_Pin_2
#define USART2_RX_GPIO_PIN    GPIO_Pin_3

void usart_init2(unsigned int baud);

void USART_Send_Byte(USART_TypeDef* USARTx, uint16_t Data);
void USART_Send_String(USART_TypeDef* USARTx, char *str);
#endif

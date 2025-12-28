#ifndef __COLOR_H
#define __COLOR_H
#include "stm32f10x.h"
#include "tcs34725.h"
#include "delay.h"
#include "Servo.h"
#include "string.h"

extern COLOR_RGBC rgb;
extern COLOR_HSL  hsl;

/*
定义一个结构体，因为不同颜色的球rgb的c值和hsl的h值差异最明显，我们只需要这两个参数
就能判断颜色，因此我们构建结构体只需要这两个条件，简化参数
*/
typedef struct{
	unsigned short h;       //[0,360]
	unsigned short c;
}HC;

void GPIO_Config(void);
void Color_Reset(void);
void Color_Drop(void);
HC Color_recognition(void);
uint16_t Color_Judge(HC hc);

#endif

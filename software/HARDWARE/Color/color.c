#include "color.h"

void GPIO_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO, ENABLE);//GPIOA,GPIOB,GPIOC,AFIO;
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);
}
/*
三个舵机复位，数值为安装测试后调整
*/
void Color_Reset(void){
	TIM_SetCompare2(TIM5,750);	//分选舵机初始化位置	TIM5 2
	TIM_SetCompare2(TIM2,545);	//转盘舵机初始化位置	TIM2 2
	TIM_SetCompare4(TIM2,500);  //开门舵机初始化位置	TIM2 4
}
/*
控制舵机相连的拨片波动，防止卡球，数值需安装后测试调整
*/
void Color_Drop(void){
	static uint16_t Drop=0;
	if(Drop==0){
		TIM_SetCompare2(TIM5,475);	//
		Drop=1-Drop;
	}
	else{
	TIM_SetCompare2(TIM5,770);	//
		Drop=1-Drop;
	}
}
/*
*************************颜色识别函数****************************
从TCS34725中获得rgb和hsl值，并赋值给hc，返回hc，hc的数据类型为HC
*/
HC Color_recognition(void){
	HC hc;
  TCS34725_GetRawData(&rgb);
	TCS34725_GetRawData(&rgb);
  RGBtoHSL(&rgb,&hsl);
	hc.h=hsl.h;
	hc.c=rgb.c;
	return hc;
}
/*
*********************************颜色-数字判断函数*******************************
根据获得的hc值来判断是哪种颜色的球，并获得它所对应的数字
需要注意的是，在不同的球、不同光照情况下，球的rgb和hsl等值各不相同，需要反复试验，
找出最佳的区间，否则球的识别和分选极易出错
*/
uint16_t Color_Judge(HC hc){
	uint16_t flag=0;
  if(hc.h>=190&&hc.h<=215){
     flag=4;
    }
  if(hc.h>=90&&hc.h<=130&&hc.c>=100&&hc.c<=500){		//160     
     flag=5;
     }
  if(hc.h>0&&hc.h<=15){     
     flag=2;
     }
  if(hc.h>=60&&hc.h<=85){          
     flag=3;
     }
  if(hc.h>=135&&hc.h<190&&hc.c>500){
     flag=1;
     }
	return flag;
}

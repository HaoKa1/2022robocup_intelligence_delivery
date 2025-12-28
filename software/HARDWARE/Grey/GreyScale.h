#ifndef GREYSCALE_H
#define GREYSCALE_H

#include "stm32f10x.h" 
/*
对灰度的端口进行宏定义，方便后续的调用使用
*/
#define	GREY_1 PBin(12)
#define	GREY_2 PBin(13)
#define	GREY_3 PBin(14)
#define	GREY_4 PBin(15)
#define	GREY_5 PCin(6)
#define	GREY_6 PCin(7)
#define	GREY_7 PCin(8)
#define	GREY_8 PCin(9)
#define	GREY_9 PAin(11)
#define	GREY_10 PAin(12)
#define	GREY_11 PCin(14)
#define	GREY_12 PCin(15)

void GREY_Configure(void);

#endif

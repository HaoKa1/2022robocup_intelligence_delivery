#ifndef __ROUTE_H
#define __ROUTE_H

#include "stm32f10x.h" 

//初始化函数
void Route_Straight(void);
void Route_Stop(void);
void Route_TurnLeft(double Num);
void Route_TurnRight(double Num);
void Route_TurnCorner(void);
void Route_Start(void);
void Route_GoLine(void);
void Route_Movein(void);
void Route_Moveout(void);

void Route_StraightLow(void);
void Route_TurnLeftLow(double Num);
void Route_TurnRightLow(double Num);
void Route_GoLineLow(void);
void Route_TurnCornerLow(void); 

void Route_FirstGear(void);
void Route_SecondGear(void);
void Route_ThirdGear(void);
void Route_FourthGear(void);
#endif

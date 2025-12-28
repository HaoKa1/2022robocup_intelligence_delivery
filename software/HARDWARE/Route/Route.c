#include "Route.h"
#include "GreyScale.h"
#include "stm32f10x.h"
#include "Motor.h"
#include "Laser.h"
#include "pid.h"
#include "delay.h"
/*
因为驱动接线正反有差别，代码上统一将前进的数值记为负数
*/
#define speed -350
#define round -45
#define speedlow -150
#define	roundlow -25
#define ZH 1
#define YH 2
#define ZQ 3
#define	YQ 4
/*
***********************************常规速度运动*************************************
Straight 		直行
TurnRight		向右调整
TurnLeft		向左调整
TurnCorner	逆时针自转
Stop				停车
Start				从出发区启动
GoLine			巡线
Movein			向货站平移
Moveout			远离货站平移
注：因为右后轮（YH）电机存在物理上的缺陷，因此在数值上做了补偿，如果选购的电机测试后
没有明显电气特性，那么数值修改为与其他相同即可
*/
void Route_Straight(void){

	Motor(ZH,speed);
	Motor(YH,speed-18);
	Motor(ZQ,speed);
	Motor(YQ,speed-6);
}

void Route_TurnRight(double Num){
	//前进+绕正前方右摆尾线性相加
	Motor(YQ,speed-Num*round);
	Motor(ZQ,speed+Num*round);
	Motor(ZH,speed);
	Motor(YH,speed);
}

void Route_TurnLeft(double Num){
	//前进+绕正前方左摆尾线性相加
	Motor(YQ,speed+Num*round);
	Motor(ZQ,speed-Num*round);
	Motor(ZH,speed);
	Motor(YH,speed);
}

void Route_TurnCorner(void){
	//逆时针自转
	Motor(YQ,speed);
	Motor(ZQ,-speed);
	Motor(YH,speed);
	Motor(ZH,-speed);
}

void Route_Stop(void){
	Motor(ZQ,0);
	Motor(ZH,0);
	Motor(YQ,0);
	Motor(YH,0);
}

void Route_Start(void){
	Motor(YQ,speed);
	Motor(ZQ,0);
	Motor(YH,0);
	Motor(ZH,speed);
}

/*
**********************************常规速度巡线*************************************
因为灰度传感器的缘故，我们无法使用PID做精确控制，对轨迹的偏移仅仅做了比例控制处理，
仍然建议有条件上编码电机重构Route，并且采用模数转换做PID控制提高精确性（在赛事大规则
不变化的背景下，本套Route巡线能也能发挥出较好的巡线精度，但若其他高校实力变强，运动
速度明显加快，本套巡线功能的弊端暴露会很明显）
*/
void Route_GoLine(void){	
	if(GREY_1==0){
		Route_TurnLeft(5);	
	}
	else if(GREY_2==0){
		Route_TurnLeft(4);	
	}
	else if(GREY_3==0){
		Route_TurnLeft(3);	
	}
	else if(GREY_4==0){
		Route_TurnLeft(2);	
	}
	else if(GREY_5==0){
		Route_TurnLeft(1);	
	}
	else if(GREY_8==0){
		Route_TurnRight(1);	
	}
	else if(GREY_9==0){
		Route_TurnRight(2);	
	}
	else if(GREY_10==0){
		Route_TurnRight(3);	
	}
	else if(GREY_11==0){
		Route_TurnRight(4);	
	}
	else if(GREY_12==0){
		Route_TurnRight(5);	
	}
	else if(GREY_6==0||GREY_7==0){
		Route_Straight();
	}
}

void Route_Movein(void){
	Motor(ZQ,0.375*speed);
	Motor(YH,0.46*speed);
	Motor(ZH,0.375*-speed);
	Motor(YQ,0.375*-speed);
}

void Route_Moveout(void){
	Motor(YQ,0.375*speed);
	Motor(ZH,0.375*speed);
	Motor(ZQ,0.375*-speed);
	Motor(YH,0.46*-speed);
}
void Route_StraightLow(void){
	Motor(ZH,speedlow);
	Motor(YH,1.2*speedlow);
	Motor(ZQ,speedlow);
	Motor(YQ,speedlow);
}
/*
***************************************慢速巡线*******************************************
仅将速度降低，功能无变化，在调整慢速比例的过程中，需要注意的是，不合理的配重设计+过慢的速度
会让四个电机的特性产生出较大差异，因此不建议将慢速调整的太慢，难以排查问题
*/
void Route_TurnLeftLow(double Num){
	Motor(YQ,speedlow+Num*roundlow);
	Motor(ZQ,speedlow-Num*roundlow);
	Motor(ZH,speedlow);
	Motor(YH,1.2*speedlow);
}
void Route_TurnRightLow(double Num){
	Motor(YQ,speedlow-Num*roundlow);
	Motor(ZQ,speedlow+Num*roundlow);
	Motor(ZH,speedlow);
	Motor(YH,1.2*speedlow);
}
void Route_GoLineLow(void){
	if(GREY_1==0){
		Route_TurnLeftLow(5);	
	}
	else if(GREY_2==0){
		Route_TurnLeftLow(4);	
	}
	else if(GREY_3==0){
		Route_TurnLeftLow(3);	
	}
	else if(GREY_4==0){
		Route_TurnLeftLow(2);	
	}
	else if(GREY_5==0){
		Route_TurnLeftLow(1);	
	}
	else if(GREY_8==0){
		Route_TurnRightLow(1);	
	}
	else if(GREY_9==0){
		Route_TurnRightLow(2);	
	}
	else if(GREY_10==0){
		Route_TurnRightLow(3);	
	}
	else if(GREY_11==0){
		Route_TurnRightLow(4);	
	}
	else if(GREY_12==0){
		Route_TurnRightLow(5);	
	}
	else if(GREY_6==0||GREY_7==0){
		Route_StraightLow();
	}
}
void Route_TurnCornerLow(void){
	//逆时针自转
	Motor(YQ,speedlow);
	Motor(ZQ,-speedlow);
	Motor(YH,speedlow);
	Motor(ZH,-speedlow);
}

/*
************四挡减速设计**************
*/
void Route_FirstGear(void){
	Motor(ZH,0.15*speed);
	Motor(YH,0.15*speed);
	Motor(ZQ,0.15*speed);
	Motor(YQ,0.15*speed);
}
void Route_SecondGear(void){
	Motor(ZH,0.3*speed);
	Motor(YH,0.3*speed);
	Motor(ZQ,0.3*speed);
	Motor(YQ,0.3*speed);
}
void Route_ThirdGear(void){
	Motor(ZH,0.45*speed);
	Motor(YH,0.45*speed);
	Motor(ZQ,0.45*speed);
	Motor(YQ,0.45*speed);
}
void Route_FourthGear(void){
	Motor(ZH,0.6*speed);
	Motor(YH,0.6*speed);
	Motor(ZQ,0.6*speed);
	Motor(YQ,0.6*speed);
}

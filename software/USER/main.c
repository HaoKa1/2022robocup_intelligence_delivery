/*
*---------------------------------中国机器人大赛智能投送赛道国赛---------------------------------*
芯片型号：STM32F103RCT6
硬件型号：
	灰度传感器：S312B--模拟量版本（数字量版本将导致红蓝出发区出发严重问题，需长时间调试，互换性不强，不建议使用）
	颜色识别模块：TCS34725
	激光测距模块：正点原子MS53L0M（此版本为标注量程为2m，超出量程将随机返回错误数值，易产生问题，建议上4m版本的MS53L1M）
	超声波测距模块：HC-SR04（超声波的优势在于无需使用串口，我们选用的芯片型号IO口过少，使用十分紧张，
	因此在避障时设计采用超声波，减少对串口的占用，但因为超声波的trig需要TIM发出PWM波信号，而这个信号又会与电机驱动的PWM输出产生冲突，
	因此我们最终两害相权取其轻，放弃了超声波测距的使用，但对于一个扇形的测距范围和一个IO紧张的设计来说，可以尝试攻克这个PWM的冲突问题以更好发挥性能
	电机驱动：得科技术2路大功率驱动
		（1、建议电机驱动队伍重新购买其他硬件或自行设计，此驱动200一块，性价比太低，此外MOS管虚焊，短路等问题频繁
			2、选购驱动时可选择较便宜硬件，此赛项赛道短，调整时间少，运动速度慢，无需大功率驱动支持）				
	数字舵机：RDS3218（舵机型号无需过于拘泥，但必须是数字舵机，180度舵机选择15kg扭矩，360度舵机选择30kg扭矩即可）
	扫码模块：GM65	
	语音播报模块：SYN6288	
	减速电机：不再建议使用减速电机，在启动和急停时十分不稳定，并且各个电机的电气特性存在差异，需要在软件上不断调整，可采用编码电机等，
	但对于驱动部分的代码则需自行修改，另外选购时转速100转完全满足要求，过高转速反而影响减速比参数
选购指南：如果赛事场地大小、规则等没有大的变动，硬件可完全参照以上购买，如赛事有其他要求，在选购时，建议大家选择大店铺购买，如正点原子等，
	价格可能略贵，但是在例程代码和技术支持和售后服务上都有较好的保障，能让大家快速上手调试，其他例如risym之流的淘宝店铺，
	完全没有技术支持，只能凭论坛发问和自行摸索，会花费大量时间
操作系统：FreeRTOS
开发成员：Haokai Dai/Kaicheng Jin
指导教师：Mingyu Du
注释维护：Haokai Dai
更新时间：2025/12/08
*---------------------------------中国机器人大赛智能投送赛道国赛---------------------------------*
*/
#include "stm32f10x.h"
#include "led.h"
#include "GreyScale.h"
#include "Laser.h"
#include "Motor.h"
#include "Route.h"
#include "syn6288.h"
#include "Servo.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "USART1.h"
#include "USART2.h"
#include "uart4.h"
#include "uart5.h"
#include "tcs34725.h"
#include "color.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
/**************SYN6288芯片设置命令（模块配置，无需改动）*********************/
u8 SYN_StopCom[] = {0xFD, 0X00, 0X02, 0X02, 0XFD}; //停止合成
u8 SYN_SuspendCom[] = {0XFD, 0X00, 0X02, 0X03, 0XFC}; //暂停合成
u8 SYN_RecoverCom[] = {0XFD, 0X00, 0X02, 0X04, 0XFB}; //恢复合成
u8 SYN_ChackCom[] = {0XFD, 0X00, 0X02, 0X21, 0XDE}; //状态查询
u8 SYN_PowerDownCom[] = {0XFD, 0X00, 0X02, 0X88, 0X77}; //进入POWER DOWN 状态命令


/*
*******定义一个转盘位置与相应颜色的结构体**********
index为颜色代表的数字1-5
quantity为某位置中index颜色已有球的数量
*/
typedef struct{
	unsigned short index; 
	unsigned short quantity;
}POSITION;
POSITION position[5]={0,0};
/*	
***************************************测距模块********************************
注意：上位机通过USB-TTL模块连接开发板的串口1，可以用串口调试助手查看测距结果
测距模块A接在开发板串口4（PC10、PC11），串口4测距结果始终保存在A_distance变量中
测距模块B接在开发板串口5（PC12、PD2），串口5测距结果始终保存在B_distance变量中
*/
extern uint16_t A_distance,B_distance;
/*
***********************************操作系统说明****************************************
代码采用的是FreeRTOS操作系统，本工程已经将操作系统移植，与操作系统相关的代码均在main函数中，
涉及到的任务时间片轮转、句柄、队列、任务调度、优先级等知识需先行了解学习，
通过OS，可以让单片机实现伪多线程的功能，在22年的比赛中，规则中的“边走边分选”要求需要OS
，比赛时将此规则弱化了，但我认为使用OS能够更好地理清思路和逻辑，也方便后续的代码维护
*/
//各任务句柄
TaskHandle_t 	Drop_Handle;
TaskHandle_t	Recognition_Handle;
TaskHandle_t	Judge_Handle;
TaskHandle_t	Select_Handle;
TaskHandle_t 	Stop_Handle;
TaskHandle_t 	Movein_Handle;
TaskHandle_t 	QR_Handle;
TaskHandle_t 	Put_Handle;
TaskHandle_t 	Moveout_Handle;
TaskHandle_t 	Forward_Handle;
TaskHandle_t  Speak_Handle;
TaskHandle_t	Wave_Handle;
TaskHandle_t	Avoid_Handle;
TaskHandle_t	TOF_Handle;
TaskHandle_t	Goline_Handle;
TaskHandle_t	Corner_Handle;
TaskHandle_t	GolineLow_Handle;
TaskHandle_t	GoLineNew_Handle;
TaskHandle_t	Gohome_Handle;

//各任务所需队列
QueueHandle_t	xQueueHC;
QueueHandle_t	xQueueIndex;
QueueHandle_t	xQueueTOF;
QueueHandle_t	xQueueQR;
QueueHandle_t	xQueueSpeak;

/*
RTOS移植所需函数，无需修改
*/
static void prvSetupHardware( void ){
	/* Start with the clocks in their expected state. */
	RCC_DeInit();

	/* Enable HSE (high speed external clock). */
	RCC_HSEConfig( RCC_HSE_ON );

	/* Wait till HSE is ready. */
	while( RCC_GetFlagStatus( RCC_FLAG_HSERDY ) == RESET )
	{
	}

	/* 2 wait states required on the flash. */
	*( ( unsigned long * ) 0x40022000 ) = 0x02;

	/* HCLK = SYSCLK */
	RCC_HCLKConfig( RCC_SYSCLK_Div1 );

	/* PCLK2 = HCLK */
	RCC_PCLK2Config( RCC_HCLK_Div1 );

	/* PCLK1 = HCLK/2 */
	RCC_PCLK1Config( RCC_HCLK_Div2 );

	/* PLLCLK = 8MHz * 9 = 72 MHz. */
	RCC_PLLConfig( RCC_PLLSource_HSE_Div1, RCC_PLLMul_9 );

	/* Enable PLL. */
	RCC_PLLCmd( ENABLE );

	/* Wait till PLL is ready. */
	while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
	{
	}

	/* Select PLL as system clock source. */
	RCC_SYSCLKConfig( RCC_SYSCLKSource_PLLCLK );

	/* Wait till PLL is used as system clock source. */
	while( RCC_GetSYSCLKSource() != 0x08 )
	{
	}

	/* Enable GPIOA, GPIOB, GPIOC, GPIOD, GPIOE and AFIO clocks */
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |RCC_APB2Periph_GPIOC
							| RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE );

	/* SPI2 Periph clock enable */
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_SPI2, ENABLE );


	/* Set the Vector Table base address at 0x08000000 */
	NVIC_SetVectorTable( NVIC_VectTab_FLASH, 0x0 );

	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );

	/* Configure HCLK clock as SysTick clock source. */
	SysTick_CLKSourceConfig( SysTick_CLKSource_HCLK );
}

/*
**************舵机复位****************
将三个舵机复位并临时挂起不使用的子任务
*/
void RervoReset_task(void *pvParameters){
	Color_Reset();
	vTaskDelay(10);
	vTaskSuspend(GoLineNew_Handle);
	vTaskSuspend(Movein_Handle);
	vTaskSuspend(Moveout_Handle);
	vTaskSuspend(QR_Handle);
	vTaskSuspend(Put_Handle);
	vTaskSuspend(Avoid_Handle);
	vTaskSuspend(Speak_Handle);
	vTaskSuspend(GolineLow_Handle);
	vTaskSuspend(Wave_Handle);
	vTaskDelete(NULL);
}
/*
***********************************出发区出发*************************************
从红/蓝色出发区出发，灰度第7路为0并且第八路为1时判断为已走到赛道中心，结束出发区任务
*/
void Start_task(void *pvParameters){
	Route_Start();
	while(1){
		if(GREY_7==0&&GREY_8==1)	break;
	}

	vTaskDelete(NULL);
}
/*
********************************侧边激光测距***************************************
获得串口返回的B_distance数值，并判断是否在范围内，如果cnt==1，那么是第一次侧边有箱体
（即翻斗）立即发信号给停车子任务，如果cnt!=1，那么是碰到了后续需要投球的货站，则继续前进，
直到B_distance数值超出范围，认为小车移动到了箱体正中央，发信号给停车
注：B_distance的阈值范围和安装位置有关，与场地布置也有关，需要自行测试确定
*/
void TOF_task(void *pvParameters){
	TickType_t xLastWakeTime;
	uint16_t Send_distance;
	uint8_t	cnt=0;
	while(1){
		Send_distance=B_distance;
		if((Send_distance>=90&&Send_distance<=220)){
			cnt++;
			if(cnt==1){
				xTaskNotifyGive(Stop_Handle);
				vTaskSuspend(Goline_Handle);
				vTaskSuspend(NULL);			
			}
			else{
				while(1){
					if(B_distance>=220)	break;
				}
				xTaskNotifyGive(Stop_Handle);
				vTaskSuspend(Goline_Handle);
				vTaskSuspend(NULL);				
			}

		}
		xLastWakeTime = xTaskGetTickCount();
		vTaskDelayUntil(&xLastWakeTime,20);		
	}
}
/*
**************前方激光测距**************
采用的是激光测距模块，有能力可改用超声波
*/
void Wave_task(void *pvParameters){
	TickType_t xLastWakeTime;
	while(1){
		if(330>=A_distance&&A_distance>=280){
			vTaskSuspend(Goline_Handle);
			xTaskNotifyGive(Avoid_Handle);
			vTaskSuspend(NULL);
		}
		xLastWakeTime = xTaskGetTickCount();
		vTaskDelayUntil(&xLastWakeTime,10);
	}
}
/*
***********转盘转动下球*************
调用Color_Drop
*/
void Drop_task(void *pvParameters){
	uint16_t val=0;
	while(1){
		val=ulTaskNotifyTake(pdTRUE,2);
		if(val){
				Color_Drop();
				vTaskDelay(5);
				xTaskNotifyGive(Recognition_Handle);	//将信号发给Recognition
		}
	}

}
/*
*******************************颜色识别************************************
启动颜色识别模块的TCS34725的IIC传输，识别两次是为了防止当求还没落下来时误判了球的颜色，
如果两次值差别较大，则重新识别一次，否则取两次的平均值
实际上可以只判断一次，因为模块识别十分精确，如需提升速度可以省略二次识别
*/
void Recognition_task(void *pvParameters){
	uint16_t val=0;
	HC Get1_hc,Get2_hc,Send_hc;
	TCS34725_I2C_Start();
	while(1){
		val=ulTaskNotifyTake(pdTRUE,2);
		if(val){
			vTaskDelay(20);
			Get1_hc=Color_recognition();
			vTaskDelay(10);
			Get2_hc=Color_recognition();
			vTaskDelay(5);
			if(Get1_hc.h-Get2_hc.h>15||Get2_hc.h-Get1_hc.h>15){
				Send_hc=Color_recognition();
			}
			else{
				Send_hc.h=(Get1_hc.h+Get2_hc.h)/2;
				Send_hc.c=(Get1_hc.c+Get2_hc.c)/2;
			}
			xQueueSendToBack(xQueueHC,&Send_hc,pdMS_TO_TICKS(5));
			xTaskNotifyGive(Judge_Handle);

		}
	}
}
/*
****************************颜色索引判断**********************
调用Send_Judge，通过传来的颜色数值判断球对应的是1-5哪个数字
*/
void Judge_task(void *pvParameters){
	uint16_t val=0;
	uint16_t Send_judge;
	HC Receive_hc;
	while(1){
		val=ulTaskNotifyTake(pdTRUE,2);
		if(val){
			xQueueReceive(xQueueHC,&Receive_hc,pdMS_TO_TICKS(5));//从队列里读取数据
			Send_judge=Color_Judge(Receive_hc);
			xQueueSendToBack(xQueueIndex,&Send_judge,pdMS_TO_TICKS(5));//把颜色序号信息送入队列
			xTaskNotifyGive(Select_Handle);
		}
	}
}
/*
**************************************颜色分选********************************************
如果收到的Judge信号非0，那么判断到了通道内确实有球需要分选，否则即为此次通道内无求，
若无球则需重新执行下落等一系列子任务。
若有球，先判断Judge信号是否与position中的index值有对应，如果有对应的，则之前这个颜色
的球已经落下过，在转盘中已经有它对应的位置，那么把这个球也投到这个位置，并且quantity++
（在代码中，quantity实际并没有使用到，可以将它作为同一颜色的球是否都已经完全落下的辅助判断）
如果没有对应的index值，说明这个球是一个“新球”，需要找一个离初始位置最近的新位置来放置这个球

current和past两个变量记录的是现在所在的转盘位置和上次放球的转盘位置，做差值即可计算
转动延时时间，545这个值是安装后测试的舵机初始位置，建议后续使用时将其宏定义，否则改动较麻烦
*/
void Select_task(void *pvParameters){
	uint16_t val=0,timer;
	static uint16_t current=0;
	static uint16_t	past=0;
	uint16_t Receive_judge,cnt;
	while(1){
		val=ulTaskNotifyTake(pdTRUE,2);
		if(val){
			xQueueReceive(xQueueIndex,&Receive_judge,pdMS_TO_TICKS(5));//从队列里读取信息
			if(Receive_judge!=0){
				for(cnt=0;cnt<=4;cnt++){
					if(Receive_judge==position[cnt].index) break;
				}
				current=cnt;
				if(cnt<5){
					position[cnt].quantity++;
					TIM_SetCompare2(TIM2,545+cnt*400);
					timer=(current-past>=0)?current-past:past-current;
				}
				else{
					for(cnt=0;cnt<=4;cnt++){
						if(position[cnt].index==0){
							position[cnt].index=Receive_judge;
							position[cnt].quantity++;
							TIM_SetCompare2(TIM2,545+cnt*400);
							timer=(current-past>=0)?current-past:past-current;
							break;
						}
					}
				}
			}
			else{
			}
				vTaskDelay(timer*30+15);
				past=current;
				Color_Drop();
				vTaskDelay(50);
			xTaskNotifyGive(Drop_Handle);
		}
	}
}
/*
************************************巡线*********************************************
if中GREY_1为灰度传感器最左侧第一路，如果第一路为0，那么到了直角转弯的转角，向前移动后，
原地逆时针转动，知道第七路为0，判断转正
else中调用巡线函数进行循迹
*/
void GoLine_task(void *pvParameters){
	while (1)
	{
		if(GREY_1==0){
			Route_Straight();
			vTaskDelay(30);
			Route_Stop();
			vTaskDelay(20);
			while(GREY_7!=0){
				Route_TurnCorner();
			}
		}
		else{
			Route_GoLine();
		}
	}
}
/*
**********************转弯（未使用）*******************
可以将转弯单独分为一个子任务，如果任务切片需要可以采用
void Corner_task(void *pvParameters){
	TickType_t xLastWakeTime;	
	uint16_t val;
	while(1){
		val=ulTaskNotifyTake(pdTRUE,pdMS_TO_TICKS(5));
		if(val){
			Route_Straight();
			xLastWakeTime = xTaskGetTickCount();
			vTaskDelayUntil(&xLastWakeTime,15);
			Route_TurnCorner();	
			xLastWakeTime = xTaskGetTickCount();
			vTaskDelayUntil(&xLastWakeTime,800);		
		}
		xLastWakeTime = xTaskGetTickCount();
		vTaskDelayUntil(&xLastWakeTime,10);
	}
}
*/
/***********模块初始化*************/
void __init__(void){
	NVIC_Configuration();
	delay_init();	//延时函数初始化
	LED_Init();		//LED灯初始化	OK
	Motor_Configure(); 	//电机驱动初始化
	GREY_Configure();		//灰度传感器初始化	OK
	TCS34725_Init();		//颜色识别传感器初始化	OK
	TIM5_PWM_Init(19999,71);		//所有舵机初始化 OK
	TIM2_PWM_Init(19999,71);
	UART4_init(115200);	//左测距波特率
	UART5_init(115200);	//前测距波特率
	USART1_Init(9600);	//语音播报波特率
	usart_init2(9600);	//扫码器波特率	
	GPIO_Config();	//JAGT disable 
	TCS34725_Init();
	UART5_Start();
	USART2_Stop();
	UART4_Stop();
}
/*
**********************************停车*************************************************
第一次停车时，拉起后续需要的子任务，然后进行逐级减速，停车后发送队列信息，进行分球操作
如果是第二次停车，那么可以删除分球的子任务（因为在之前到达第一个货站之前已经全部分选完成）
后续进行平移投球即可
*/
void Stop_task(void *pvParameters){
	uint16_t val;
	static uint16_t	cnt=0;
	while(1){
	val=ulTaskNotifyTake(pdTRUE,pdMS_TO_TICKS(5));
		if(val){
			cnt++;
			if(cnt==1){
				vTaskResume(Movein_Handle);
				vTaskResume(Moveout_Handle);
				vTaskResume(QR_Handle);
				vTaskResume(Put_Handle);
				Route_FourthGear();
				vTaskDelay(15);
				Route_ThirdGear();
				vTaskDelay(15);				
				Route_SecondGear();
				vTaskDelay(15);
				Route_FirstGear();
				vTaskDelay(15);
				Route_GoLineLow();
				vTaskDelay(90);
				Route_Stop();
				vTaskDelay(250);
				xTaskNotifyGive(Drop_Handle);
				vTaskDelay(2500);			
				xTaskNotifyGive(Forward_Handle);
				
			}
			else if(cnt<=7){
				if(cnt==2){
					vTaskDelete(Drop_Handle);
					vTaskDelete(Recognition_Handle);
					vTaskDelete(Judge_Handle);
					vTaskDelete(Select_Handle);
				}				
				Route_GoLineLow();
				vTaskDelay(25);
				Route_Stop();
				xTaskNotifyGive(Movein_Handle);
				
			}
			vTaskSuspend(NULL);
		}
	}
}
/*
****************************向货站平移*********************************
第一路灰度检测到白线后继续平移一段距离，开启USART2，为扫码数据传输做准备
*/
void Movein_task(void *pvParameters){
	uint16_t val;
	while(1){
		val=ulTaskNotifyTake(pdTRUE,pdMS_TO_TICKS(5));
		if(val){
			while(GREY_1!=0){
				Route_Movein();
			}
			vTaskDelay(30);
			Route_Stop();
			vTaskDelay(70);
			USART2_Start();
			xTaskNotifyGive(QR_Handle);
		}
	}
}
/*
*************************************扫码********************************************
调用QR函数，如果扫到的码数值和上一次扫到的相同或者值为6，那么再次进行扫码确认扫码结果
*/
void QR_task(void *pvParameters){
	TickType_t xLastWakeTime;	
	extern u8 uart_buf[32];
	uint16_t val,Send_QR,past_QR;
	while(1){
		val=ulTaskNotifyTake(pdTRUE,pdMS_TO_TICKS(5));
		if(val){
			Send_QR=QR();
			while(Send_QR==6||Send_QR==past_QR){
				Send_QR=QR();
			}
			past_QR=Send_QR;
			USART2_Stop();
			xQueueSendToBack(xQueueQR,&Send_QR,pdMS_TO_TICKS(5));
			xTaskNotifyGive(Put_Handle);
			vTaskSuspend(QR_Handle);	
		}

		xLastWakeTime = xTaskGetTickCount();
		vTaskDelayUntil(&xLastWakeTime,100);
	}
}
/*
****************************************投球**************************************
如果接收到的扫码器信号为0，那么无需投球直接播报并撤出
如果非0，那么调用put让转盘转到对应位置，调用open开启转盘门，然后close关闭结束投球
*/
void Put_task(void *pvParameters){	
	uint16_t val,Receive_QR,cnt;
	unsigned short Zero_QR;
	while(1){
		val=ulTaskNotifyTake(pdTRUE,pdMS_TO_TICKS(5));
		if(val){
			xQueueReceive(xQueueQR,&Receive_QR,pdMS_TO_TICKS(5));
			if(Receive_QR==0){
				vTaskResume(Speak_Handle);
				Zero_QR=(unsigned short)Receive_QR;
				xQueueSendToBack(xQueueSpeak,&Zero_QR,pdMS_TO_TICKS(5));
				xTaskNotifyGive(Speak_Handle);
				vTaskDelay(350);
			}
			else{
				for(cnt=0;cnt<=4;cnt++){
					if(Receive_QR==position[cnt].index){
						break;
					}
				}
				vTaskResume(Speak_Handle);
				xQueueSendToBack(xQueueSpeak,&position[cnt].index,pdMS_TO_TICKS(5));
				xTaskNotifyGive(Speak_Handle);
				Put(cnt);
				vTaskDelay(100);
				Open();
				vTaskDelay(280);
				Close();			
			}
			xTaskNotifyGive(Moveout_Handle);
		}
	}
}
/*
************************************远离货站平移*******************************************
一直向外平移，直到第七路灰度检测到了白线，判断回到了赛道中
如果判断到第六次移出，那么认为六个货站（包含空货站）均已经投放完毕，则删除掉不需要的子任务
*/
void Moveout_task(void *pvParameters){	
	uint16_t val,cnt;
	while(1){
		val=ulTaskNotifyTake(pdTRUE,pdMS_TO_TICKS(5));
		if(val){
			cnt++;
			while(GREY_7!=0){
				Route_Moveout();
			}	
			Route_Stop();
			vTaskResume(Goline_Handle);
			if(cnt==6){
				UART4_Start();
				UART5_Stop();
				vTaskDelete(TOF_Handle);
				vTaskDelete(Stop_Handle);
				vTaskResume(Wave_Handle);
				vTaskResume(Avoid_Handle);
			}
			xTaskNotifyGive(Forward_Handle);
		}
	}
}
/*
*********************停车后启动**********************
停车完成后，重新启动侧边测距、扫码、巡线和停车的子任务
*/
void Forward_task(void *pvParameters){
	uint16_t val;
	while(1){
		val=ulTaskNotifyTake(pdTRUE,pdMS_TO_TICKS(5));
		if(val){
			vTaskResume(TOF_Handle);
			vTaskResume(Stop_Handle);	
			vTaskResume(QR_Handle);
			vTaskResume(Goline_Handle);
		}
	}

}
/*
**************************避障*****************************
接收到前方测距子任务发来的信号后利用Movein和Moveout进行避障
*/
void Avoid_task(void *pvParameters){
	TickType_t xLastWakeTime;	
	uint16_t val;
	while(1){
		val=ulTaskNotifyTake(pdTRUE,pdMS_TO_TICKS(5));
		if(val){
			Route_Moveout();
			vTaskDelay(200);
			Route_Straight();
			vTaskDelay(250);
			while(GREY_6!=0){
				Route_Movein();
			}
			vTaskResume(GoLineNew_Handle);
			vTaskDelete(Wave_Handle);
			vTaskDelete(Avoid_Handle);
		}
		xLastWakeTime = xTaskGetTickCount();
		vTaskDelayUntil(&xLastWakeTime,20);
	}
}
/*
*******************语音播报********************************
*/
void Speak_task(void *pvParameters){
	TickType_t xLastWakeTime;	
	unsigned short tag;
	uint16_t val;
	while(1){
		val=ulTaskNotifyTake(pdTRUE,pdMS_TO_TICKS(5));
		if(val){
			xQueueReceive(xQueueSpeak,&tag,pdMS_TO_TICKS(5));
			switch(tag){
				case 0:
					SYN_FrameInfo(0,"[v16][m0][t5]此站无邮件");
					break;
				case 1:
					SYN_FrameInfo(0,"[v16][m0][t5]1货站投放成功");
					break;
				case 2:
					SYN_FrameInfo(0,"[v16][m0][t5]2货站投放成功");
					break;
				case 3:
					SYN_FrameInfo(0,"[v16][m0][t5]3货站投放成功");
					break;
				case 4:
					SYN_FrameInfo(0,"[v16][m0][t5]4货站投放成功");
					break;
				case 5:
					SYN_FrameInfo(0,"[v16][m0][t5]5货站投放成功");
					break;	
			}
			vTaskSuspend(NULL);
		}
		xLastWakeTime = xTaskGetTickCount();
		vTaskDelayUntil(&xLastWakeTime,200);
	}	
	
}
/*
************************************二版巡线************************************
在过完跷跷板后，原来的巡线功能过于不能顾及返回出发区，因此在过完跷跷板后使用带有
Gohome功能的巡线子任务
*/
void GoLineNew_task(void *pvParameters){
	while(1)
	{
		if(GREY_1==0){
			xTaskNotifyGive(Gohome_Handle);
			vTaskDelete(NULL);
		}
		else{
			Route_GoLine();
		}
	}
}
/*
******************返回出发区*********************
*/
void Gohome_task(void *pvParameters){
	uint16_t val;
	while(1){
		val=ulTaskNotifyTake(pdTRUE,pdMS_TO_TICKS(5));
		if(val){
			Route_Start();
			vTaskDelay(70);
			Route_Straight();
			vTaskDelay(80);
			Route_Moveout();	
			vTaskDelay(80);
			Route_Stop();
			vTaskSuspendAll();
		}
	}
	
}
/*做测试用子任务*/
void Test_task(void *pvParameters){
	Route_Movein();
}
int main(void)
{
	__init__();
	
	printf("FreeRTOS test\n");
	prvSetupHardware();
	xQueueHC=xQueueCreate(1,sizeof(HC));							//RGB和HSL传输队列
	xQueueIndex=xQueueCreate(1,sizeof(uint16_t));			//颜色对应数字传输队列
	xQueueTOF=xQueueCreate(1,sizeof(uint16_t));				//激光测距传输队列
	xQueueQR=xQueueCreate(1,sizeof(uint16_t));				//二维码测距传输队列
	xQueueSpeak=xQueueCreate(1,sizeof(unsigned short));			//语音播报传输队列
	
	xTaskCreate(RervoReset_task,"ServoReset",1024,NULL,3,NULL);
	xTaskCreate(Start_task,"Start",128,NULL,3,NULL);
	xTaskCreate(Drop_task,"Drop",128,NULL,1,&Drop_Handle);
	xTaskCreate(Recognition_task,"Recognition",256,NULL,1,&Recognition_Handle);
	xTaskCreate(Judge_task,"Judge",128,NULL,1,&Judge_Handle);
	xTaskCreate(Select_task,"Select",128,NULL,1,&Select_Handle);
	xTaskCreate(GoLine_task,"GoLine",128,NULL,1,&Goline_Handle);	
	xTaskCreate(Stop_task,"Stop",128,NULL,1,&Stop_Handle);
	xTaskCreate(Movein_task,"Movein",128,NULL,1,&Movein_Handle);
	xTaskCreate(QR_task,"QR",128,NULL,1,&QR_Handle);
	xTaskCreate(Put_task,"Put",128,NULL,1,&Put_Handle);
	xTaskCreate(Speak_task,"Speak",128,NULL,2,&Speak_Handle);
	xTaskCreate(Moveout_task,"Moveout",128,NULL,1,&Moveout_Handle);	
	xTaskCreate(Forward_task,"Forward",128,NULL,1,&Forward_Handle);
	xTaskCreate(Avoid_task,"Avoid",128,NULL,2,&Avoid_Handle);
	xTaskCreate(TOF_task,"TOF",128,NULL,2,&TOF_Handle);
	xTaskCreate(Wave_task,"Wave",128,NULL,2,&Wave_Handle);
	xTaskCreate(GoLineNew_task,"GoLineNew",128,NULL,1,&GoLineNew_Handle);
	xTaskCreate(Gohome_task,"Gohome",128,NULL,1,&Gohome_Handle);
	
  vTaskStartScheduler();
}



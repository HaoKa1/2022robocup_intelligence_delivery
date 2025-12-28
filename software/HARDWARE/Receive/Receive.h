#ifndef __RECEIVE_H
#define __RECEIVE_H

#include "stm32f10x_exti.h"
#include "Route.h"
#include "uart4.h"
#include "color.h"


extern uint16_t A_distance;
void Receive_search(void);

#endif

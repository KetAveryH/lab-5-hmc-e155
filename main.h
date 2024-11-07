// main.h
// Josh Brake
// jbrake@hmc.edu
// 10/31/22 

#ifndef MAIN_H
#define MAIN_H

#include "STM32L432KC.h"
#include "STM32L432KC_TIM.h"
#include <stm32l432xx.h>

///////////////////////////////////////////////////////////////////////////////
// Custom defines
///////////////////////////////////////////////////////////////////////////////


#define INTERRUPT_A PB1
#define INTERRUPT_B PB4

#define COUNTER_TIM TIM6

#endif // MAIN_H
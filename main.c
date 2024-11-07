// button_interrupt.c
// Josh Brake
// jbrake@hmc.edu
// 10/31/22

#include "STM32L432KC.h"
#include "STM32L432KC_TIM.h"
#include <stm32l432xx.h>

///////////////////////////////////////////////////////////////////////////////
// Custom defines
///////////////////////////////////////////////////////////////////////////////


#define INTERRUPT_A PB1
#define INTERRUPT_B PB4

#define COUNTER_TIM TIM6


volatile uint32_t MSCOUNT = 0;
volatile uint32_t rotating = 0;
volatile int32_t  A_INTR_COUNT = 0;
volatile int32_t  B_INTR_COUNT = 0;
volatile double  RPS = 0.0f;
volatile int direction = 0;

int main(void) {
    // Enable GPIO Port B
    gpioEnable(GPIO_PORT_B);

    // Enable PB1 Interrupt_A as Input
    pinMode(INTERRUPT_A, GPIO_INPUT);
    GPIOB->PUPDR |= _VAL2FLD(GPIO_PUPDR_PUPD1, 0b01); // Set PB1 as pull-up

    // Enable PB4 Interrupt_B as Input
    pinMode(INTERRUPT_B, GPIO_INPUT);
    GPIOB->PUPDR |= _VAL2FLD(GPIO_PUPDR_PUPD4, 0b01); // Set PB4 as pull-up

    // Initialize timer
    RCC->APB1ENR1 |= _VAL2FLD(RCC_APB1ENR1_TIM2EN, 1); // Enable TIM2 clock
    RCC->APB1ENR1 |= _VAL2FLD(RCC_APB1ENR1_TIM6EN, 1); // Enable TIM6 clock
    initTIM(TIM2);
    init_pico_TIM(COUNTER_TIM);                   //initialize counter timer TIM6

    // Enable SYSCFG clock domain in RCC
    RCC->APB2ENR |= _VAL2FLD(RCC_APB2ENR_SYSCFGEN, 1);

    // Configure EXTICR for the input button interrupt in SYSCFG 9.2.4

    // Pin B1 Set Mux
    SYSCFG->EXTICR[0] &= ~_VAL2FLD(SYSCFG_EXTICR1_EXTI1, 0b111);
    SYSCFG->EXTICR[0] |= _VAL2FLD(SYSCFG_EXTICR1_EXTI1, 0b001);

    // Pin B4 Set Mux
    SYSCFG->EXTICR[1] &= ~_VAL2FLD(SYSCFG_EXTICR2_EXTI4, 0b111);
    SYSCFG->EXTICR[1] |= _VAL2FLD(SYSCFG_EXTICR2_EXTI4, 0b001);

    // Enable interrupts globally
    __enable_irq();

    // Configure interrupt for falling edge of GPIO pin for button

    // Configure mask bit
    EXTI->IMR1 |= _VAL2FLD(EXTI_IMR1_IM1, 1); // Unmask EXTI line 1
    EXTI->IMR1 |= _VAL2FLD(EXTI_IMR1_IM4, 1); // Unmask EXTI line 4

    // Event mask bit (optional, if needed)
    EXTI->EMR1 |= _VAL2FLD(EXTI_EMR1_EM1, 1); // Enable event for EXTI line 1
    EXTI->EMR1 |= _VAL2FLD(EXTI_EMR1_EM4, 1); // Enable event for EXTI line 4

    // Enable rising edge trigger
    EXTI->RTSR1 |= _VAL2FLD(EXTI_RTSR1_RT1, 1); // Rising trigger for EXTI line 1
    EXTI->RTSR1 |= _VAL2FLD(EXTI_RTSR1_RT4, 1); // Rising trigger for EXTI line 4

    // Enable falling edge trigger
    //EXTI->FTSR1 |= _VAL2FLD(EXTI_FTSR1_FT1, 1); // Falling trigger for EXTI line 1
    //EXTI->FTSR1 |= _VAL2FLD(EXTI_FTSR1_FT4, 1); // Falling trigger for EXTI line 4

    // Turn on EXTI interrupt in NVIC
    NVIC_EnableIRQ(EXTI1_IRQn);
    NVIC_EnableIRQ(EXTI4_IRQn);

    

    while(1){   
        delay_millis(TIM2, 1);

        if (COUNTER_TIM->CNT > 50) {
          rotating = 0;
        }

        if (rotating == 0) {
          RPS = 0.0f;
        } else {
          if (direction == 1) {

            if (MSCOUNT != 0) {
              RPS = (((float) 2.0f / 120.0f) / (float) MSCOUNT) * 1000.0f;
            }

            printf("B_INTR_COUNT %d\n", B_INTR_COUNT);
            printf("Rotations Per Second: %fl \n", RPS);
          }
          if (direction == -1) {

            if (MSCOUNT != 0) {
              RPS = (((float) 2.0f / 120.0f) / (float) MSCOUNT) * 1000.0f;
            }

            printf("Rotations Per Second: %fl \n", RPS);
          } 
        }
      
    }

}



void EXTI1_IRQHandler(void) {
    // Current status of A and B encoder signals
    int A_encoder = (GPIOB->IDR >> 1) & 0b1; 
    int B_encoder = (GPIOB->IDR >> 4) & 0b1;

    if (EXTI->PR1 & (1 << 1)) { // Check if EXTI line 1 triggered the interrupt
        // Handle the interrupt for EXTI1
        rotating = 1;
        if (A_encoder && B_encoder) {
          MSCOUNT = COUNTER_TIM->CNT;
          direction = 1;
          COUNTER_TIM->CNT = 0;
        }
        
        //printf("Direction: %d\n", direction); 
        //direction = 0;
        EXTI->PR1 |= (1 << 1); // Clear the pending flag for EXTI1
    }
}

void EXTI4_IRQHandler(void) {
    // Current status of A and B encoder signals
    int A_encoder = (GPIOB->IDR >> 1) & 0b1; 
    int B_encoder = (GPIOB->IDR >> 4) & 0b1;

    if (EXTI->PR1 & (1 << 4)) { // Check if EXTI line 4 triggered the interrupt
        // Handle the interrupt for EXTI4
        rotating = 1;
        
         if (A_encoder && B_encoder) {
          MSCOUNT = COUNTER_TIM->CNT;
          direction = -1;
          COUNTER_TIM->CNT = 0;
        }

        //printf("Direction: %d\n", direction); 
        //direction = 0;
        EXTI->PR1 |= (1 << 4); // Clear the pending flag for EXTI4
    }
}
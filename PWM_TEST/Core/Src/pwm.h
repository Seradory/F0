/*
 * pwm.h
 *
 *  Created on: Apr 3, 2024
 *      Author: alise
 */
#include "main.h"

#ifndef SRC_PWM_H_
#define SRC_PWM_H_


extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim14;


 void start_pwm_m(uint32_t tick_25us)
 {
     // PWM frekansı ve duty cycle ayarları
	  htim3.Instance->ARR = tick_25us - 1;
	  htim3.Instance->CCR2=2;
	  //sConfigOC.Pulse = tick/2;

     // PWM başlatma
     HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  }

  // PWM durdurma fonksiyonu
 void stop_pwm_m() {
	  //HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
	  HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
  }

 void start_uart_timer()
  {
  	htim6.Instance->CR1=0;
  	htim6.Instance->SR=0;
  	htim6.Instance->CNT=0;
  	htim6.Instance->DIER=1;
  	htim6.Instance->CR1=1;

  }

 void start_int_timer(TIM_HandleTypeDef *timer,int count)
 {
	 timer->Instance->CR1=0;
	 timer->Instance->SR=0;
	 timer->Instance->CNT=1;
	 timer->Instance->ARR=count;
	 timer->Instance->DIER=1;
	 timer->Instance->CR1=1;

 }

 void stop_int_timer(TIM_HandleTypeDef *timer)
 {
	 timer->Instance->CR1=0;
	 timer->Instance->SR=0;
	 timer->Instance->CNT=1;
	 timer->Instance->DIER=1;

 }

 void enable_triac_int()
 {
	 HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);
 };
 void disable_triac_int()
 {
	 HAL_NVIC_DisableIRQ(EXTI0_1_IRQn);
 };
#endif /* SRC_PWM_H_ */

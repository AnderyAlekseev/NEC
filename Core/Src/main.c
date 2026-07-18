/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "NEC/nec_Rx.h"
#include "NEC/nec_Tx.h"
/* Private includes ----------------------------------------------------------*/
#include "stm32f0xx_it.h"
#include <stdio.h>
#define LOG_UART       0
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

#define REC_ADDR       0
#define SEND_ADDR      7
#define INDX_REC       0
#define INDX_SEND      1
// the first column is the received command
// the second column is the one being sent
const uint8_t compliance_table[][2]={
  {69,   21},//MUTE
  {70,   67},// on/off
  {71,   70},// mode, 2.0 or 5.1
  
  {21,   86},// volume+
  {7,    82},// volume-
  {12,   7},// sub_lvl+
  {8,    3},// sub_lvl-
  {24,   6},// center_lvl+
  {28,   2},// center_lvl-
  {94,   4},// surround_lvl+
  {90,   0},// surround_lvl-
  
};



uint8_t get_send_comm(uint8_t rec_comm)
{
  uint8_t comm=0;
  uint8_t nm_rows = sizeof(compliance_table)/sizeof(compliance_table[0]);
  do{
    nm_rows--;
    if(compliance_table[nm_rows][INDX_REC] == rec_comm){
      comm = compliance_table[nm_rows][INDX_SEND];
      break;  
    }
  }while(nm_rows);
  return comm;
}

void test_coom()
{
   uint8_t nm_rows = sizeof(compliance_table)/sizeof(compliance_table[0]);
   uint8_t com =0;
   for(uint8_t i=1; i<=nm_rows; i++){
    com = get_send_comm(i);
   }
}
/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
  NVIC_SetPriority(SysTick_IRQn, 3);
  SystemClock_Config();
  SysTick_Config(32000);
  MX_GPIO_Init();
 
  NEC_RX_Input_Init();
  NEC_RX_TimerInit();
  NEC_TX_OutInit();
  NEC_TX_TimerInit();
  
#if (LOG_UART)
  MX_USART1_UART_Init();
#endif

  while (1)
  {
    
    if( NEC_RX_IsComplete() )
    {
        #if (LOG_UART)
        UART_Printf("Address %d %sCommand %d\r\n", NEC_RX_Get_Address(), NEC_RX_IsRepeat()?"Repeat ":"", NEC_RX_Get_Command());
        #endif
  
        //__ starting the transmitter
        if(NEC_RX_Get_Address() == REC_ADDR)
        {// if it is addressed to this device
          uint8_t send_comm = get_send_comm(NEC_RX_Get_Command());
          NEC_TX_SendCommand(SEND_ADDR, send_comm,  NEC_RX_IsRepeat());
   
        }
        NEC_RX_CompleteReset(); // сбрасываем вконце, т.к. нужно сначала обработать флаг repeat, а потом сбрасывать
    }
    NEC_TX_Proced();
    #if (LOG_UART)
    UART_debug_mess_Handle();
    #endif
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);
  while(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_0)  {
  }
  LL_RCC_HSI_Enable();

   /* Wait till HSI is ready */
  while(LL_RCC_HSI_IsReady() != 1)  {
  }
  LL_RCC_HSI_SetCalibTrimming(16);
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI_DIV_2, LL_RCC_PLL_MUL_12);
  LL_RCC_PLL_Enable();

   /* Wait till PLL is ready */
  while(LL_RCC_PLL_IsReady() != 1)  {
  }
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)  {
  }
  LL_Init1msTick(48000000);
  LL_SetSystemCoreClock(48000000);
  LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK1);
}

/* USER CODE BEGIN 4 */

uint8_t timeIsOver(uint32_t time_now, uint32_t time_stop)
{
  return (time_now >= time_stop ? 1 : 0);
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

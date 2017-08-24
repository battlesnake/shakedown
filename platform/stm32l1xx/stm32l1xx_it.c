/**
  ******************************************************************************
  * @file    Project/STM32L1xx_StdPeriph_Templates/stm32l1xx_it.c 
  * @author  MCD Application Team
  * @version V1.2.0
  * @date    16-May-2014
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <FreeRTOS.h>
#include "stm32l1xx_it.h"
#include <core_cm3.h>
//#include <semphr.h>
#include "main.h"
//#include <stdio.h>
#include <stm32xxxx_adc.h>
#include <stm32xxxx_dma.h>
#include <stm32xxxx_tim.h>

extern void vTaskADC(void);

/*
SemaphoreHandle_t xADCSemaphore;

void vISRCreateADCSemaphore(const UBaseType_t uxCounterSize, SemaphoreHandle_t *pxSemaphoreHandle)
{
	xADCSemaphore = xSemaphoreCreateCounting(uxCounterSize, 0);
	*pxSemaphoreHandle = xADCSemaphore;
}
*/
/** @addtogroup Template_Project
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
/*void NMI_Handler(void)
{
}*/

void prvGetRegistersFromStack( uint32_t *pulFaultStackAddress )
{
/* These are volatile to try and prevent the compiler/linker optimising them
away as the variables never actually get used.  If the debugger won't show the
values of the variables, make them global my moving their declaration outside
of this function. */
volatile uint32_t r0;
volatile uint32_t r1;
volatile uint32_t r2;
volatile uint32_t r3;
volatile uint32_t r12;
volatile uint32_t lr; /* Link register. */
volatile uint32_t pc; /* Program counter. */
volatile uint32_t psr;/* Program status register. */

    r0 = pulFaultStackAddress[ 0 ];
    r1 = pulFaultStackAddress[ 1 ];
    r2 = pulFaultStackAddress[ 2 ];
    r3 = pulFaultStackAddress[ 3 ];

    r12 = pulFaultStackAddress[ 4 ];
    lr = pulFaultStackAddress[ 5 ];
    pc = pulFaultStackAddress[ 6 ];
    psr = pulFaultStackAddress[ 7 ];

    __asm__("BKPT");
    
    //printf("r0 = %d, r1 = %d, r2 = %d, r3 = %d, r12 = %d, lr = %d, pc = %d, psr = %d.\n", (int)r0, (int)r1, (int)r2, (int)r3, (int)r12, (int)lr, (int)pc, (int)psr);

    /* When the following line is hit, the variables contain the register values. */
    for( ;; );
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
	__asm volatile
    (
        " tst lr, #4                                                \n"
        " ite eq                                                    \n"
        " mrseq r0, msp                                             \n"
        " mrsne r0, psp                                             \n"
        " ldr r1, [r0, #24]                                         \n"
        " ldr r2, handler2_address_const                            \n"
        " bx r2                                                     \n"
        " handler2_address_const: .word prvGetRegistersFromStack    \n"
    );
	
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
/*void SVC_Handler(void)
{
}*/

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
/*void PendSV_Handler(void)
{
}*/

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
/*void SysTick_Handler(void)
{
}*/

/******************************************************************************/
/*                 STM32L1xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32l1xx_xx.s).                                            */
/******************************************************************************/
/**
  * @brief  This function handles the TIM1 Capture Compare interrupt request.
  * @param  None
  * @retval None
  */

void TIM2_IRQHandler(void)
{
       	//BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	//__asm__("BKPT");
	
	//printf("CNT register at address 0x%08x = 0x%08x\n", (int)&TIM2->CNT, (int)TIM2->CNT);
	
       	// Check if the interrupt source
       	if(TIM_GetITStatus(TIM2, TIM_IT_CC2))
	{		
       	       	// Clear the interrupt flag
		TIM_ClearITPendingBit(TIM2, TIM_IT_CC2);		
      	}

	// Perform a context switch to the highest priority task
        //portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
  * @brief  This function handles ADC interrupt request.
  * @param  None
  * @retval None
  */

void ADC1_IRQHandler(void)
{
       	//BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	__asm__("BKPT");
	
	//printf("enter ADC interrupt handler\n");
	
       	// Check if the interrupt source is the ADC end of conversion
       	if(ADC_GetITStatus(ADC1, ADC_IT_EOC))
	{		
       	       	// Clear the interrupt flag
		ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
		//printf("EOC interrupt\n");
                // Give a semaphore to the ADC
       	       	//xSemaphoreGiveFromISR(xADCSemaphore, &xHigherPriorityTaskWoken);
      	}

	//printf("exiting ADC interrupt handler\n");
  
        // Perform a context switch to the highest priority task
        //portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void DMA1_Channel1_IRQHandler(void)
{
       	//BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	//ADC1 0x40012000
	
	// Clear half transfer interrupt
	if(DMA_GetITStatus(DMA1_IT_HT1))
	{		
       	       	// Clear the interrupt flag
		DMA_ClearITPendingBit(DMA1_IT_HT1);
	}
       	// Check if the interrupt source is transfer complete
	else if(DMA_GetITStatus(DMA1_IT_TC1))
	{		
       	       	// Clear the interrupt flag
		DMA_ClearITPendingBit(DMA1_IT_TC1);

		// Give the semaphore
       	       	//xSemaphoreGiveFromISR(xADCSemaphore, &xHigherPriorityTaskWoken);

		vTaskADC();
      	}

        /* Perform a context switch to the highest priority task */
        //portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

void WWDG_IRQHandler(void)
{
	__asm__("BKPT");
}

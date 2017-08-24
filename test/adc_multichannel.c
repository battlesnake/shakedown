#include <test.h>
#include <stm32xxxx_adc.h>
#include <stm32xxxx_rcc.h>
#include <stm32xxxx_gpio.h>
#include <stm32xxxx_tim.h>
#include <stm32xxxx_dma.h>
#include <config.h>

#define BUFFER_SIZE 11

/* 
 * The ADC semaphore is created in stm32l1xx_it.c as it is used from the ISR.
 * Obtain here a references to the semaphore. 
 */
//SemaphoreHandle_t xADCSemaphore;

static void vADCStart(void);
static uint16_t normalization(uint16_t v);
static void clocksInit(void);
static void GPIOInit(void);
static void TIMInit(TIM_TypeDef* TIMx);
static void PWMInit(TIM_TypeDef* TIMx);
static void ADCInit(ADC_TypeDef* ADCx);
static void NVICInit(void);
static void DMAInit(void);
void vTaskADC(void);

uint16_t adcBuffer[BUFFER_SIZE];
uint16_t mvBuffer[BUFFER_SIZE];

TEST_DEFINE(adc_multichannel)
{
	vADCStart();
	
	//test_assert("Calibration Pass", 5 != 0);
	//test_assert("Timer Sync", 5 != 0);
	//test_assert("DMA Buffer Check", 5 != 0);
	//test_assert("Data Acquired", 5 != 0);
}

/* Start the ADC by creating the required tasks and initialize the resources */
void vADCStart(void)
{
        clocksInit();

        GPIOInit();

	TIMInit(TIM2);
	
	NVICInit();

	DMAInit();
	
	PWMInit(TIM4);
	
	ADCInit(ADC1);
	
	for ( ;; );
}

/* Convert the raw 12 bits ADC reading in mV */
static uint16_t normalization(uint16_t v)
{
	uint32_t voltage;

	voltage = (uint32_t) v;
	voltage *= 3000;
	
        return (voltage >> 12); 
}

static void clocksInit()
{
	/* Enable ADC clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	// Enable HSI clock
	RCC_HSICmd(ENABLE);

	/* Enable the clock for the ADC GPIOs */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE);

	/* Enable the PWM GPIO Clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOD, ENABLE);

	/* Enable the DMA1 Clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
	/* Enable the clock for the timer used to trigger the ADC conversion */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	/* Enable the clock for the PWM */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);	
}
 
static void GPIOInit()
{
	GPIO_InitTypeDef GPIO_InitStruct;

	// Configure the pins for the PWM
	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOD, &GPIO_InitStruct);

	
	
	// Configure the pin AF muxing for the OC PWM
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource13, GPIO_AF_TIM4); // TIM4_CH2
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_TIM4); // TIM4_CH3
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_TIM4); // TIM4_CH4

	// Configure the pins of the analog inputs
	GPIO_StructInit(&GPIO_InitStruct); // ADC9 ADC10 ADC1 ADC2
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;
	GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	GPIO_StructInit(&GPIO_InitStruct); // ADC3 ADC4
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;
	GPIO_Init(GPIOB, &GPIO_InitStruct);

	GPIO_StructInit(&GPIO_InitStruct); // ADC5 ADC6 ADC7 ADC8
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;
	GPIO_Init(GPIOE, &GPIO_InitStruct);

	GPIO_StructInit(&GPIO_InitStruct); // BATT_V_SENS
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
}

// Specific for STM32L1xx
static void PWMInit(TIM_TypeDef* TIMx)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
        TIM_OCInitTypeDef TIM_OCInitStruct;
       
	/* Configure the timer for the triggers */
	TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
	TIM_TimeBaseInitStruct.TIM_Period = 0x03E8;	// 1000 -> the timer overflow will be 1 KHz
	TIM_TimeBaseInitStruct.TIM_Prescaler = 0x00017; // 24MHz / 23 + 1 (0x18) =  1 MHz
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIMx, &TIM_TimeBaseInitStruct);

	/* Configure the output capture */
	TIM_OCStructInit(&TIM_OCInitStruct);
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    	TIM_OCInitStruct.TIM_Pulse = 0x01F4; // 500 -> 500/1000 -> 50% duty cycle
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High; 
	TIM_OC2Init(TIMx, &TIM_OCInitStruct); // Enable chanel 2, 3 & 4 with the same settings
	
	TIM_OC2PreloadConfig(TIMx, TIM_OCPreload_Enable);

    	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    	TIM_OCInitStruct.TIM_Pulse = 0x01F4;
	TIM_OC3Init(TIMx, &TIM_OCInitStruct);
	
	TIM_OC3PreloadConfig(TIMx, TIM_OCPreload_Enable);

      	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    	TIM_OCInitStruct.TIM_Pulse = 0x01F4;
	TIM_OC4Init(TIMx, &TIM_OCInitStruct);
	
	TIM_OC4PreloadConfig(TIMx, TIM_OCPreload_Enable);

	TIM_ARRPreloadConfig(TIMx, ENABLE);
	
	TIM_Cmd(TIMx, ENABLE);
}

static void NVICInit()
{
	NVIC_InitTypeDef NVIC_InitStruct;

	/* Enable Timer 2 global interrupt */
	//NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;
	//NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	//NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 4;
	//NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	//NVIC_Init(&NVIC_InitStruct);
	
	/* Enable ADC global interrupt */
	//NVIC_InitStruct.NVIC_IRQChannel = ADC1_IRQn;
	//NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	//NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 4;
	//NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	//NVIC_Init(&NVIC_InitStruct);

	/* Enable DMA global interrupt */
	NVIC_InitStruct.NVIC_IRQChannel = DMA1_Channel1_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 5;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
}

static void DMAInit()
{
	DMA_InitTypeDef DMA_InitStruct;
	
	/* Configure the DMA channel */
	DMA_StructInit(&DMA_InitStruct);
	DMA_InitStruct.DMA_PeripheralBaseAddr = (int)&ADC1->DR; // platform dependent
	DMA_InitStruct.DMA_MemoryBaseAddr = (int)adcBuffer;
	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;
	DMA_Init(DMA1_Channel1, &DMA_InitStruct);
	
	DMA_SetCurrDataCounter(DMA1_Channel1, BUFFER_SIZE);

	/* Enable DMA local interrupt */
	DMA_ITConfig(DMA1_Channel1, DMA_IT_HT, ENABLE);
	DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);

	/* Enable the DMA */
	DMA_Cmd(DMA1_Channel1, ENABLE);
}

static void ADCInit(ADC_TypeDef* ADCx)
{
	ADC_InitTypeDef ADC_InitStruct;
	__IO uint32_t Timeout = 0;
	
        /* Configure the ADC Prescaler, conversion resolution and data 
	   alignment */
	ADC_StructInit(&ADC_InitStruct);
	ADC_InitStruct.ADC_ContinuousConvMode = DISABLE; // Disable for multichannel
	ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
	ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_CC2;
	ADC_InitStruct.ADC_ScanConvMode = ENABLE; // ENABLE for multichannel
	ADC_InitStruct.ADC_NbrOfConversion = 11;
	ADC_Init(ADCx, &ADC_InitStruct);
	ADC_RegularChannelConfig(ADCx, ADC_Channel_1, 1, ADC_SampleTime_48Cycles);    // VBAT  PA1
	ADC_RegularChannelConfig(ADCx, ADC_Channel_14, 2, ADC_SampleTime_48Cycles);   // ADC1  PC4
	ADC_RegularChannelConfig(ADCx, ADC_Channel_15, 3, ADC_SampleTime_48Cycles);   // ADC2  PC5
	ADC_RegularChannelConfig(ADCx, ADC_Channel_8, 4, ADC_SampleTime_48Cycles);  // ADC3  PB0
	ADC_RegularChannelConfig(ADCx, ADC_Channel_9, 5, ADC_SampleTime_48Cycles);  // ADC4  PB1
        ADC_RegularChannelConfig(ADCx, ADC_Channel_22, 6, ADC_SampleTime_48Cycles); // ADC5  PE7
        ADC_RegularChannelConfig(ADCx, ADC_Channel_23, 7, ADC_SampleTime_48Cycles); // ADC6  PE8
        ADC_RegularChannelConfig(ADCx, ADC_Channel_24, 8, ADC_SampleTime_48Cycles); // ADC7  PE9
        ADC_RegularChannelConfig(ADCx, ADC_Channel_25, 9, ADC_SampleTime_48Cycles); // ADC8  PE10
	ADC_RegularChannelConfig(ADCx, ADC_Channel_12, 10, ADC_SampleTime_48Cycles); // ADC9  PC2
	ADC_RegularChannelConfig(ADCx, ADC_Channel_13, 11, ADC_SampleTime_48Cycles); // ADC10 PC3

        ADC_DMARequestAfterLastTransferCmd(ADCx, ENABLE);
	
	/* Enable DMA requests */
	ADC_DMACmd(ADCx, ENABLE);
	
	/* Enable EOC local interrupt */		
	ADC_ITConfig(ADCx, ADC_IT_EOC, ENABLE); // Not required, just for test
	
	// Enable the ADC only after the ADONS is set to 0
	//Timed(!ADC_GetFlagStatus(ADCx, ADC_FLAG_ADONS));
	
	/* Activate peripheral */
	ADC_Cmd(ADCx, ENABLE);

	//ADC_ResetCalibration(ADCx);
	//while(ADC_GetResetCalibrationStatus(ADCx));
	//ADC_StartCalibration(ADCx);
	//while(ADC_GetCalibrationStatus(ADCx));
	
	ADC_SoftwareStartConv(ADCx); // just for test
}

static void TIMInit(TIM_TypeDef* TIMx)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
        TIM_OCInitTypeDef TIM_OCInitStruct;
	
	// Configure the timer for the triggers
	TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
	TIM_TimeBaseInitStruct.TIM_Period = 0x7A120;
	TIM_TimeBaseInitStruct.TIM_Prescaler = 0x00019;
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIMx, &TIM_TimeBaseInitStruct);

	// Configure the output capture
	TIM_OCStructInit(&TIM_OCInitStruct);
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_Toggle;
    	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    	TIM_OCInitStruct.TIM_Pulse = 0x7A120;
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_Low; 
	TIM_OC2Init(TIMx, &TIM_OCInitStruct);
	TIM_CCxCmd(TIMx, TIM_Channel_2, TIM_CCx_Enable);
	
	//TIM_OC2PreloadConfig(TIMx, TIM_OCPreload_Enable);
	//TIM_ARRPreloadConfig(TIMx, ENABLE);

	//TIM_ITConfig(TIMx, TIM_IT_CC2, ENABLE); // Not required, just for test
	
	TIM_Cmd(TIMx, ENABLE);
}

/* Acquiring the analog values after the ADC conversion is done */
void vTaskADC(void)
{
	for(int i = 0; i < BUFFER_SIZE; i++)
	{
	        mvBuffer[i] = normalization(adcBuffer[i]);
	        //printf("analog at address 0x%08x = %dmV.\r\n", (int)(adcBuffer + i), (int)fAnalogValue);
	}

	//__asm__("BKPT");
}



 

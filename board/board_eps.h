#include <FreeRTOS.h>
#include <stm32xxxx_gpio.h>
#include <stm32xxxx_rcc.h>

#ifndef OBDH_BOARD_SPEC
#define OBDH_BOARD_SPEC 1

#define I2C1_GPIOClockEnable(A, B) RCC_AHBPeriphClockCmd(A, B)
#define I2C1_GPIO_Clock RCC_AHBPeriph_GPIOB
#define I2C1_GPIO GPIOB
#define I2C1_GPIO_Pin1 GPIO_PinSource8
#define I2C1_GPIO_Pin2 GPIO_PinSource9
#define I2C1_PIN_SCL GPIO_Pin_8
#define I2C1_PIN_SDA GPIO_Pin_9
#define I2C1_GPIO_SPEED GPIO_Speed_40MHz

#define I2C1_ClockEnable(A, B) RCC_APB1PeriphClockCmd(A, B);
#define I2C1_Clock RCC_APB1Periph_I2C1

#endif

/**
  ******************************************************************************
  * @file    main.c
  * @author  Arjun
  * @version V1.0
  * @date    31-May-2020
  * @brief   Control Led with on-board button using FreeRTOS on a Nucleo-64
  ******************************************************************************
*/

#include<stdio.h>
#include<stdint.h>
#include<string.h>

#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"

// Global constants
#define NOT_PRESSED 0
#define PRESSED ~NOT_PRESSED

// Function declaration
static void prvSetupHW(void);
static void prvSetupUart(void);
static void prvSetupGpio(void);
static void prvSetupInterrupt(void);
void prvPrintMsg(const char *message);
void led_task(void *parameters);
void button_task(void *parameters);

// Function declaration
TaskHandle_t led_task_handle = NULL;
TaskHandle_t button_task_handle = NULL;

//Global variables
uint8_t status_flag = NOT_PRESSED;


// Driver funtion
int main(void)
{
	// 1. Resets the RCC clock configuration to the default reset state.
	// Set the System clock = 16 MHz -> CPU_clock = 16MHz
	RCC_DeInit();

	// 2. Update the SystemCoreClock
	SystemCoreClockUpdate();

	prvSetupHW();

	xTaskCreate(led_task, "LED-task", configMINIMAL_STACK_SIZE, NULL, 3, &led_task_handle);

	vTaskStartScheduler();

	for(;;);
}

/**
  * @brief  A task to control on-board LED (LD2)
  *
  * @param  void pointer to parameters structure
  *
  * @retval None
  */
void led_task(void *parameters)
{
	while(1)
	{
		if(status_flag == NOT_PRESSED)
		{
			// Turn OFF LED
			GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_RESET);

		}else
		{
			// Turn ON LED
			GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_SET);
		}
	}

}

/**
  * @brief  Task to toggle LED control
  *
  * @param  void pointer to parameters structure
  *
  * @retval None
  */
void button_task(void *parameters)
{
	//Toggle LED
	status_flag ^= 1;

}

/**
  * @brief  Utility function to print string using USART
  *
  * @param  constant string
  *
  * @retval None
  */
void prvPrintMsg(const char *message)
{
	for (uint32_t i = 0; i < strlen(message); i++)
	{
		while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) != SET);
		USART_SendData(USART2, message[i]);
	}
}

/**
  * @brief  A private function that initilizes USART
  *
  * @param  None
  *
  * @retval None
  */
static void prvSetupUart(void)
{

	USART_InitTypeDef uart_init;

	// 1. Enable Peripheral clock for UART2
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	// UART initialize
	memset(&uart_init, 0, sizeof(uart_init));
	uart_init.USART_BaudRate = 115200;
	uart_init.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	uart_init.USART_Parity = USART_Parity_No;
	uart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	uart_init.USART_StopBits = USART_StopBits_1;
	uart_init.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART2, &uart_init);

	// Enable USART
	USART_Cmd(USART2, ENABLE);
}

/**
  * @brief  A private function that initilizes GPIO
  *
  * @param  None
  *
  * @retval None
  */
static void prvSetupGpio(void)
{
	GPIO_InitTypeDef gpio_uart_init, led_init, button_init;

	// Enable GPIO clock
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	// Enable GPIO ports
	// UART2_TX = PA2
	// UART2_RX = PA3
	// Alternate func configured to behave as Tx/Rx
	memset(&gpio_uart_init, 0, sizeof(gpio_uart_init));
	gpio_uart_init.GPIO_Mode = GPIO_Mode_AF;
	gpio_uart_init.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	gpio_uart_init.GPIO_OType = GPIO_OType_PP;
	gpio_uart_init.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &gpio_uart_init);

	// UART Tx/Rx
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2); // PA2
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2); // PA3

	// Clock already enabled
	// Configure Led PA5
	memset(&led_init, 0, sizeof(led_init));
	led_init.GPIO_Mode = GPIO_Mode_OUT;
	led_init.GPIO_OType = GPIO_OType_PP;
	led_init.GPIO_Pin = GPIO_Pin_5;
	led_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
	led_init.GPIO_Speed = GPIO_Low_Speed;

	GPIO_Init(GPIOA, &led_init);

	// Enable Clock
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	// Configure Button PC5
	memset(&button_init, 0, sizeof(button_init));
	button_init.GPIO_Mode = GPIO_Mode_IN;
	button_init.GPIO_Pin = GPIO_Pin_13;
	button_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
	button_init.GPIO_Speed = GPIO_Low_Speed;

	GPIO_Init(GPIOC, &button_init);

}

/**
  * @brief  A private function that initilizes
  * 		on-board user button Interrupt (B1)
  *
  * @param  None
  *
  * @retval None
  */
static void prvSetupInterrupt(void)
{
	EXTI_InitTypeDef exti_init;

	// Enable Clock fir SYSCONFIG
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	// Interrupt Configuration for Button (PC13)
	// System Config
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource13);


	// Interrupt Mode, Falling Edge, PC13
	memset(&exti_init, 0, sizeof(exti_init));
	exti_init.EXTI_Line = EXTI_Line13;
	exti_init.EXTI_Mode = EXTI_Mode_Interrupt;
	exti_init.EXTI_Trigger = EXTI_Trigger_Falling;
	exti_init.EXTI_LineCmd = ENABLE;

	EXTI_Init(&exti_init);

	// Enable NVIC for IRQ and set priority
	NVIC_SetPriority(EXTI15_10_IRQn, 5);
	NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* @brief  A private function that initilizes STM32F4 Nucleo-F446RE
 *
 * @param  None
 *
 * @retval None
 */
static void prvSetupHW(void)
{
	prvSetupGpio();
	prvSetupUart();
	prvSetupInterrupt();
}

/**
  * @brief  EXTI Interrupt Handler that controls LED
  *
  * @param  void pointer to parameters structure
  *
  * @retval None
  */
void EXTI15_10_IRQHandler(void)
{
	// Clear the int pending bit of EXT (13)
	EXTI_ClearITPendingBit(EXTI_Line13);
	button_task(NULL);
}


/*********************** Unused ******************/
//void button_task(void *parameters)
//{
//	while(1)
//	{
//		// Button is Active High
//		if (!GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13))
//		{
//			// Button is pressed
//			status_flag = PRESSED;
//		}else
//		{
//			// Button is not pressed
//			status_flag = NOT_PRESSED;
//		}
//	}
//
//}

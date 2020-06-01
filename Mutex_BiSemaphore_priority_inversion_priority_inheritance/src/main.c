/**
  ******************************************************************************
  * @file    main.c
  * @author  Arjun
  * @version V1.0
  * @date    31-May-2020
  * @brief   Implement Binary Semaphore and Mutex
  * 		 Demonstrate Priority Inversion and Priority
  * 		 Inheritance using Mutex
  *
  * 		 Demo on STM32F4 Nucleo-F446RE dev board
  ******************************************************************************
*/


#include<stdio.h>
#include<stdint.h>
#include<string.h>

#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"


// Global variables
char user_msg[250] = {0};

// Function declaration
static void prvSetupHW(void);
static void prvSetupUart(void);
static void prvSetupGpio(void);
void prvPrintMsg(const char *message);
void HPT_Task(void *parameters);
void MPT_Task(void *parameters);
void LPT_Task(void *parameters);

// Synchronization handles
xSemaphoreHandle simple_mutex; // mutex
xSemaphoreHandle binary_semaphore;

// Function declaration
TaskHandle_t HPT_handle = NULL;
TaskHandle_t MPT_handle = NULL;
TaskHandle_t LPT_handle = NULL;

// Driver funtion
int main(void)
{
	// 1. Resets the RCC clock configuration to the default reset state.
	// Set the System clock = 16 MHz -> CPU_clock = 16MHz
	RCC_DeInit();

	// 2. Update the SystemCoreClock
	SystemCoreClockUpdate();

	prvSetupHW();

	binary_semaphore = xSemaphoreCreateBinary();
	if (binary_semaphore != NULL)
	{
		sprintf(user_msg, "Binary Semaphore Created successfully \r\n");
		prvPrintMsg(user_msg);
	}
	xSemaphoreGive(binary_semaphore);

	simple_mutex = xSemaphoreCreateMutex();
	if(simple_mutex != NULL)
	{
		sprintf(user_msg,"Mutex Created successfully \r\n");
		prvPrintMsg(user_msg);
	}

	xTaskCreate(HPT_Task, "High Priority task", configMINIMAL_STACK_SIZE, NULL, 3, &HPT_handle);
	xTaskCreate(MPT_Task, "Medium Priority task", configMINIMAL_STACK_SIZE, NULL, 2, &MPT_handle);
	xTaskCreate(LPT_Task, "Low Priority task", configMINIMAL_STACK_SIZE, NULL, 1, &LPT_handle);

	vTaskStartScheduler();

	for (;;);
}
/* Function that writes data to USART which
   is guarded bby Binary Semaphores */
void Send_Uart (uint8_t *message)
{
	xSemaphoreTake(binary_semaphore, portMAX_DELAY);
	vTaskDelay(5000);
	for (uint32_t i = 0; i < strlen(message); i++)
	{
		while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) != SET);
		USART_SendData(USART2, message[i]);
	}
	xSemaphoreGive(binary_semaphore);
}

/**
  * @brief  A High priority task sending data
  * 		to USART thru Send_Uart()
  *
  * @param  void pointer to parameters structure
  *
  * @retval None
  */
void HPT_Task(void *parameters)
{
	uint8_t *data = "============== HPT =============\r\n";
	while (1)
	{
		char *str = "I am HPT and going to take Semaphore\r\n";
		prvPrintMsg(str);

		Send_Uart(data);

		char *str2 = "Leaving HPT\r\n\n";
		prvPrintMsg(str2);

		vTaskDelay(750);
	}

}

/**
  * @brief  A Medium priority task just printing a string
  *
  * @param  void pointer to parameters structure
  *
  * @retval None
  */
void MPT_Task(void *parameters)
{
	char *str = "**************** MPT *****************\r\n";
	while(1)
	{
		prvPrintMsg(str);
		vTaskDelay(5000);
	}

}

/**
  * @brief  A Low priority task sending data
  * 		to USART thru Send_Uart()
  *
  * @param  void pointer to parameters structure
  *
  * @retval None
  */
void LPT_Task(void *parameters)
{
	uint8_t *data = "----------------- LPT -----------------\r\n";
	while (1)
	{
		char *str = "I am LPT and going to take Semaphore\r\n";
		prvPrintMsg(str);

		Send_Uart(data);

		char *str2 = "Leaving LPT\r\n\n";
		prvPrintMsg(str2);

		vTaskDelay(1000);
	}

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
	GPIO_InitTypeDef gpio_uart_init;

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

}

/**
  * @brief  A private function that initilizes STM32F4 Nucleo-F446RE
  *
  * @param  None
  *
  * @retval None
  */
static void prvSetupHW(void)
{
	prvSetupGpio();
	prvSetupUart();
}

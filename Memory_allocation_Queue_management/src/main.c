/**
  ******************************************************************************
  * @file    main.c
  * @author  Arjun
  * @version V1.0
  * @date    31-May-2020
  * @brief   A FreeRTOS application to demonstrate usage of Interrupts,
  * 		 Queue, Binary Semaphore, Heap memory management, Peripheral
  * 		 Usage (USART, GPIOs, Clocks) on a STM32F4 based uC
  ******************************************************************************
*/

#include "demo.h"
#include "demo_queue.h"
#include "sender_receiver.h"

xTaskHandle producer_handle = NULL;
xTaskHandle interrupt_handle = NULL;
xTaskHandle consumer_handle = NULL;

// driver funtion
int main(void)
{
	// Resets the RCC clock configuration to the default reset state.
	// Set the System clock = 16 MHz -> CPU_clock = 16MHz
	RCC_DeInit();

	// Update the SystemCoreClock
	SystemCoreClockUpdate();

	// Setup Hardware
	prvSetupHW();

	// Initialize the Demo Queue
	queue_init();

	// Create tasks
	xTaskCreate(producer_task, "Producer", 500, NULL, 2, &producer_handle);
	xTaskCreate(interrupt_producer_handler, "Interrupt producer", 1000, NULL, 2, &interrupt_handle);
	xTaskCreate(consumer_task, "Consumer", 500, NULL, 1, &consumer_handle);

	// Start Scheduler
	vTaskStartScheduler();

	for(;;);
}






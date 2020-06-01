#include "sender_receiver.h"
#include "demo.h"
#include "demo_queue.h"
#include "semphr.h"

// Global variables
uint32_t incr_val = 1;
uint32_t incr_val_1 = 1;

// Binary Semaphore
SemaphoreHandle_t xBiSemaphore;

/**
  * @brief  A producer task, sending data to Consumer using Queue.
  * 		Uses global variable incr_val to keep increasing value.
  * 		Periodic task with priority 2 handled by scheduler
  * @param  void pointer to parameters structure
  *
  * @retval None
  */

void producer_task(void *parameters)
{
	Queue_st *sender_st;
	uint32_t delay = pdMS_TO_TICKS(5000);
	char msg[100];

	while(1)
	{
		// Memory allocation
		sender_st = pvPortMalloc(sizeof(Queue_st));

		if (sender_st != NULL)
		{

			// Populate struct
			sender_st->value = incr_val * 10;
			sender_st->string = "producer_task sending data";

			xQueueSend(demo_queue_handler, &sender_st,portMAX_DELAY);

			sprintf(msg,
					"Producer task  addr: %p, sent data %ld to Queue\r\n\n",
					sender_st,
					sender_st->value);
			prvPrintMsg(msg);

			incr_val = incr_val + 1;
		}

		vTaskDelay(delay);
	}
}

/**
  * @brief  A producer task, that blocks on a Binary semaphore
  * 		until released by Interrupt on Button Press. Sends
  * 		data to Consumer using same Queue.
  * 		Uses global variable incr_val_1 to keep increasing value.
  *
  * @param  void pointer to parameters structure
  *
  * @retval None
  */
void interrupt_producer_handler(void *parameters)
{
	Queue_st  * int_sender_st;
	xBiSemaphore = xSemaphoreCreateBinary();
	char msg[100];
	while(1)
	{

		if (xSemaphoreTake(xBiSemaphore, portMAX_DELAY) == pdTRUE) {
			int_sender_st = pvPortMalloc(sizeof(Queue_st));

			int_sender_st->value = incr_val_1 * 20;
			int_sender_st->string = "Interrupt_Producer sending data";

			// Send data to Queue
			if (xQueueSend(demo_queue_handler, &int_sender_st,
					portMAX_DELAY) == pdPASS) {
				sprintf(msg,
						"Interrupt producer Sent addr: %p with value: %ld Successfully to Queue\r\n\n",
						int_sender_st,
						int_sender_st->value);
				prvPrintMsg(msg);
			}

			incr_val_1++;
		}
	}
}

/**
  * @brief  A Consumer task that blocks on a Queue and prints
  * 		received data to USART2
  *
  * @param  void pointer to parameters structure
  *
  * @retval None
  */
void consumer_task(void *parameters)
{
	Queue_st *receiver_st;
	uint32_t delay = pdMS_TO_TICKS(6000);

	while(1)
	{
		if(xQueueReceive(demo_queue_handler, &receiver_st, portMAX_DELAY) == pdPASS)
		{
			char *r_ptr = pvPortMalloc(100 * sizeof(char)); // Memory to print values

			sprintf(r_ptr,
					"Received from Queue: addr: %p Value: %ld String: %s\r\n\n",
					receiver_st,
					receiver_st->value,
					receiver_st->string);

			prvPrintMsg(r_ptr);

			vPortFree(r_ptr);
		}
		vPortFree(receiver_st);

		vTaskDelay(delay);
	}
}

/**
  * @brief  EXTI Interrupt Handler that gives a Binary
  * 		Semaphore so interrupt_producer can push
  * 		data into Queue
  *
  * @param  void pointer to parameters structure
  *
  * @retval None
  */
void EXTI15_10_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken;

	// Clear the int pending bit of EXT (13)
	EXTI_ClearITPendingBit(EXTI_Line13);

	// Initialize
	xHigherPriorityTaskWoken = pdFALSE;

	// Unblock the task by releasing the semaphore.
	xSemaphoreGiveFromISR(xBiSemaphore, &xHigherPriorityTaskWoken);

	// Yeild if True
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}





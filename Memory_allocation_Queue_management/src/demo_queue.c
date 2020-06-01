/*
 * Implement Queue
 */
#include "demo.h"
#include "demo_queue.h"

/**
  * @brief  Initialize Queue for the Demo
  *
  * @param  None
  *
  * @retval None
  */
void queue_init(void)
{
	// Init Queue
	demo_queue_handler = xQueueCreate(5, sizeof(Queue_st));

	if (demo_queue_handler != NULL) {
		char *str = "Queue successfully created\r\n\n";
		prvPrintMsg(str);
	} else {
		char *str = "Error while creating Queue\r\n\n";
		prvPrintMsg(str);
	}
}


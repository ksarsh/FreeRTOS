#include<stdint.h>

typedef struct Queue_struct
{
	uint32_t value;
	uint8_t *string;
} Queue_st;

void queue_init(void);

QueueHandle_t demo_queue_handler;

#include<stdio.h>
#include<stdint.h>
#include<string.h>
#include<stdint.h>

#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"


void prvPrintMsg(const char *message);
void prvSetupUart(void);
void prvSetupGpio(void);
void prvSetupInterrupt(void);
void prvSetupHW(void);

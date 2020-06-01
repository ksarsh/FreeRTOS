#include "demo.h"

/**
  * @brief  Utility to print data to USART
  *
  * @param  String message -> constant
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

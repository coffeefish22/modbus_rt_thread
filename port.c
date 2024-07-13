/* ----------------------- System includes --------------------------------*/

/* ----------------------- Modbus includes ----------------------------------*/
#include "port.h"
/* ----------------------- Variables ----------------------------------------*/

/* ----------------------- Start implementation -----------------------------*/
void EnterCriticalSection(void)
{
	//关闭全局中断
	__disable_irq();
}

void ExitCriticalSection(void)
{
	//开启全局中断
	__enable_irq();
}

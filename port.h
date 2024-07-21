#ifndef _PORT_H
#define _PORT_H

#include <stm32f10x_conf.h>
#include "mbconfig.h"
#include <rthw.h>
#include <rtthread.h>

#include <assert.h>
#include <inttypes.h>

#include "globalval.h"

#define	INLINE
#define PR_BEGIN_EXTERN_C           extern "C" {
#define	PR_END_EXTERN_C             }

#define COVER_DELAY		60000

#if (USE_SERIAL_X_FOR_SLAVE== 3)
//TODO  暂时先写B13引脚，等组网测试时再确认
#define SLAVE_RS485_SEND_MODE  do{GPIO_SetBits(GPIO_RS485_COM_SLAVE,PIN_RS485_COM_SLAVE);}while(0)//rt_hw_us_delay(200000);
#define SLAVE_RS485_RECEIVE_MODE 	do{GPIO_ResetBits(GPIO_RS485_COM_SLAVE,PIN_RS485_COM_SLAVE);}while(0)//rt_hw_us_delay(10000);
#else
#define SLAVE_RS485_SEND_MODE  
#define SLAVE_RS485_RECEIVE_MODE 

#endif
#define MASTER_RS485_SEND_MODE  GPIO_SetBits(GPIO_RS485_COM,PIN_RS485_COM)
#define MASTER_RS485_RECEIVE_MODE  GPIO_ResetBits(GPIO_RS485_COM,PIN_RS485_COM)

#define ENTER_CRITICAL_SECTION()	EnterCriticalSection()
#define EXIT_CRITICAL_SECTION()    ExitCriticalSection()

void EnterCriticalSection(void);
void ExitCriticalSection(void);

typedef uint8_t BOOL;

typedef unsigned char UCHAR;
typedef char    CHAR;

typedef uint16_t USHORT;
typedef int16_t SHORT;

typedef uint32_t ULONG;
typedef int32_t LONG;

#ifndef TRUE
#define TRUE            1
#endif

#ifndef FALSE
#define FALSE           0
#endif

#endif

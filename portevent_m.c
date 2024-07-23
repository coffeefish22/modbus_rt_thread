/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

#if MB_MASTER_RTU_ENABLED > 0 || MB_MASTER_ASCII_ENABLED
/* ----------------------- Variables ----------------------------------------*/
static eMBMasterEventType eMasterQueuedEvent;
static BOOL     xMasterEventInQueue;

/* ----------------------- Start implementation -----------------------------*/
BOOL
xMBMasterPortEventInit( void )
{
    xMasterEventInQueue = FALSE;
    return TRUE;
}

BOOL
xMBMasterPortEventPost( eMBMasterEventType eEvent )
{
    xMasterEventInQueue = TRUE;
    eMasterQueuedEvent = eEvent;
    return TRUE;
}

BOOL
xMBMasterPortEventGet( eMBMasterEventType * eEvent )
{
    BOOL            xEventHappened = FALSE;

    if( xMasterEventInQueue )
    {
        *eEvent = eMasterQueuedEvent;
        xMasterEventInQueue = FALSE;
        xEventHappened = TRUE;
    }
    return xEventHappened;
}

#endif

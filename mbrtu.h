#include "mbconfig.h"
#ifndef _MB_RTU_H
#define _MB_RTU_H

#ifdef __cplusplus
PR_BEGIN_EXTERN_C
#endif
eMBErrorCode    eMBRTUInit( UCHAR slaveAddress, UCHAR ucPort, ULONG ulBaudRate,
						    eMBParity eParity );
void            eMBRTUStart( void );
void            eMBRTUStop( void );
eMBErrorCode    eMBRTUReceive( UCHAR * pucRcvAddress, UCHAR ** pucFrame, USHORT * pusLength );
eMBErrorCode    eMBRTUSend( UCHAR slaveAddress, const UCHAR * pucFrame, USHORT usLength );
BOOL            xMBRTUReceiveFSM( void );
BOOL            xMBRTUTransmitFSM( void );
BOOL            xMBRTUTimerT15Expired( void );
BOOL            xMBRTUTimerT35Expired( void );

#if MB_MASTER_RTU_ENABLED > 0
eMBErrorCode    eMBMasterRTUInit( UCHAR ucPort, ULONG ulBaudRate,eMBParity eParity );
void            eMBMasterRTUStart( void );
void            eMBMasterRTUStop( void );
eMBErrorCode    eMBMasterRTUReceive( UCHAR * pucRcvAddress, UCHAR ** pucFrame, USHORT * pusLength );
eMBErrorCode    eMBMasterRTUSend( UCHAR slaveAddress, const UCHAR * pucFrame, USHORT usLength );
BOOL            xMBMasterRTUReceiveFSM( void );
BOOL            xMBMasterRTUTransmitFSM( void );
BOOL            xMBMasterRTUTimerExpired( void );
#endif

#ifdef __cplusplus
PR_END_EXTERN_C
#endif
#endif

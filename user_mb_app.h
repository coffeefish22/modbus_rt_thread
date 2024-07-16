#ifndef	USER_APP
#define USER_APP

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mb_m.h"
#include "mbconfig.h"
//#include "mbframe.h"
#include "mbutils.h"
#include "globalval.h"
//#include "port.h"
/* -----------------------Slave Defines -------------------------------------*/
#define S_DISCRETE_INPUT_START        1
#define S_DISCRETE_INPUT_NDISCRETES   16
#define S_COIL_START                  1
#define S_COIL_NCOILS                 64
#define S_REG_INPUT_START             1
#define S_REG_INPUT_NREGS             100
#define S_REG_HOLDING_START           1
#define S_REG_HOLDING_NREGS           200
//从机模式：在保持寄存器中，各个地址对应的功能定义
#define          S_HD_RESERVE                     0		  //保留
#define          S_HD_CPU_USAGE_MAJOR             1         //当前CPU利用率的整数位
#define          S_HD_CPU_USAGE_MINOR             2         //当前CPU利用率的小数位

//从机模式：在输入寄存器中，各个地址对应的功能定义
#define          S_IN_RESERVE                     0		  //保留

//从机模式：在线圈中，各个地址对应的功能定义
#define          S_CO_RESERVE                     2		  //保留

//从机模式：在离散输入中，各个地址对应的功能定义
#define          S_DI_RESERVE                     1		  //保留

/* -----------------------Master Defines -------------------------------------*/
#define M_DISCRETE_INPUT_START        1
#define M_DISCRETE_INPUT_NDISCRETES   16
#define M_COIL_START                  1
#define M_COIL_NCOILS                 64
#define M_REG_INPUT_START             1
#define M_REG_INPUT_NREGS             100*2
#define M_REG_HOLDING_START           1
#define M_REG_HOLDING_NREGS           100*2
//主机模式：在保持寄存器中，各个地址对应的功能定义
#define          M_HD_RESERVE                     0		  //保留

//主机模式：在输入寄存器中，各个地址对应的功能定义
#define          M_IN_RESERVE                     0		  //保留

//主机模式：在线圈中，各个地址对应的功能定义
#define          M_CO_RESERVE                     2		  //保留

//主机模式：在离散输入中，各个地址对应的功能定义
#define          M_DI_RESERVE                     1		  //保留

     

/*------------------------Slave mode use these variables----------------------*/
//Slave mode:DiscreteInputs variables
extern USHORT   usSDiscInStart ;
#if S_DISCRETE_INPUT_NDISCRETES%8
extern UCHAR    ucSDiscInBuf[S_DISCRETE_INPUT_NDISCRETES/8+1];
#else
extern UCHAR    ucSDiscInBuf[S_DISCRETE_INPUT_NDISCRETES/8]  ;
#endif
//Slave mode:Coils variables
extern USHORT   usSCoilStart;
#if S_COIL_NCOILS%8
extern UCHAR    ucSCoilBuf[S_COIL_NCOILS/8+1]                ;
#else
extern UCHAR    ucSCoilBuf[S_COIL_NCOILS/8]                  ;
#endif
//Slave mode:InputRegister variables
extern USHORT   usSRegInStart ;
extern USHORT   usSRegInBuf[S_REG_INPUT_NREGS]               ;
//Slave mode:HoldingRegister variables
extern USHORT   usSRegHoldStart ;
extern USHORT   usSRegHoldBuf[S_REG_HOLDING_NREGS];
/*-----------------------Master mode use these variables----------------------*/
#if MB_MASTER_RTU_ENABLED > 0 || MB_MASTER_ASCII_ENABLED > 0
//Master mode:DiscreteInputs variables
extern USHORT   usMDiscInStart ;
#if      M_DISCRETE_INPUT_NDISCRETES%8
extern UCHAR    ucMDiscInBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_DISCRETE_INPUT_NDISCRETES/8+1];
#else
extern UCHAR    ucMDiscInBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_DISCRETE_INPUT_NDISCRETES/8];
#endif
//Master mode:Coils variables
extern USHORT   usMCoilStart ;
#if      M_COIL_NCOILS%8
extern UCHAR    ucMCoilBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_COIL_NCOILS/8+1];
#else
extern UCHAR    ucMCoilBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_COIL_NCOILS/8];
#endif
//Master mode:InputRegister variables
extern USHORT   usMRegInStart ;
extern USHORT   usMRegInBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_REG_INPUT_NREGS];
//Master mode:HoldingRegister variables
extern USHORT   usMRegHoldStart ;
extern USHORT   usMRegHoldBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_REG_HOLDING_NREGS];
#endif



#endif

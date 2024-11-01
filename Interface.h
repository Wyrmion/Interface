/**
  ******************************************************************************
  * @file    Interface.h
  * @author  Kukushkin A.V.
  * @brief   header file for Interface.c
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __INTERFACE_H__
#define __INTERFACE_H__


#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>
#include <stdbool.h>
#include "InterfacePrivate.h"

#define INTERFACE_MAX_PACK_LENG 256

#define INTERFACE_DATA_OFFSET   1


//typedef  IImInterfaceCmd_t InterfaceCmd_t;

#define ADDR_IN_PACK

typedef struct
{
    #ifdef ADDR_IN_PACK
      uint8_t   addr;
    #endif
    uint8_t   cmd;
    uint8_t   Data[INTERFACE_MAX_PACK_LENG];
    uint32_t  len;
}InterfaceCmdData_t;

typedef struct InterfaceHandel InterfaceHandel_t;

typedef enum
{
  kInterfeceProto_ClassicBStaff,
  kInterfaceProto_Time_Separete_MB
  //kInterfeceProto_ASCii,
}eInterfaceProtocol_t;

InterfaceHandel_t*  Interface_ctor(HWInterface_t* HwInter);
void                Interface_dtor(InterfaceHandel_t* hdev);

void                Interface_SetProtoAlgoritm(InterfaceHandel_t* phdev,eInterfaceProtocol_t algoritm);
bool                Interface_InstallCRCAlgoritm(InterfaceHandel_t* phdev,uint16_t(*CRCfunc)(const uint8_t*/*data*/,uint32_t /*len*/),bool CrcHeadCalc);

void                Interface_process(InterfaceHandel_t* hdev);
bool                Interface_isCMDdRDY(InterfaceHandel_t * hdev);
bool                Interface_readCMD(InterfaceHandel_t * hdev,InterfaceCmdData_t *payload);
bool                Interface_sendCMD(InterfaceHandel_t * hdev,InterfaceCmdData_t *payload);
bool                Interface_Connect(InterfaceHandel_t * phdev);

void                Interface_SetAddr(InterfaceHandel_t * phdev,uint32_t addr);
uint32_t            Interface_GetAddr(InterfaceHandel_t * phdev);

#ifdef __cplusplus
}
#endif

#endif
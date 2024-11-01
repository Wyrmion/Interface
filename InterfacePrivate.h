/**
  ******************************************************************************
  * @file    InterfacePrivate.h
  * @author  Kukushkin A.V.
  * @brief   header file for private Interface.c
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef INTERFACE_PRIVATE___H__
#define INTERFACE_PRIVATE___H__


#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>
#include <stdbool.h>

#include "stddef.h"


typedef uint16_t interface_crc_t;


typedef struct {
    bool (*ReadRxBuff)(void* /*this*/,uint8_t* /*data*/,size_t* /*len*/,size_t /*max len*/);
	void (*SetRxBuff)(void* /*this*/,uint8_t* /*data*/,size_t /*max len*/);
    void (*SetTxBuff)(void* /*this*/,uint8_t* /*data*/,size_t /*max len*/);
    bool (*SendData)(void* /*this*/,uint8_t* /*data*/,uint8_t /*len*/);
    bool (*IsFree)(void* /*this*/);
    void (*Process)(void*/*this*/);
    bool (*Connect)(void*/*this*/);
}HwInterface_vtable_t;



typedef struct {
    HwInterface_vtable_t *vtable;
}HWInterface_t;

#ifdef __cplusplus
}
#endif

#endif
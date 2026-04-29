/**
  ******************************************************************************
  * @file    Interface.h
  * @author  Kukushkin A.V.
  * @brief   header file for Interface.c
  * @version  V1.0.4
  * @date     29. Apr. 2026
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



#include "../Interface/InterfacePrivate.h"
#include "CRCInterface.h"

/**
 * @addtogroup Wyrm_Drivers Wyrm Drivers
 * @{
 */

/**
 * @defgroup Interface Interface
 * @{
 */

/* Private macro -------------------------------------------------------------*/
/**
 * @defgroup Interface_defines_group Interface defines 
 * @todo     should be move to cfg file
 * @{
 */
#define make_interface(parent,IntBuffSize,CircDeep) Interface_ctor((HWInterface_t*)parent,IntBuffSize,CircDeep)
#define INTERFACE_HEAD_SIZE sizeof(InterfaceCmdDataHead_t)
/** @}*/



typedef struct InterfaceHandel InterfaceHandel_t;       /*!< Interface Class typedef*/

/**
 * @brief Interface Rx Tx irq handel mode
 * 
 */
typedef enum
{
  kInterfaceRxTx_process, /*!<  Check DRDY flag from Rx HW and check 
                                is circbuff empty for tx in none process*/
  kInterfaceRxTx_irq,     /*!<  Rx HW calling the interface handler directly in IRQ (for fast response) */
}eInterfaceRxTxHandel_t;

/**
 * @brief Transmit complete/error callback to parent class 
 * @note  shoulde be set by parent class
 * 
 * @param parent  pointer to parent class
 * @param this    pointer to @ref InterfaceHandel_t
 */
typedef void (*ParentCbErrTx)(void* parent,InterfaceHandel_t* cthis);

/**
 * @brief Transmit complete/error callback to parent class 
 * @note  shoulde be set by parent class
 * 
 * @param parent  pointer to parent class
 * @param this    pointer to @ref InterfaceHandel_t
 * @param data    pointer to data 
 * @param len     size of rx data
 */
typedef void (*ParentCbRx)(void* parent,InterfaceHandel_t* cthis,uint8_t* data,size_t len);

typedef struct 
{
  void* parent;
  ParentCbRx    RxCb;
  ParentCbErrTx TxCb;
  ParentCbErrTx ErrCb;
}sInterfaceIrqParentCB_t;


/**
 * @brief Protocol algoritm size_t func(uint8_t* dst,uint8_t* src,uint32_t size)
 * 
 * @param   dst   pointer to destination data buffer 
 * @param   src   pointer to destination data buffer 
 * @param   size  input data size
 * 
 * @return  total data after pack or unpack
 */
typedef size_t (*AlgoProto)(uint8_t* dst,const uint8_t* src,size_t size);

/**
 * @brief Rx Filter algoritmc bool func(uint8_t* src,size_t leng)
 * 
 * @param parent pointer to parent class
 * @param src    pointer to rx data
 * @param leng   leng of rx data
 */
typedef bool  (*RxFilter)(void* parent,const uint8_t* src,size_t leng);

/**
 * @brief Rx Filter class 
 * 
 */
typedef struct 
{
  void*     parent; /*!< Pointer to parent*/
  RxFilter  func;   /*!< Pointer to filter algoritm*/
}sInterfaceRxFilter_t;

/**
 * @defgroup Interface_public_func Interface public function
 * @{
 */
 /**
   * @defgroup Interface_public_ctor_dtor Interface constructor/destructor
   * @{
   */ 
  InterfaceHandel_t*  Interface_ctor(HWInterface_t* HwInter,size_t IntBuffSize,size_t CircDeep);
  void                Interface_dtor(InterfaceHandel_t* hdev);
  /** @}*/
  
  /**
   * @defgroup Interface_public_process Interface process function
   * @{
   */
  void                Interface_process(InterfaceHandel_t* cthis);
  /** @}*/
  
   /**
  * @defgroup Interface_public_link Interface link/install function
  * @{
  */  
  void                Interface_InstallProtoAlgoritm(InterfaceHandel_t* cthis,AlgoProto pack, AlgoProto unpack);
  bool                Interface_InstallCRCAlgoritm(InterfaceHandel_t* cthis,sCRCInterface_t* crc);
  bool                Interface_InstallFilter(InterfaceHandel_t* cthis,sInterfaceRxFilter_t* filter);

  bool                Interface_SetCB(InterfaceHandel_t* cthis,sInterfaceIrqParentCB_t* parentCB);
   /** @}*/
   
  
  /**
  * @defgroup Interface_public_basic Interface basic function
  * @{
  */
  void                Interface_SetMode(InterfaceHandel_t* cthis,const eInterfaceRxTxHandel_t mode);

  void                Interface_SetRawMode(InterfaceHandel_t* cthis,bool state);
  bool                Interface_isRawMode(InterfaceHandel_t* cthis);

  size_t              Interface_readData(InterfaceHandel_t* cthis,void *dst);
  size_t              Interface_readDataPtr(InterfaceHandel_t* cthis,uint8_t** dst);

  bool                Interface_SendData(InterfaceHandel_t* cthis,void *payload,size_t leng);
  bool                Interface_Send_cu8(InterfaceHandel_t* cthis,const uint8_t* data,size_t leng);
  bool                Interface_Send_str(InterfaceHandel_t* cthis,const char* str,size_t leng); 
  

  




  bool                Interface_Connect(InterfaceHandel_t* cthis);
  bool                Interface_Disconnect(InterfaceHandel_t* cthis);
  bool                Interface_IsTxFree(InterfaceHandel_t* cthis);
  bool                Interface_isRxNe(InterfaceHandel_t* cthis);
  
  size_t              Interface_GetMaxDatalng(InterfaceHandel_t* cthis);

  
  /** @}*/ 
/** @}*/

/** @}*/
/** @}*/
#ifdef __cplusplus
} 

inline bool Interface_Send(InterfaceHandel_t* p, void* d, size_t s)          { return Interface_SendData(p, d, s); }
inline bool Interface_Send(InterfaceHandel_t* p, const uint8_t* d, size_t s) { return Interface_Send_cu8(p, d, s); }
inline bool Interface_Send(InterfaceHandel_t* p, uint8_t* d, size_t s)       { return Interface_Send_cu8(p, d, s); }
inline bool Interface_Send(InterfaceHandel_t* p, const char* d, size_t s)    { return Interface_Send_str(p, d, s); }
inline bool Interface_Send(InterfaceHandel_t* p, char* d, size_t s)          { return Interface_Send_str(p, d, s); }
#else

#define Interface_Send(parent, src, size) _Generic((src), \
    const uint8_t*: Interface_Send_cu8, \
    uint8_t*:       Interface_Send_cu8, \
    const char*:    Interface_Send_str, \
    char*:          Interface_Send_str, \
    default:        Interface_SendData  \
)(parent, src, size)
#endif


#endif
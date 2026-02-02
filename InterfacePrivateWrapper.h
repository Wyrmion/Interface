/**
 ****************************************************************************
 * @file     InterfacePrivateWrapper.h
 * @author   Kukushkin A.V.
 * @brief    This file provides code for @ref HWInterface_t wrapper macro/functions
 * @version  V1.0.0
 * @date     30. Jan. 2024
 *************************************************************************
 */
 
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __INTERFACE_PRIVATE_WRAPPER_H__
#define __INTERFACE_PRIVATE_WRAPPER_H__


#ifdef __cplusplus
extern "C"{
#endif

#include "InterfacePrivate.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

//#include <string.h>
//#include <math.h>

/**
 * @addtogroup Interface_Private
 * @{
 */

/**
 * @defgroup Interface_Private_wrapper Interface private wrapper functions
 * @{
 */
 
/* Private macro -------------------------------------------------------------*/
#define CONVERT_TO_HW(this) ((HWInterface_t*)this)
 
/* Private typedef -----------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
#define INTERFACE_PRIVATE_WRAPPER_USE_MACRO
/* Private function prototypes -----------------------------------------------*/
#ifndef INTERFACE_PRIVATE_WRAPPER_USE_MACRO
    /** @defgroup Interface_Private_wrapper Interface @ref HWInterface_t functions wrapper
    * @{
    */
  static void HwEnterCritical(void* this_ptr,eCriticalSection Section);
  static void HwExitCritical( void* this_ptr,eCriticalSection Section);
  static void HwSetRxBuff(void* this_ptr,uint8_t* data,size_t max_len);
  static void HwSetTxBuff(void* this_ptr,uint8_t* data,size_t max_len);
  static void HwProcess(void* this_ptr);
  static bool HwSendData(void* this_ptr,uint8_t* data,uint8_t len);
  static bool HwIsFree(void* this_ptr);
  static bool HwConnect(void* this_ptr);
  static bool HwDisconnect(void* this_ptr);
  static void HwSetCB(void* this_ptr,sInterfaceIrqCallback_t* p_cb);
  /** @}*/ /* End of Interface @ref HWInterface_t functions wraper group*/
/* Exported functions -------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
 


/**
 * @brief   Wraper of @ref HWInterface_t read rx buffer 
 * 
 * @param   this_ptr  pointer to @ref HWInterface_t
 * @param   data      pointer to read data  
 * @param   len       size of read data     
 * @param   max_len   max data size         
 * @return  true      if data ready at this class
 * @return  false     if data unready
 */
static bool  HwReadRxBuff(void* this_ptr,uint8_t* data,size_t* len,size_t max_len)
{
  //HWInterface_t* HwInter = (HWInterface_t*)this_ptr;
  return(CONVERT_TO_HW(this_ptr)->vtable->ReadRxBuff(this_ptr,data,len,max_len));
}


/**
 * @brief Wraper of @ref HWInterface_t set external rx buffer pointer
 * 
 * @param this_ptr  pointer to @ref HWInterface_t
 * @param data      pointer to linked external rx buffer
 * @param max_len   size of external buffer
 */
static void HwSetRxBuff(void* this_ptr,uint8_t* data,size_t max_len)
{
  return(CONVERT_TO_HW(this_ptr)->vtable->SetRxBuff(this_ptr,data,max_len));
}

/**
 * @brief Wraper of @ref HWInterface_t set external tx buffer pointer
 * 
 * @param this_ptr  pointer to @ref HWInterface_t
 * @param data      pointer to linked external tx buffer
 * @param max_len   size of external buffer
 */
static void HwSetTxBuff(void* this_ptr,uint8_t* data,size_t max_len)
{	
 return(CONVERT_TO_HW(this_ptr)->vtable->SetTxBuff(this_ptr,data,max_len));
}

/**
 * @brief Wraper of @ref HWInterface_t HwProcess
 * 
 * @param this_ptr  pointer to @ref HWInterface_t
 */ 
static void HwProcess(void* this_ptr)
{
  CONVERT_TO_HW(this_ptr)->vtable->Process(this_ptr);
}

/**
 * @brief   Wraper of @ref HWInterface_t Connect function
 * @param   this_ptr  pointer to @ref HWInterface_t
 * @return  true      if everything ok
 */
static bool HwConnect(void* this_ptr)
{
  return CONVERT_TO_HW(this_ptr)->vtable->Connect(this_ptr);
}

/**
 * @brief   Wraper of @ref HWInterface_t Disconnect function
 * @param   this_ptr  pointer to @ref HWInterface_t
 * @return  true      if everything ok
 */
static bool HwDisconnect(void* this_ptr)
{
  return CONVERT_TO_HW(this_ptr)->vtable->Disconnect(this_ptr);
}



/**
 * @brief Wraper of @ref HWInterface_t SendData
 * 
 * @param this_ptr  pointer to @ref HWInterface_t
 * @param data      pointer to tx data
 * @param len       sizeof tx data
 * @return true     if packet move to send buffer
 * @return false    if buse or error
 */
static bool HwSendData(void* this_ptr,uint8_t* data,uint8_t len){
  return CONVERT_TO_HW(this_ptr)->vtable->SendData(this_ptr,data,len);
}

/**
 * @brief Wraper of @ref HWInterface_t IsFree
 * 
 * @param this_ptr  pointer to @ref HWInterface_t
 * @return true    if free
 * @return false   buse
 */
static bool HwIsFree(void* this_ptr){
  return CONVERT_TO_HW(this_ptr)->vtable->IsFree(this_ptr);
}

/**
 * @brief Wrapper of @ref HWInterface_t set callback function
 * 
 * @param this_ptr  pointer to @ref HWInterface_t
 * @param p_cb      pointer to @ref sInterfaceIrqCallback_t callback
 */
static void HwSetCB(void* this_ptr,sInterfaceIrqCallback_t* p_cb)
{
  CONVERT_TO_HW(this_ptr)->vtable->irqcb = p_cb;
}

#else /* #ifdef INTERFACE_PRIVATE_WRAPPER_USE_MACRO */

/**
 * @brief Macro variant of @ref EnterCriticalRx wrapper function
 * 
 * @param this_ptr pointer to @ref HWInterface_t
 * @param Section  from @ref  eCriticalSection
 */
#define HwEnterCriticalRx(this_ptr) CONVERT_TO_HW(this_ptr)->vtable->EnterCriticalRx(this_ptr)

/**
 * @brief Macro variant of @ref EnterCriticalTx wrapper function
 * 
 * @param this_ptr pointer to @ref HWInterface_t
 * @param Section  from @ref  eCriticalSection
 */
#define HwEnterCriticalTx(this_ptr) CONVERT_TO_HW(this_ptr)->vtable->EnterCriticalTx(this_ptr)


/**
 * @brief Macro variant of @ref ExitCritical wrapper function
 * 
 * @param this_ptr pointer to @ref HWInterface_t
 * @param Section  from @ref  eCriticalSection
 */
#define HwExitCriticalRx(this_ptr) CONVERT_TO_HW(this_ptr)->vtable->ExitCriticalRx(this_ptr)

/**
 * @brief Macro variant of @ref ExitCritical wrapper function
 * 
 * @param this_ptr pointer to @ref HWInterface_t
 * @param Section  from @ref  eCriticalSection
 */
#define HwExitCriticalTx(this_ptr) CONVERT_TO_HW(this_ptr)->vtable->ExitCriticalTx(this_ptr)

/**
 * @brief Macro variant of @ref SendData wrapper function
 * 
 * @param this_ptr  pointer to @ref HWInterface_t
 * @param data      pointer to tx data
 * @param len       sizeof tx data
 * @return true     if packet move to send buffer
 * @return false    if buse or error
 */
#define HwSendData(this_ptr,data,len) CONVERT_TO_HW(this_ptr)->vtable->SendData(this_ptr,data,len)



/**
 * @brief Macro variant of @ref HwProcess wrapper function
 * 
 * @param this_ptr  pointer to @ref HWInterface_t
 */ 
#define HwProcess(this_ptr) ((HWInterface_t*)(this_ptr))->vtable->Process(this_ptr)

/**
 * @brief   Macro variant of @ref Connect wrapper function
 * @param   this_ptr  pointer to @ref HWInterface_t
 * @return  true      if everything ok
 */
#define HwConnect(this_ptr) ((HWInterface_t*)(this_ptr))->vtable->Connect(this_ptr)

/**
 * @brief   Macro variant of @ref Disconnect wrapper function
 * @param   this_ptr  pointer to @ref HWInterface_t
 * @return  true      if everything ok
 */
#define HwDisconnect(this_ptr) ((HWInterface_t*)(this_ptr))->vtable->Disconnect(this_ptr)

/**
 * @brief macro variant of @ref SetTxBuff wrapper function
 * 
 * @param this_ptr  pointer to @ref HWInterface_t
 * @param data      pointer to linked external tx buffer
 * @param max_len   size of external buffer
 */

#define HwSetTxBuff(this_ptr,data,max_len) ((HWInterface_t*)(this_ptr))->vtable->SetTxBuff(this_ptr,data,max_len)

/**
 * @brief macro variant of @ref SetRxBuff wrapper function
 * @param this_ptr  pointer to @ref HWInterface_t
 * @param data      pointer to linked external rx buffer
 * @param max_len   size of external buffer
 */
#define HwSetRxBuff(this_ptr,data,max_len)  ((HWInterface_t*)(this_ptr))->vtable->SetRxBuff(this_ptr,data,max_len)

/**
 * @brief macro variant of @ref ReadRxBuff wrapper function
 * @param   this      pointer to @ref HWInterface_t
 * @param   data      pointer to read data  
 * @param   len       size of read data     
 * @param   max_len   max data size         
 */
#define HwReadRxBuff(this_ptr,data,len,max_len) CONVERT_TO_HW(this_ptr)->vtable->ReadRxBuff(this_ptr,data,len,max_len)

/**
 * @brief Macro variant of @ref IsHwFree wrapper function
 * 
 * @param this_ptr  pointer to @ref HWInterface_t
 * @return true    if free
 * @return false   buse
 */
#define HwIsFree(this_ptr) CONVERT_TO_HW(this_ptr)->vtable->IsFree(this_ptr)


/**
 * @brief Wrapper of @ref HWInterface_t set callback function
 * 
 * @param this_ptr  pointer to @ref HWInterface_t
 * @param p_cb      pointer to @ref sInterfaceIrqCallback_t callback
 */
#define HwSetCB(this_ptr,p_cb) CONVERT_TO_HW(this_ptr)->vtable->irqcb = p_cb


/**
 * @brief Wrapper of @ref HWInterface_t get max data leng function
 * 
 * @param this_ptr  pointer to @ref HWInterface_t
 * @return max data leng in size_t
 */
#define HwGetMaxDataLeng(this_ptr)  CONVERT_TO_HW(this_ptr)->vtable->GetMaxDataLeng(this_ptr)

#endif /* #ifdef INTERFACE_PRIVATE_WRAPPER_USE_MACRO */

#ifdef __cplusplus
}
#endif

#endif

/** @}*/ /*End of Interface private wrapper functions group*/
/** @}*/ /*End of Interface_Private group*/
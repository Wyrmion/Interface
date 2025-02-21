/**
  ******************************************************************************
  * @file    InterfacePrivate.h
  * @author  Kukushkin A.V.
  * @brief   header file for private Interface.c
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __INTERFACE_PRIVATE_H__
#define __INTERFACE_PRIVATE_H__


#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>
#include <stdbool.h>

#include "stddef.h"


/**
 * @addtogroup Interface
 * @{
 */

/**
 * @defgroup Interface_Private Interface abstract class
 * @{
 */

/**
 * @brief Interface critical section enum
 * 
 */
typedef enum
{
  eCritical_rx,     /*!< Rx     critical section*/
  eCritical_tx,     /*!< Tx     critical section*/
  eCritical_error,  /*!< Error  critical section*/
  eCritical_number_of_elem, /*!< elements number of this enumerate */
}eCriticalSection;

/**
 * @brief   Interface callback struct 
 * @details use it if you need notify parent in IRQ
 */
typedef struct 
{
  void* parent;
  void (*rx_cb) (void*/*parent*/,uint8_t* /*data*/,size_t /*len*/); /*!< Rx callback function (if you should answer in callback)*/
  void (*tx_cb) (void*/*parent*/); /*!< Tx callback function (if you should answer in callback)*/
  void (*err_cb)(void*/*parent*/); /*!< Error callback function*/
}sInterfaceIrqCallback_t;    

/**
 *  @brief Virtual function table struct
 */
typedef struct {

    void    (*SetRxBuff)(void* /*this*/,uint8_t* /*data*/,size_t /*max len*/);       /*!< Link external RX buffer */   
    void    (*SetTxBuff)(void* /*this*/,uint8_t* /*data*/,size_t /*max len*/);       /*!< Link external TX buffer */

    void    (*EnterCriticalRx)(void* /*this*/);
    void    (*ExitCriticalRx)(void* /*this*/);
    
    void    (*EnterCriticalTx)(void* /*this*/);
    void    (*ExitCriticalTx)(void* /*this*/);

    bool    (*Connect)(void*/*this*/);                                                          /*!< Connect */
    bool    (*Disconnect)(void*/*this*/);                                                       /*!< Disconnect */
    
    void    (*Process)(void*/*this*/);                                                          /*!< None blocking while(if needed)*/
        
    bool    (*IsFree)(void* /*this*/);                                                          /*!< Get interface state*/
    bool    (*SendData)(void* /*this*/,uint8_t* /*data*/,uint8_t /*len*/);                      /*!< Send data */
    bool    (*ReadRxBuff)(void* /*this*/,uint8_t* /*data*/,size_t* /*len*/,size_t /*max len*/); /*!< Read Rx buffer */
    size_t  (*GetMaxDataLeng)(void* /*this*/);
    
    sInterfaceIrqCallback_t *irqcb;
}HwInterface_vtable_t;


/**
 * @brief HWInterface interface class
 * 
 */
typedef struct {
    HwInterface_vtable_t *vtable;   /*!< virtual function table*/
}HWInterface_t;

/** @}*/
/** @}*/

#ifdef __cplusplus
}
#endif

#endif
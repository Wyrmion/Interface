/**
  ******************************************************************************
  * @file     InterfaceConf.h
  * @author   Kukushkin A.V.
  * @brief    Header file for Interface configuration file
  * @version  V1.0.1
  * @date     02. Nov. 2024
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __INTERFACE_CONF_H__
#define __INTERFACE_CONF_H__


#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <../Interface/InterfacePrivate.h>


/**
 * @addtogroup Interface 
 * @{
 */

/**
 * @defgroup Interface_config Interface config file
 * @{
 */

/* Public macro -------------------------------------------------------------*/
//#define INTERFACE_PROTOCOL_TIME_SEPARATION 1
#define INTERFACE_PROTOCOL_BYTESTAFF 2

#ifdef ADDR_IN_PACK
//  #define ADDR_MULTICAST_SUPPORT
  #ifdef ADDR_MULTICAST_SUPPORT
    #define MULTICAST_ADDR 0
  #endif
#endif

//#define LEN_IN_PACK
//#define CRC_IN_PACK 


#ifdef CRC_IN_PACK
  #define INTERFACE_CRC_SIZE 2
#else
  #define INTERFACE_CRC_SIZE 0
#endif

#ifndef USE_INTERFACE_MALLOC 
  
  #define INTERFACE_CNT 1



#ifndef USER_INTERFACE_MAX_DATA_LEN
  #define INTERFACE_MAX_PACK_LENG 112
#else
  #define INTERFACE_MAX_PACK_LENG USER_INTERFACE_MAX_DATA_LEN + INTERFACE_HEAD_SIZE + INTERFACE_CRC_SIZE
#endif
  
#define RX_BUFFER_LEN           INTERFACE_MAX_PACK_LENG  

#define TX_BUFFER_LEN           RX_BUFFER_LEN
  

#define INTERFACE_HEAD_IN_LENG

//  #define INTERFACE_CMD_BYTE      0
//  #define INTERFACE_ADDR_BYTE     0
//  #define INTERFACE_DATA_OFFSET   1

  #define CIRC_BUFF_CNT           2
  
  #define CIRC_RX_DEEP            5
  #define CIRC_TX_DEEP            5

#endif

/* Public typedef -----------------------------------------------------------*/
/* Public constants ---------------------------------------------------------*/
/* Public function prototypes -----------------------------------------------*/
/* Public variables ---------------------------------------------------------*/

/* Public function prototypes -----------------------------------------------*/

 
  /** @}*/ /* End of Interface_config  group  */
/** @}*/ /* End of Interface group            */


#ifdef __cplusplus
}
#endif

#endif


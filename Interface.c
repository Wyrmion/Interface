/**
 ****************************************************************************
 * @file     Interface.c
 * @author   Wyrm
 * @brief    This code is designed to work with various kinds of interfaces. It is a parent class
 * @version  V1.7.1
 * @date     04 Feb. 2026.
 *************************************************************************
 */
/*
   @verbatim
  ==============================================================================
                        ##### How to use this class #####
  ==============================================================================


*/


#include <string.h>
#include <stdlib.h>

#include "wheap.h"

#include "Interface.h"
#include "InterfacePrivate.h"
#include "InterfacePrivateWrapper.h"

#include "../Interface/CircBuff/CircBuff.h"
//#include "../Memory/MyHeap/my_heap.h"







/**
 * @addtogroup Interface
 * @{
 */

#define CAST_INTERFACE(cthis) ((InterfaceHandel_t*)cthis)

/* Private function prototypes -----------------------------------------------*/
/** @defgroup Interfafce_Private_Functions Interfafce Private Functions
  * @{
  */
  static bool   _this_CRCcheck(const InterfaceHandel_t* cthis,const uint8_t* pack,const uint32_t len);
  static void   _this_InsertCRC(const InterfaceHandel_t* cthis,uint8_t* src,size_t* len);
  //static size_t _this_TimeProtocol(uint8_t * dst,const uint8_t* src,size_t len){memcpy(dst,src,len); return len;};
  
  static void   _this_rx_irq(void* cthis,uint8_t* src,size_t len);
  static void   _this_tx_irq(void* this_ptr);
  static void   _this_err_irq(void* this_ptr){(void)this_ptr;};


  static size_t _this_rx_parser(InterfaceHandel_t* cthis,uint8_t* src,size_t len);
  static void   _this_CmdRxUploadProc(InterfaceHandel_t* cthis);
  static void   _this_CmdTxUploadProc(InterfaceHandel_t* cthis);
/** @}*/ /* Interfafce Private Functions */

/**
 * @brief InterfaceHandel Class
 * 
 */
struct InterfaceHandel
{   
  AlgoProto   AlgoritmPack;     /*!< Pointer to pack function*/    
  AlgoProto   AlgoritmUnpuck;   /*!< Pointer to unpack function*/

  sInterfaceRxFilter_t* cFilter;  /*!< Pointer to Riceve Filter Class*/
  

  sCRCInterface_t*      cCRC;         /*!< Pointer to crc @ref sCRCInterface_t class*/ 


  eInterfaceRxTxHandel_t irqmode;
  //sHeadInterface_t* headchk;                               

  uint8_t*  RxBuff;      /*!< Internal Rx Buffer*/
  size_t    Rx_len;      /*!< Internal Rx Buffer len*/
  size_t    RxBuffLen;

  uint8_t*  TxBuff;      /*!< Internal Tx Buffer*/
  size_t    Tx_len;     /*!< Internal Tx Buffer len*/
  size_t    TxBuffLen;
  
  uint8_t*  Pack;       /*!< Temp pack buffer*/

  size_t    LastLeng;   /*!< LastLeng buffer*/
  uint8_t*  CurData;
 
	CircBuff_t*     CircBuffRx; /*!<Pointer to Tx Circbuff obj*/
	CircBuff_t*     CircBuffTx; /*!<Pointer to Rx Circbuff obj*/
  
  HWInterface_t*  HwInter;              /*!< pointer to @ref HWInterface_t*/
  sInterfaceIrqCallback_t hwCB;         /*!< pointer to @ref sInterfaceIrqCallback_t callback from hardware to interface*/
  sInterfaceIrqParentCB_t parentCB;     /*!< pointer to @ref sInterfaceIrqParentCB_t callback from interface to parent*/
};


/**
* @brief InterfaceHandel Class
* @param pointer to abstract Harware interface class @ref HWInterface_t
* @return pointer to allocated memory
*/
InterfaceHandel_t*  Interface_ctor(HWInterface_t* HwInter,size_t IntBuffSize,size_t CircDeep)
{
  if(HwInter == NULL) return NULL;

  InterfaceHandel_t* cthis = NULL;
 
  if((cthis = heap_malloc_cast(InterfaceHandel_t)) == NULL)

    while(1); /* bug catch tag*/
  cthis->HwInter = HwInter;
  
  cthis->RxBuffLen = cthis->TxBuffLen = IntBuffSize;

  if((cthis->RxBuff = heap_malloc(IntBuffSize)) == NULL)
    while(1);
  if((cthis->TxBuff = heap_malloc(IntBuffSize)) == NULL)
    while(1);
  if((cthis->Pack = heap_malloc(IntBuffSize)) == NULL)
    while(1);
  
  if(CircDeep > 1)
  {
    cthis->CircBuffRx = CircBuff_ctor(IntBuffSize,CircDeep);
  
    if(cthis->CircBuffRx == NULL)
    {
      CircBuff_dctor(cthis->CircBuffRx);
      Interface_dtor(cthis);  
      return NULL;
    }

    cthis->CircBuffTx = CircBuff_ctor(IntBuffSize,CircDeep);

    if(cthis->CircBuffTx == NULL)
    {
      CircBuff_dctor(cthis->CircBuffTx);
      Interface_dtor(cthis);  
      return NULL;
    }
  }
  else 
  {
    cthis->CircBuffRx = cthis->CircBuffTx =NULL;
  }

  cthis->irqmode = kInterfaceRxTx_process;
  
  memset(cthis->RxBuff,0,IntBuffSize);
  cthis->Rx_len = 0;

  memset(cthis->TxBuff,0,IntBuffSize);
  cthis->Tx_len = 0;
  
  memset(&cthis->parentCB,0,sizeof(cthis->parentCB));
  memset(&cthis->hwCB,0,sizeof(cthis->hwCB));

  HwSetRxBuff(HwInter,cthis->RxBuff,IntBuffSize);
  HwSetTxBuff(HwInter,cthis->TxBuff,IntBuffSize);

  cthis->AlgoritmPack = cthis->AlgoritmUnpuck = NULL;

  cthis->cFilter = NULL;
  cthis->cCRC = NULL;

  return cthis;
}

/**
 * @brief Interface class destructor
 * 
 * @param cthis pointer to @ref InterfaceHandel_t
 */
void Interface_dtor(InterfaceHandel_t* cthis)
{
  CircBuff_dctor(cthis->CircBuffRx);
  CircBuff_dctor(cthis->CircBuffTx);
  
  heap_free(cthis);

  cthis = NULL;
}

/**
 * @brief Set Intereface irq mode
 * 
 * @param cthis pointer to @ref InterfaceHandel_t 
 * @param mode irq mode from @ref eInterfaceRxTxHandel_t
 */
void   Interface_SetMode(InterfaceHandel_t* cthis,const eInterfaceRxTxHandel_t mode)
{
  switch (mode)
  {
  case kInterfaceRxTx_process:  cthis->irqmode = mode;
                                break;
  case kInterfaceRxTx_irq:      cthis->irqmode = mode;
                                cthis->hwCB.parent = cthis;
                                cthis->hwCB.rx_cb  = _this_rx_irq;
                                cthis->hwCB.tx_cb  = _this_tx_irq;
                                cthis->hwCB.err_cb = _this_err_irq;
                                HwSetCB(cthis->HwInter,&cthis->hwCB);
                                break;
  case kInterfaceRxTx_Os:       cthis->irqmode = mode;
                                break;

  }
}

/**
 * @brief Get Interface max data length
 * 
 * @param cthis pointer to @ref InterfaceHandel
 * @return size_t maximum packet length
 */
size_t    Interface_GetMaxDatalng(InterfaceHandel_t* cthis) {return HwGetMaxDataLeng(cthis->HwInter);}

/**
 * @brief Sets pack and unpack algoritm 
 * 
 * @param cthis pointer to @ref InterfaceHandel
 * @param pack  pointer to pack @ref AlgoProto function
 * @param unpack pointer to unpack @ref AlgoProto functiono
 */
void Interface_InstallProtoAlgoritm(InterfaceHandel_t* cthis,AlgoProto pack, AlgoProto unpack)
{
  if(
      (cthis   == NULL)  
    ||(pack   == NULL)
    ||(unpack == NULL)
    )
    return;

  cthis->AlgoritmPack = pack;
  cthis->AlgoritmUnpuck = unpack;
}

/**
 * @brief Link CRC algoritm function to @ref InterfaceHandel
 * @param cthis pointer to @ref InterfaceHandel_t 
 * @param crcf pointer to CRC @ref AlgoCrc algoritm func
 * @param CrcHeadCalc true if need calculate crc for head, false for calculate only payload
 * @return true if ok
 * @return false if error
 */
bool  Interface_InstallCRCAlgoritm(InterfaceHandel_t* cthis,sCRCInterface_t* crc)
{
  if(crc == NULL)
    return false;
  
  cthis->cCRC = crc;

  return true;
}
/**
 * @brief installation rx filter class to interface 
 * Filter is used to discard messages before they hit the buffer
 * @param cthis pointer to @ref InterfaceHandel_t 
 * @param filter pointer to @ref sInterfaceRxFilter_t class
 * @return true   if filter install
 * @return false  error
 */
bool Interface_InstallFilter(InterfaceHandel_t* cthis, sInterfaceRxFilter_t *filter)
{
  if((cthis == NULL)||(filter==NULL))
    return false;
  
  cthis->cFilter = filter;

  return true;
}
/**
 * @brief set parent callback function for fast call in irq
 * 
 * @param cthis     pointer to @ref InterfaceHandel_t 
 * @param parentCB  pointer to Interface irq parent callback struct @ref sInterfaceIrqParentCB_t
 * @return true     if set successful
 * @return false    if error
 */
bool Interface_SetCB(InterfaceHandel_t* cthis, sInterfaceIrqParentCB_t* parentCB)
{
  if((cthis == NULL) || (parentCB == NULL))
    return false;
  if(   (parentCB->parent == NULL)
      ||(parentCB->RxCb == NULL)
      ||(parentCB->TxCb == NULL)
      ||(parentCB->ErrCb == NULL))
    return false;

  cthis->parentCB = *parentCB;

  return true;
}

/**
 * @brief Interface Rx Function for directly run in an interrupt 
 * @details if irqmode = @arg kInterfaceRxTx_irq
 * @param[in] this pointer to @ref InterfaceHandel_t 
 * @param[in] src  data
 * @param[in] len  data leng 
 */
static void _this_rx_irq(void* cthis,uint8_t* src,size_t len)
{
  if(cthis == NULL) 
    return;

  if((CAST_INTERFACE(cthis)->LastLeng = _this_rx_parser(cthis,src,len)) == 0)
    return; /* No valid data*/
    
  if(CAST_INTERFACE(cthis)->parentCB.RxCb != NULL)
    CAST_INTERFACE(cthis)->parentCB.RxCb(CAST_INTERFACE(cthis)->parentCB.parent,CAST_INTERFACE(cthis),CAST_INTERFACE(cthis)->CurData,CAST_INTERFACE(cthis)->LastLeng);
  else
  {
    if(CAST_INTERFACE(cthis)->CircBuffRx)
      CircBuff_push(CAST_INTERFACE(cthis)->CircBuffRx,CAST_INTERFACE(cthis)->CurData,CAST_INTERFACE(cthis)->LastLeng);
  }
}


/**
 * @brief Interface data parser
 * 
 * @param[in]   this pointer to @ref InterfaceHandel_t 
 * @param[in]   src data  
 * @param[in]   len data leng
 * @return      size_t parsed data leng
 */
static size_t _this_rx_parser(InterfaceHandel_t* cthis,uint8_t* src,size_t len)
{
  size_t pack_leng = 0;
  
  if(cthis->AlgoritmUnpuck)
  {
    if((pack_leng = cthis->AlgoritmUnpuck(cthis->Pack,src,len)) == 0)
      return 0;
    cthis->CurData = cthis->Pack;
  }
  else
  {
    pack_leng = len;
    cthis->CurData = src;
  }
  
  if(cthis->cFilter != NULL)
    if(!cthis->cFilter->func(cthis->cFilter->parent,cthis->CurData,len))
      return 0;    
    
  if(cthis->cCRC != NULL)
  {
    if(pack_leng<=CRC_GetSize(cthis->cCRC))
      return 0;
    if(!_this_CRCcheck(cthis,cthis->CurData,len))
      return 0;
    else 
      pack_leng-=CRC_GetSize(cthis->cCRC);
  }

  
  return pack_leng;
}




/**
 * @brief Check crc wrapper 
 * 
 * @param cthis pointer to @ref InterfaceHandel_t 
 * @param data pointer to data
 * @param len  sizeof of data
 * @return true   if crc ok
 * @return false  else
 */
bool  _this_CRCcheck(const InterfaceHandel_t* cthis,const uint8_t* pack,const uint32_t len)
{
  uint32_t crc_shift = len-CRC_GetSize(cthis->cCRC);
     
  uint16_t crc = CRC_GetCRC(cthis->cCRC,pack,crc_shift);
  
  if(crc != (*((uint16_t*)(pack+crc_shift)))) 
    return false;
  else 
    return true;
}



/**
 * @brief Send data via @ref InterfaceHandel_t
 * 
 * @param[in] cthis     pointer to @ref InterfaceHandel_t 
 * @param[in] payload   pointer to data 
 * @param[in] leng      data size to send
 * @return true 
 * @return false 
 */
bool Interface_SendData(InterfaceHandel_t* cthis,void *payload,size_t leng)
{ 
  if(cthis->cCRC != NULL) 
  {
    if(leng+CRC_GetSize(cthis->cCRC)>cthis->TxBuffLen)
      return false;
    _this_InsertCRC(cthis,payload,&leng);
  }
  uint8_t* cur_data = NULL;
  
  if(cthis->AlgoritmPack)
  {
    cthis->Tx_len = cthis->AlgoritmPack(cthis->TxBuff,(uint8_t*)payload,leng);
    cur_data = cthis->TxBuff;
  }
  else  
  {  
    cthis->Tx_len = leng;
    cur_data = payload;
  }


  if(cthis->Tx_len == 0) return false;
  
  bool state = false;

  if(HwSendData(cthis->HwInter,cur_data,cthis->Tx_len)) 
    return true;
  else 
  {
    if(!cthis->CircBuffTx)
      return false;

    if(cthis->irqmode == kInterfaceRxTx_irq)
    {  
      HwEnterCriticalTx(cthis->HwInter);
      
      state = CircBuff_push(cthis->CircBuffTx,cthis->TxBuff,cthis->Tx_len);
    
      HwExitCriticalTx(cthis->HwInter);
    }
    else
      state = CircBuff_push(cthis->CircBuffTx,cthis->TxBuff,cthis->Tx_len);
  }
  return state;
}

/**
 * @brief Interface tx Function for directly run in an interrupt 
 * 
 * @details if irqmode = @arg kInterfaceRxTx_irq. whis should be  TX Notempty irq
 * @param[in] this_ptr pointer to @ref InterfaceHandel_t  
 */
static void _this_tx_irq(void* this_ptr)
{
  InterfaceHandel_t* cthis = CAST_INTERFACE(this_ptr);
  
  if(CircBuff_pop(cthis->CircBuffTx,cthis->TxBuff,&cthis->Tx_len))
    HwSendData(cthis->HwInter,cthis->TxBuff,cthis->Tx_len);
}


/**
 * @brief Insert crc to data
 * 
 * @param cthis pointer to @ref InterfaceHandel_t
 * @param src  pointer to data
 * @param leng pointer to size of data (increment then)
 */
static void _this_InsertCRC(const InterfaceHandel_t* cthis,uint8_t* src,size_t* len)
{
  *((uint16_t*)(src+*len)) = CRC_GetCRC(cthis->cCRC,src,*len);
  *len += CRC_GetSize(cthis->cCRC);
}


/**
 * @brief Read Data from Interface buffer
 * 
 * @param cthis this pointer to @ref InterfaceHandel_t  
 * @param dst  pointer to output data
 * @return size_t size of output data
 */
size_t Interface_readData(InterfaceHandel_t* cthis,void* dst)
{
  if(cthis->CircBuffRx)
  {
    if(CircBuff_IsFree(cthis->CircBuffRx))
      return 0;
    
    size_t leng = 0;

    if(cthis->irqmode == kInterfaceRxTx_irq)
    {
      HwEnterCriticalRx(cthis->HwInter);

      CircBuff_pop(cthis->CircBuffRx,dst,&leng);

      HwExitCriticalRx(cthis->HwInter);
    }
    else
      CircBuff_pop(cthis->CircBuffRx,dst,&leng);

    return leng;
  }
  else 
  { 
    if(cthis->LastLeng)
    {
      __auto_type ret = cthis->LastLeng;
      cthis->LastLeng = 0;
      memcpy(dst,cthis->RxBuff,ret);      
      return ret;
    }
    else
      return 0;
    
  }
}
/**
 * @brief Check is Interface cmd buffer not empty
 * 
 * @param cthis    this pointer to @ref InterfaceHandel_t  
 * @return true   if cmd buffer not empty
 * @return false  else
 */
bool  Interface_isRxNe(InterfaceHandel_t* cthis)
{
  if(cthis->CircBuffRx)
    return !CircBuff_IsFree(cthis->CircBuffRx);
  else
    return cthis->LastLeng;
}

/**
 * @brief Connect to interface
 * 
 * @param cthis    pointer to @ref InterfaceHandel_t
 * @return true   if connect successful
 * @return false  else
 */
bool Interface_Connect(InterfaceHandel_t* cthis){return HwConnect(cthis->HwInter);}

/**
 * @brief Disconnect from interface
 * 
 * @param cthis    pointer to @ref InterfaceHandel_t
 * @return true   if disonnect successful
 * @return false  else
 */
bool Interface_Disconnect(InterfaceHandel_t* cthis)
{
  if(HwIsFree(cthis->HwInter))
    return HwDisconnect(cthis->HwInter);
  else
    return false;
}

/**
 * @brief Interface main process
 * @note  it's not blocking function and must run in an infinite loop, or rtos task
 *        It processes the @ref _this_CmdTxUploadProc()and @ref _this_CmdRxUploadProc()
 * @param cthis pointer to @ref InterfaceHandel_t 
 */
void Interface_process(InterfaceHandel_t* cthis)
{
  if(cthis->irqmode == kInterfaceRxTx_irq)
    return; /* should not be use in irq mode*/
  if(cthis->irqmode == kInterfaceRxTx_Os)
  {
    HwProcess(cthis);
  }
  _this_CmdRxUploadProc(cthis);
  _this_CmdTxUploadProc(cthis);
  
}

/**
 * @brief Rx Command upload none blocking process 
 * @note  Check Rx flag, and append CircBuff is not empty
 * @param cthis pointer to @ref InterfaceHandel_t 
 */
static void _this_CmdRxUploadProc(InterfaceHandel_t* cthis)
{
  if(HwReadRxBuff(cthis->HwInter,cthis->RxBuff,&cthis->Rx_len,cthis->RxBuffLen))
  {
    if(cthis->Rx_len > cthis->RxBuffLen)
      while(1);

    if((cthis->LastLeng = _this_rx_parser(cthis,cthis->RxBuff,cthis->Rx_len)) == 0)
      return; /* No valid data*/       
    
    if(cthis->CircBuffRx)
      CircBuff_push(cthis->CircBuffRx,cthis->Pack,cthis->LastLeng);
  }
}

/**
 * @brief Tx Command upload none blocking process 
 * @note  Check Rx flag, and append CircBuff is not empty
 * @param cthis pointer to @ref InterfaceHandel_t 
 */
static void _this_CmdTxUploadProc(InterfaceHandel_t* cthis)
{
  if(!cthis->CircBuffTx)
    return;
  
  if(CircBuff_IsFree(cthis->CircBuffTx)) 
    return;
  
  if(!HwIsFree(cthis->HwInter)) 
    return;
  
  if(CircBuff_pop(cthis->CircBuffTx,cthis->TxBuff,&cthis->Tx_len))
      HwSendData(cthis->HwInter,cthis->TxBuff,cthis->Tx_len);
}
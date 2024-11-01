/**
 ****************************************************************************
 * @file     Interface.c
 * @author   Kukushkin A.V.
 * @brief    This code is designed to work with various kinds of interfaces. It is a parent class
 * @version  V1.6.1
 * @date     09 Oct. 2024.
 *************************************************************************
 */
 
#define INTERFACE_PROTOCOL_TIME_SEPARATION
#define INTERFACE_PROTOCOL_BYTESTAFF

#include "Interface.h"
#include "InterfacePrivate.h"
#include "CmdCircBuff/CmdCircBuff.h"
#ifdef INTERFACE_PROTOCOL_BYTESTAFF
  #include "ByteStaff/ByteStaff.h"
#endif
#include <string.h>
#include <stdlib.h>
#include "Hex2ASCii/Hex2ASCII.h"



#ifdef ADDR_IN_PACK
//  #define ADDR_MULTICAST_SUPPORT
  #ifdef ADDR_MULTICAST_SUPPORT
    #define MULTICAST_ADDR 0
  #endif
#endif

#define RX_BUFFER_LEN 255
#define TX_BUFFER_LEN 255

#define PACK_BUFFER_LEN 256

#define SMO_CIRC_RX_DEEP 5
#define SMO_CIRC_TX_DEEP 5

#define INTERFACE_CMD_BYTE      0
#define INTERFACE_ADDR_BYTE     0

static void SetRxBuff(void* this_ptr,uint8_t* data,size_t max_len);
static void SetTxBuff(void* this_ptr,uint8_t* data,size_t max_len);
static void HwProcess(void* this_ptr);
static bool SendData(void* this_ptr,uint8_t* data,uint8_t len);
static bool IsHwFree(void* this_ptr);
static bool Connect(void* this_ptr);
static bool Interface_ChkCrc(InterfaceHandel_t *phdev,uint8_t* pack,uint32_t len);
static void Interface_InsertCrc(InterfaceHandel_t *phdev,InterfaceCmdData_t* message);

static uint32_t Interface_TimeProtocol(uint8_t * dst,const uint8_t* src,const uint32_t len){memcpy(dst,src,len); return len;};

//static void Interface_PackToBytestaffTx(InterfaceHandel_t* phdev,InterfaceCmdData_t *payload);
//static bool Interface_PackToASCii(InterfaceHandel_t* phdev,InterfaceCmdData_t *payload);

//typedef Algoritm (uint32_t)(void*)(uint8_t*,uint8_t*,uint32_t);

struct InterfaceHandel
{
  eInterfaceProtocol_t ProtAlgoritm;
  
  uint32_t  (*AlgoritmRepack)(uint8_t* dst,const uint8_t* src,const uint32_t len);
  uint32_t  (*AlgoritmPack)(uint8_t* dst,const uint8_t* src,const uint32_t len);

  uint16_t  (*AlgoritmCrc)(const uint8_t* data,uint32_t len);

  uint8_t RxBuff[RX_BUFFER_LEN];
  size_t 	Rx_len;
  uint8_t	TxBuff[TX_BUFFER_LEN];
  size_t 	Tx_len;
  
  

  #ifdef CIRC_BUFF_NO_MALLOC_CNT
    InterfaceCmdData_t IntrfcCmdDataRx[SMO_CIRC_RX_DEEP];
    InterfaceCmdData_t IntrfcCmdDataTx[SMO_CIRC_TX_DEEP];
  #endif
 
  uint8_t           Pack[PACK_BUFFER_LEN];

	CircBuf_Handle_t *CircBuffRx;
	CircBuf_Handle_t *CircBuffTx;
	 
  bool    CrcInPack;
  bool    CrcChck;
  bool    CrcHeadCalc;
  
  bool    AddrInPack;

  
  
  uint32_t Addr;
  
  HWInterface_t* HwInter; 
};

#ifdef NO_MY_MALLOC
  #define NUMBER_OF_OBJ 3
  static size_t hdev_indx = 0;
  static InterfaceHandel_t hdev[NUMBER_OF_OBJ];
#endif

InterfaceHandel_t* Interface_ctor(HWInterface_t* HwInter)
{
  if(HwInter == NULL) return NULL;

  InterfaceHandel_t * phdev = NULL;
  #ifdef NO_MY_MALLOC
  if(hdev_indx < NUMBER_OF_OBJ)
  {
    #if (NUMBER_OF_OBJ == 1)
      phdev = &hdev;
    #else
		  phdev = &hdev[hdev_indx];
		  hdev_indx++;
    #endif
  }
  else return NULL;
  #else 
    phdev = malloc(sizeof(InterfaceHandel_t));
  #endif

  if(phdev==NULL) return NULL;
  phdev->HwInter = HwInter;
    
  #ifdef CIRC_BUFF_NO_MALLOC_CNT
    phdev->CircBuffRx = CircBuf_ctor(phdev->IntrfcCmdDataRx,SMO_CIRC_RX_DEEP);
  #else
    phdev->CircBuffRx = CircBuf_ctor(SMO_CIRC_RX_DEEP);
  #endif

  if(!phdev->CircBuffRx)
  {
    Interface_dtor(phdev);
    return phdev;
  }

  #ifdef CIRC_BUFF_NO_MALLOC_CNT
    phdev->CircBuffTx = CircBuf_ctor(phdev->IntrfcCmdDataTx,SMO_CIRC_RX_DEEP);
  #else
    phdev->CircBuffTx = CircBuf_ctor(SMO_CIRC_RX_DEEP);
  #endif

  if(!phdev->CircBuffTx)
  {
    Interface_dtor(phdev);
    CircBuf_dctor(phdev->CircBuffRx);
      
    #ifndef NO_MY_MALLOC
      free(phdev);
    #endif

    return NULL;
  }

  phdev->ProtAlgoritm = kInterfeceProto_ClassicBStaff;
  #ifdef INTERFACE_PROTOCOL_BYTESTAFF
    phdev->AlgoritmRepack = StaffGetPACKfromBuff;
  #endif
  memset(phdev->RxBuff,0,sizeof(phdev->RxBuff));
  phdev->Rx_len = 0;

  memset(phdev->TxBuff,0,sizeof(phdev->RxBuff));
  phdev->Tx_len = 0;

  SetRxBuff(HwInter,phdev->RxBuff,sizeof(phdev->RxBuff));
  SetTxBuff(HwInter,phdev->TxBuff,sizeof(phdev->TxBuff));

  phdev->CrcInPack    = true;
  phdev->CrcChck      = false;
  phdev->CrcHeadCalc  = true;
  phdev->AddrInPack   = true;
  
  phdev->Addr = 0;

  return phdev;
}

void Interface_dtor(InterfaceHandel_t *phdev)
{
  #ifndef NO_MY_MALLOC
    free(phdev);
  #else
    if(hdev_indx != NUMBER_OF_OBJ) hdev_indx--;
    else while(1); //bug_catch
  #endif
  phdev = NULL;
}
void                Interface_SetAddr(InterfaceHandel_t * phdev,uint32_t addr) {phdev->Addr = addr;};
uint32_t            Interface_GetAddr(InterfaceHandel_t * phdev){return phdev->Addr;};

void Interface_SetProtoAlgoritm(InterfaceHandel_t *phdev, eInterfaceProtocol_t algoritm)
{
  switch (algoritm)
  {
    #ifdef INTERFACE_PROTOCOL_BYTESTAFF
    case kInterfeceProto_ClassicBStaff:     phdev->AlgoritmRepack = StaffGetPACKfromBuff;
                                            phdev->AlgoritmPack = StaffSetBufftoPACK;
                                            break;
    #endif // INTERFACE_PROTOCOL_BYTESTAFF

    #ifdef INTERFACE_PROTOCOL_TIME_SEPARATION
    case kInterfaceProto_Time_Separete_MB:  phdev->AlgoritmRepack = Interface_TimeProtocol;
                                            phdev->AlgoritmPack = Interface_TimeProtocol;    
                                            break;                                  
    #endif //INTERFACE_PROTOCOL_TIME_SEPARATION
    
    #ifdef INTERFACE_PROTOCOL_ASCII
    case kInterfeceProto_ASCii:         phdev->AlgoritmRepack = hex2ascii_getUintArr;
                                        break;
    #endif
    default: return;
  }

  phdev->ProtAlgoritm = algoritm;
}

static bool  ReadRxBuff(void* this_ptr,uint8_t* data,size_t* len,size_t max_len){
  HWInterface_t* HwInter = (HWInterface_t*)this_ptr;
  return(HwInter->vtable->ReadRxBuff(HwInter,data,len,max_len));
}

static void SetRxBuff(void* this_ptr,uint8_t* data,size_t max_len)
{
  HWInterface_t* HwInter = (HWInterface_t*)this_ptr;
  return(HwInter->vtable->SetRxBuff(HwInter,data,max_len));
}
 
static void SetTxBuff(void* this_ptr,uint8_t* data,size_t max_len)
{	
 HWInterface_t* HwInter = (HWInterface_t*)this_ptr;
 return(HwInter->vtable->SetTxBuff(HwInter,data,max_len));
}
 
static void HwProcess(void* this_ptr){
  HWInterface_t* HwInter = (HWInterface_t*)this_ptr;
  HwInter->vtable->Process(HwInter);
}

static bool SendData(void* this_ptr,uint8_t* data,uint8_t len){
  HWInterface_t* HwInter = (HWInterface_t*)this_ptr;
  return HwInter->vtable->SendData(this_ptr,data,len);
}

static bool IsHwFree(void* this_ptr){
  HWInterface_t* HwInter = (HWInterface_t*)this_ptr;
  return HwInter->vtable->IsFree(this_ptr);
}

static bool Connect(void* this_ptr){
  HWInterface_t* HwInter = (HWInterface_t*)this_ptr;
  return HwInter->vtable->Connect(this_ptr);
}

static void CmdRxUploadProc(InterfaceHandel_t* phdev)
{
  if(ReadRxBuff(phdev->HwInter,phdev->RxBuff,&phdev->Rx_len,sizeof(phdev->RxBuff)))
  {
    uint32_t pack_leng = 0;
    if((pack_leng = phdev->AlgoritmRepack(phdev->Pack,phdev->RxBuff,phdev->Rx_len))>0)
    {
      if(pack_leng < (offsetof(InterfaceCmdData_t,Data)+ sizeof(interface_crc_t)))
        return;
      
      #ifdef ADDR_IN_PACK
      if(((InterfaceCmdData_t*)phdev->Pack)->addr != phdev->Addr)      
      {    
        #ifdef ADDR_MULTICAST_SUPPORT
        if(((InterfaceCmdData_t*)phdev->Pack)->addr != MULTICAST_ADDR)
        #endif
           return;
      }     
      #endif
      
      if(phdev->CrcChck) 
      {
        if(phdev->CrcHeadCalc)
        {
          if(!Interface_ChkCrc(phdev,phdev->Pack,pack_leng))
            return;
        }
        else
        {  
          if(!Interface_ChkCrc(phdev,phdev->Pack+offsetof(InterfaceCmdData_t,Data),pack_leng-offsetof(InterfaceCmdData_t,Data)))   
            return;
        }
      }                   
      CircBuf_push_buff(phdev->CircBuffRx,phdev->Pack,pack_leng);
    }
  }
}

bool  Interface_InstallCRCAlgoritm(InterfaceHandel_t* phdev,uint16_t(*CRCfunc)(const uint8_t*/*data*/,uint32_t /*len*/),bool CrcHeadCalc)
{
  if(CRCfunc == NULL)
    return false;
  phdev->AlgoritmCrc = CRCfunc;
  phdev->CrcHeadCalc = CrcHeadCalc;
  phdev->CrcChck    = true;
  phdev->CrcInPack  = true;
  return true;
}


bool  Interface_ChkCrc(InterfaceHandel_t* phdev,uint8_t* pack,uint32_t len)
{
  uint32_t crc_shift = len-sizeof(interface_crc_t);
  
  
  
  uint16_t crc = phdev->AlgoritmCrc(pack,crc_shift);
  
  if(crc != (*((uint16_t*)(pack+crc_shift)))) return false;
  else return true;
}



static void CmdTxUploadProc(InterfaceHandel_t* phdev)
{
  if(CircBuf_IsFree(phdev->CircBuffTx)) return;
  if(!IsHwFree(phdev->HwInter)) return;

  InterfaceCmdData_t message;

  CircBuf_pop(phdev->CircBuffTx,&message);
  
  SendData(phdev->HwInter,message.Data,message.len);
}

bool Interface_sendCMD(InterfaceHandel_t* phdev,InterfaceCmdData_t *payload)
{ 
  #ifdef ADDR_IN_PACK
    payload->addr = phdev->Addr;
  #endif

  if(phdev->CrcInPack) 
  {
    uint16_t total_leng = payload->len+sizeof(interface_crc_t);
    if(total_leng>INTERFACE_MAX_PACK_LENG) while(1); //bug_catch
    if(phdev->CrcInPack) Interface_InsertCrc(phdev,payload);
  }

  phdev->Tx_len = phdev->AlgoritmPack(phdev->TxBuff,(uint8_t*)payload,payload->len+offsetof(InterfaceCmdData_t,Data));

  if(phdev->Tx_len == 0) return false;

  if(SendData(phdev->HwInter,phdev->TxBuff,phdev->Tx_len)) 
    return true;
  else 
  {
    CircBuf_push_DataCmdLen(phdev->CircBuffTx,phdev->TxBuff,payload->cmd,phdev->Tx_len);
    return true;
  }

  return false;
}

bool Interface_Connect(InterfaceHandel_t *phdev){return Connect(phdev->HwInter);}

static void Interface_InsertCrc(InterfaceHandel_t *phdev,InterfaceCmdData_t* message)
{
  if(phdev->CrcChck)
  {
    if(phdev->CrcHeadCalc)
      *((uint16_t*)(message->Data+(message->len))) = phdev->AlgoritmCrc((uint8_t*)message,message->len+offsetof(InterfaceCmdData_t,Data));
    else  
      *((uint16_t*)(message->Data+(message->len))) = phdev->AlgoritmCrc(message->Data,message->len);
  } 
  else
    *((uint16_t*)(message->Data+(message->len))) = 0xAAAA;

  message->len+=sizeof(interface_crc_t);
}

void Interface_process(InterfaceHandel_t* phdev)
{
  CmdTxUploadProc(phdev);
  CmdRxUploadProc(phdev);
}

bool Interface_readCMD(InterfaceHandel_t * phdev,InterfaceCmdData_t *payload)
{
  return CircBuf_pop(phdev->CircBuffRx,payload);
}

bool  Interface_isCMDdRDY(InterfaceHandel_t * phdev)
{
  return !CircBuf_IsFree(phdev->CircBuffRx);
}
/******************************************************************

  Name:         fe1730.c
  Created by:   Maria Martinez

  Contents:    test code of standarized frontend dealing with
  common VME module 

  $Id$
********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <math.h>
#include "midas.h"
#include "CAENDigitizer.h"
#include "CAENComm.h"
#include "v1725Raw.h"

#include "mfe.h"

typedef unsigned char BYTE;
typedef unsigned short int UINT16;
typedef short int INT16;
typedef unsigned int UINT32;
typedef int INT32;
typedef long int INT64;

#include "experim.h"
// Max event size  supported (in bytes).
// up to 1us per event ->
// 1000 us * 500 S/us * 2 B/S * 16 ch = 16MB
#define V1730_MAX_EVENT_SIZE 16000000

// 8000 us * 500 S/us * 2 B/S * 16 ch = 128MB
//#define V1730_MAX_EVENT_SIZE 128000000

#define MAXEV_SINGLEREADOUT 20

#define MAX_CH 16
#define CLOCK2NS 8
// VME base address 
DWORD V1730_BASE = 0x32100000;
DWORD VMEBUS_BOARDNO = 0;
DWORD LINK = 1;
WORD V1730EVENTID = 1;
WORD V1730TRIGGERMASK = 0;
int VMEhandle=-1;


int verbose = 1;

//-- Globals -------------------------------------------------------
CAEN_DGTZ_BoardInfo_t BoardInfo;
uint32_t  AllocatedSize;
uint32_t BufferSize;
CAEN_DGTZ_EventInfo_t       EventInfo;
char *buffer = NULL;
INT32 myBufferSize=0;
char *EventPtr = NULL;
CAEN_DGTZ_UINT16_EVENT_t    *Event16 = NULL;
int enabledChannels;


// The frontend name (client name) as seen by other MIDAS clients   
const char *frontend_name = "fe1730";
// The frontend file name, don't change it 
const char *frontend_file_name = __FILE__;

// frontend_loop is called periodically if this variable is TRUE    
BOOL frontend_call_loop = TRUE; //FALSE;

// a frontend status page is displayed with this frequency in ms 
INT display_period = 000;

// maximum event size produced by this frontend 
INT max_event_size = V1730_MAX_EVENT_SIZE;

// buffer size to hold up to MAXEV_SINGLEREADOUT  events 
INT event_buffer_size = MAXEV_SINGLEREADOUT * max_event_size + 10000;

// maximum event size for fragmented events (EQ_FRAGMENTED) 
INT max_event_size_frag = 5 * 1024 * 1024;


// Hardware 
int  inRun = 0, missed=0;
int done=0, stop_req=0;
DWORD evlimit;

// Globals 
extern HNDLE hDB;  // FROM mfe.h
HNDLE hSet;
V1730_DATA00_SETTINGS v1730_settings;
uint16_t channel_mask; // Mask of enabled channels

//-- Function declarations -----------------------------------------

int frontend_init();
INT frontend_exit();
INT begin_of_run(INT run_number, char *error);
INT end_of_run(INT run_number, char *error);
INT pause_run(INT run_number, char *error);
INT resume_run(INT run_number, char *error);
INT frontend_loop();
extern void interrupt_routine(void);

INT read_trigger_event(char *pevent, INT off);

void register_cnaf_callback(int debug);
int set_relative_Threshold();
int configure_trigger();


//-- Equipment list ------------------------------------------------

#undef USE_INT

#define N_PTS 10000

/*
BANK_LIST v1730_bank_list[] = {

   // online banks

   {"WF00", TID_WORD, N_PTS, NULL}
   ,

   {""}
   ,
};
*/

// LA DEFINICION BUENA SE HACE AQUI.
// OJO, SI NO LO PILLA BIEN A LA PRIMERA BORRARLO DEL ODB Y VOLVER A CORRER FRONTEND
EQUIPMENT equipment[] = {

   {"V1730_Data00",               // equipment name 
    {V1730EVENTID, V1730TRIGGERMASK,                   // event ID, trigger mask 
     "SYSTEM",               // event buffer 
#ifdef USE_INT
     EQ_INTERRUPT,           // equipment type 
#else
     EQ_POLLED, //PERIODIC,              // equipment type 
#endif
     LAM_SOURCE(0, 0x0),     // event source crate 0, all stations 
     "MIDAS",                // format 
     TRUE,                   // enabled 
     RO_RUNNING,          // read only when running 
     500,                    // poll for 500ms 
     0,                      // stop run after this event limit 
     0,                      // number of sub events 
     0,                      // don't log history 
     "", "", "",},
    read_trigger_event,      // readout routine 
    NULL, NULL,
    NULL,                   // aqui iria la bank list, voy a ver si vale para algo o no
    }
   ,

   {""}
};


                                             

/**********************************************************************
              Callback routines for system transitions

  These routines are called whenever a system transition like start/
  stop of a run occurs. The routines are called on the following
  occations:

  frontend_init:  When the frontend program is started. This routine
                  should initialize the hardware.

  frontend_exit:  When the frontend program is shut down. Can be used
                  to releas any locked resources like memory, commu-
                  nications ports etc.

  begin_of_run:   When a new run is started. Clear scalers, open
                  rungates, etc.

  end_of_run:     Called on a request to stop a run. Can send
                  end-of-run event and close run gates.

  pause_run:      When a run is paused. Should disable trigger events.

  resume_run:     When a run is resumed. Should enable trigger events.
*******************************************************************/


//-- Sequencer callback info  --------------------------------------
void seq_callback(INT hDB, INT hseq, void *info)
{
  printf("odb ... trigger settings touched\n");

}

//-----------------------------------------------------------------
//-----------------------------------------------------------------
//-----------------------------------------------------------------
//-- Frontend Init -------------------------------------------------
int frontend_init()
{
  CAEN_DGTZ_ErrorCode ret;
std::cout << " FRONTEND INIT!!!! " << std::endl;
#ifdef USE_INT
std::cout << " interruptioon " << std::endl;
#else
std::cout << " polled " << std::endl;
#endif
  int size, status;
  char set_str[80];

  // Book Setting space 
  V1730_DATA00_SETTINGS_STR(v1730Settings_str);

  // Map /equipment/V1730_Data00/Settings for the sequencer 
  sprintf(set_str, "/Equipment/V1730_Data00/Settings");
  status = db_create_record(hDB, 0, set_str, strcomb(v1730Settings_str));
  status = db_find_key (hDB, 0, set_str, &hSet);
  if (status != DB_SUCCESS)
    cm_msg(MINFO,"FE","Key %s not found", set_str);
  
  // Enable hot-link on settings/ of the equipment 
  size = sizeof(V1730_DATA00_SETTINGS);
  if ((status = db_open_record(hDB, hSet, &v1730_settings, size, MODE_READ
                               , seq_callback, NULL)) != DB_SUCCESS) return status;

  ////////////////////////// v1730_settings contains the configuration

  ////////////////////////
  // Open VME interface, init link board_number
  ret = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_OpticalLink, LINK, VMEBUS_BOARDNO,V1730_BASE, &VMEhandle);
  if (ret != CAEN_DGTZ_Success) 
  { 
    std::cout << " Error opening Digitizer. Digitizer error code: " << ret << std::endl;
    frontend_exit();
    exit(1);
  }

  ///////////////////// reset
  ret = CAEN_DGTZ_Reset(VMEhandle);            
  if (ret != CAEN_DGTZ_Success) 
  { 
    std::cout << " Error Resetting Digitizer. Digitizer error code: " << ret << std::endl;
    frontend_exit();
    exit(1);
  }

  ret = CAEN_DGTZ_GetInfo(VMEhandle, &BoardInfo);
  if (ret != CAEN_DGTZ_Success) 
  { 
    std::cout << " Error getting board info. Digitizer error code: " << ret << std::endl;
    frontend_exit();
    exit(1);
  }
  printf("\nConnected to CAEN Digitizer Model %s, recognized as board %d\n", BoardInfo.ModelName, VMEBUS_BOARDNO);
  printf("\tROC FPGA Release is %s\n", BoardInfo.ROC_FirmwareRel);
  printf("\tAMC FPGA Release is %s\n", BoardInfo.AMC_FirmwareRel);



  // CALIBRATE PEDESTASLS
  
  return SUCCESS;
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------
//-----------------------------------------------------------------
//-- Frontend Exit -------------------------------------------------

INT frontend_exit()
{
  CAEN_DGTZ_ErrorCode ret;
  ret = CAEN_DGTZ_CloseDigitizer(VMEhandle);
  if (ret != CAEN_DGTZ_Success) 
  { 
    std::cout << " Error closing digitizer. Digitizer error code: " << ret << std::endl;
  }
  return SUCCESS;
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------
//-----------------------------------------------------------------
//-- Begin of Run --------------------------------------------------

INT begin_of_run(INT run_number, char *error)
{
  CAEN_DGTZ_ErrorCode ret;

  // read V1730 settings 
  int status;
  int size = sizeof(V1730_DATA00_SETTINGS);
  if ((status = db_get_record (hDB, hSet, &v1730_settings, &size, 0)) != DB_SUCCESS) return status;

  // reset board
  ret = CAEN_DGTZ_Reset(VMEhandle);            
  if (ret != CAEN_DGTZ_Success) 
  { 
    std::cout << " Error Resetting Digitizer. Digitizer error code: " << ret << std::endl;
    frontend_exit();
    exit(1);
  }

  ///////////////////// CONFIGURATION
  DWORD testVal;


  ///////////////// ENABLE CHANNELS
  channel_mask=0;
  enabledChannels=0;
  for(size_t i=0;i<MAX_CH;++i)
  {
    if (v1730_settings.ch_enable[i]) 
    {
      channel_mask|=(1<<i);
      if(verbose) printf(" Channel %d enabled \n", (int)i);
      enabledChannels++;
    }
  }
  ret = CAEN_DGTZ_SetChannelEnableMask(VMEhandle,channel_mask);                   // Enable channel 0 
  if (verbose) std::cout <<  " channel mask " << std::hex << (int)channel_mask << std::dec << std::endl;

  ///////////////// RECORD LENGTH
  testVal = v1730_settings.recordlength; 
  ret = CAEN_DGTZ_SetRecordLength(VMEhandle,v1730_settings.recordlength);     // Set the lenght of each waveform (in samples) 
  if (ret != CAEN_DGTZ_Success) 
    std::cout << " Error Cannot set record length to " << v1730_settings.recordlength << " Digitizer error code: " << ret << std::endl;
  ret = CAEN_DGTZ_GetRecordLength(VMEhandle, &v1730_settings.recordlength);
  if (ret != CAEN_DGTZ_Success) 
    std::cout << " Error Cannot set record length to " << v1730_settings.recordlength << " Digitizer error code: " << ret << std::endl;
  if (testVal!=v1730_settings.recordlength) std::cout << " WARNING : RECORD LENGTH SET TO : " << v1730_settings.recordlength;



  ///////////////// POSTTRIGGER. In percent of the whole acquisition window
  testVal = v1730_settings.posttrigger; 
  ret = CAEN_DGTZ_SetPostTriggerSize(VMEhandle,v1730_settings.posttrigger);  // Set the posttrigger for each waveform (in samples) 
  if (ret != CAEN_DGTZ_Success) 
    std::cout << " Error Cannot set posttrigger to " << v1730_settings.posttrigger << " Digitizer error code: " << ret << std::endl;
  ret = CAEN_DGTZ_GetPostTriggerSize(VMEhandle, &v1730_settings.posttrigger);
  if (ret != CAEN_DGTZ_Success) 
    std::cout << " Error Cannot set posttrigger to " << v1730_settings.posttrigger << " Digitizer error code: " << ret << std::endl;
  if (testVal!=v1730_settings.posttrigger) std::cout << " WARNING : POSTRIGGER  SET TO : " << v1730_settings.posttrigger;

  ////////////// NIM LEVELS
  ret = CAEN_DGTZ_SetIOLevel(VMEhandle, CAEN_DGTZ_IOLevel_NIM);
  if (ret != CAEN_DGTZ_Success) 
    std::cout << " Error Cannot set levels to NIM.  Digitizer error code: " << ret << std::endl;

  ret = CAEN_DGTZ_SetMaxNumEventsBLT(VMEhandle,MAXEV_SINGLEREADOUT);       // Set the max number of events to transfer in a sigle readout 

  //////////////// CHANNEL DYNAMIC RANGE OFFSET AND TRIGGER THRESHOLD
  for(size_t i=0;i<BoardInfo.Channels;++i)
  {
    if (v1730_settings.ch_enable[i])
    {

       bool dyn = 0; // default +- 2V
       if (v1730_settings.ch_dynamicrange[i]<2) dyn=1;
       ret = CAEN_DGTZ_WriteRegister(VMEhandle, V1725_DYNAMIC_RANGE  + (i<<8), dyn);
       std::cout << " channel " << i << " dynamic range: " << (dyn ? "+-0.5 V" : " +-2 V") << std::endl;
       
       // DC Offset goes from 1 to 65535
       uint32_t  dcoffset = v1730_settings.ch_bslpercent[i] * 655.35;
       ret = CAEN_DGTZ_SetChannelDCOffset(VMEhandle, i, dcoffset);
       ret = CAEN_DGTZ_GetChannelDCOffset(VMEhandle, i, &dcoffset);
       //ret = CAEN_DGTZ_SetChannelTriggerThreshold(VMEhandle, i, v1730_settings.ch_threshold[i]);
       if (!strcmp(&v1730_settings.pulsePolarity,"+")) 
         ret = CAEN_DGTZ_SetTriggerPolarity(VMEhandle, i, CAEN_DGTZ_TriggerOnRisingEdge); 
       else ret = CAEN_DGTZ_SetTriggerPolarity(VMEhandle, i, CAEN_DGTZ_TriggerOnFallingEdge);
std::cout << " offset channel " << i << " set to  " << dcoffset << std::endl;
    }
  }

  //////////////// CONFIGURE TRIGGER LOGIC
  ret = (CAEN_DGTZ_ErrorCode)configure_trigger();
  if (ret != CAEN_DGTZ_Success) std::cout << " Error configuring trigger logics. Digitizer error code: " << ret << std::endl; 

  //ret = CAEN_DGTZ_SetSWTriggerMode(VMEhandle,CAEN_DGTZ_TRGMODE_ACQ_ONLY);         // Set the behaviour when a SW tirgger arrives 

 // set acquisition mode: starts by software. Other options are
 // CAEN_DGTZ_SW_CONTROLLED             = 0L,
 // CAEN_DGTZ_S_IN_CONTROLLED           = 1L,
 // CAEN_DGTZ_FIRST_TRG_CONTROLLED      = 2L,
 // CAEN_DGTZ_LVDS_CONTROLLED           = 3L,
 ret = CAEN_DGTZ_SetAcquisitionMode(VMEhandle,CAEN_DGTZ_SW_CONTROLLED);   
 if (ret != CAEN_DGTZ_Success) std::cout << " Error Cannot set acquisition sw controlled. Digitizer error code: " << ret << std::endl; 


  // TODO por ahora configurar tambien trigger externo. luego ya veremos
  // las opciones son CAEN_DGTZ_TRGMODE_DISABLED; CAEN_DGTZ_TRGMODE_ACQ_ONLY; CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT;
  ret = CAEN_DGTZ_SetExtTriggerInputMode(VMEhandle, (v1730_settings.externaltrigger ? CAEN_DGTZ_TRGMODE_ACQ_ONLY: CAEN_DGTZ_TRGMODE_DISABLED));
  if (ret != CAEN_DGTZ_Success) std::cout << " Error Cannot set external trigger. Digitizer error code: " << ret << std::endl; 


  // ACTIVATE ALSO SOFTWARE TRIGGER FOR BSL COMPUTING
  ret = CAEN_DGTZ_SetSWTriggerMode(VMEhandle, CAEN_DGTZ_TRGMODE_ACQ_ONLY);
  if (ret != CAEN_DGTZ_Success) std::cout << " Error Cannot set software trigger. Digitizer error code: " << ret << std::endl; 

  //check for possible failures after programming the digitizer
  // MARIA NOT BY NOW
/*
  ret = CheckBoardFailureStatus(VMEhandle, BoardInfo);
  if (ret != CAEN_DGTZ_Success) 
  {  
    std::cout << " Failure after configuration!. Digitizer error code: " << ret << std::endl; 
    frontend_exit();
    exit(1);
  }
*/

  if (buffer){delete [] buffer;buffer=0;} 

  myBufferSize = (16+enabledChannels*v1730_settings.recordlength*2)*(MAXEV_SINGLEREADOUT+1);
  buffer = new char [myBufferSize]; 
  // Allocate memory for event data and redout buffer
  ret = CAEN_DGTZ_AllocateEvent(VMEhandle, (void**)&Event16);
  if (ret != CAEN_DGTZ_Success) 
  {  
    std::cout << " Failure allocating buffer!. Digitizer error code: " << ret << std::endl; 
    frontend_exit();
    exit(1);
  }
  

  //ret = CAEN_DGTZ_MallocReadoutBuffer(VMEhandle, &buffer,&AllocatedSize); /* WARNING: This malloc must be done after the digitizer programming */
  std::cout <<  " Board configuration done " << std::endl;
  int retVal = set_relative_Threshold();
  if (retVal != 0) 
  {  
    std::cout << " Failure in setting relative threshold"  << std::endl; 
    frontend_exit();
    exit(1);
  }
  std::cout <<  " relative threshold done " << std::endl;


  if (ret != CAEN_DGTZ_Success) 
  {  
    std::cout << " Failure allocating buffer!. Digitizer error code: " << ret << std::endl; 
    frontend_exit();
    exit(1);
  }

  // ENABLE IRQ???

  printf("End of begin_of_run\n");

  // MARIA TODO
  // READ 0x800C register -> number of buffers
  std::cout << " active channels :  " << enabledChannels <<  " N samples: " <<  v1730_settings.recordlength << std::endl;
  uint32_t lstatus;
  ret = CAEN_DGTZ_ReadRegister(VMEhandle, V1725_BUFFER_ORGANIZATION, &lstatus);
  std::cout << " buffer organization . Number of buffers: " << pow(2,lstatus)  << std::endl;

  // MARIA 150222
  uint32_t boardCfg;
  ret = CAEN_DGTZ_ReadRegister(VMEhandle, V1725_BOARD_CONFIG, &boardCfg);
  std::cout << " board config: " << boardCfg << std::endl;
  // read channel trigger conf
  uint32_t valConf;
  uint32_t width;
  for(size_t i=0;i<BoardInfo.Channels/2;++i)
  {
    ret = CAEN_DGTZ_ReadRegister(VMEhandle, V1725_CHANNEL_TRIGGER_CONF  + (2*i<<8), &valConf);
    ret = CAEN_DGTZ_ReadRegister(VMEhandle, V1725_PULSE_WIDTH  + (2*i<<8), &width);
    std::cout << " channels pair " << i << " channel_trigger_conf: " << valConf << " widht: " << width << " (x8 to obntain ns ) "  << std::endl;
  }

  // ARM ACQUISITON
  CAEN_DGTZ_ClearData(VMEhandle);
  CAEN_DGTZ_SWStartAcquisition(VMEhandle);

  return SUCCESS;
}

//-- End of Run ----------------------------------------------------
INT end_of_run(INT run_number, char *error)
{
  // free readout buffer
std::cout << "stopping daq ..." << std::endl;
  CAEN_DGTZ_SWStopAcquisition(VMEhandle);
  CAEN_DGTZ_ClearData(VMEhandle);

std::cout << "free event ... " << std::endl;
if (Event16)  CAEN_DGTZ_FreeEvent(VMEhandle, (void**)&Event16);

std::cout << "free buffer ..." << std::endl;
  //CAEN_DGTZ_FreeReadoutBuffer(&buffer);
  if (buffer) {delete [] buffer; buffer=0;}


  //Reset all IRQs    

  return SUCCESS;
}

//-- Pause Run -----------------------------------------------------
INT pause_run(INT run_number, char *error)
{
  inRun = 0;
/*
  // Disable interrupt
  mvme_write_value(myvme, VLAM_BASE+4, inRun);
  // Close run gate
  vmeio_AsyncWrite(myvme, VMEIO_BASE, 0x0);
*/
   
  return SUCCESS;
}

//-- Resume Run ----------------------------------------------------
INT resume_run(INT run_number, char *error)
{
/*

  // EOB Pulse
  vmeio_SyncWrite(myvme, VMEIO_BASE, P_EOE);

  // Open run gate
  vmeio_AsyncWrite(myvme, VMEIO_BASE, S_RUNGATE);

  // Enable interrupt
  mvme_write_value(myvme, VLAM_BASE+4, inRun);
*/
  inRun = 1;
  return SUCCESS;
}

//-- Frontend Loop -------------------------------------------------
INT frontend_loop()
{

  // if frontend_call_loop is true, this routine gets called when
     //the frontend is idle or once between every event 
//if (verbose) std::cout << " LOOP. readout enabled =  "  << readout_enabled() << std::endl;

/*
  char str[128];

  if (stop_req && done==0) {
    db_set_value(hDB,0,"/logger/channels/0/Settings/Event limit", &evlimit, sizeof(evlimit), 1, TID_SHORT); 
    if (cm_transition(TR_STOP, 0, str, sizeof(str), ASYNC, FALSE) != CM_SUCCESS) {
      cm_msg(MERROR, "VF48 Timeout", "cannot stop run: %s", str);
    }
    inRun = 0;
    // Disable interrupt
    mvme_write_value(myvme, VLAM_BASE+4, inRun);
    done = 1;
    cm_msg(MERROR, "VF48 Timeout","VF48 Stop requested");
  }
*/
  return SUCCESS;
}

//------------------------------------------------------------------

/**********************************************

  Readout routines for different events

*******************************************************************/

//------------------------------------------------------------------
//------------------------------------------------------------------
//------------------------------------------------------------------
//-- Trigger event routines ----------------------------------------
// Polling routine for events. Returns TRUE if event
// is available. If test equals TRUE, don't return. The test
// flag is used to time the polling 
INT poll_event(INT source, INT count, BOOL test)
{

  ///////// CHECK EVENT IS READY
  /* Read data from the board */
  //CAENComm_ErrorCode sCAEN = CAENComm_Read32(VMEhandle, V1725_EVENT_STORED, val);
  int retval=0;
  CAEN_DGTZ_ReadData(VMEhandle, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer, &BufferSize);

  if (BufferSize != 0)  retval= 1;
  return retval;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
//------------------------------------------------------------------
//-- Interrupt configuration ---------------------------------------
// MARIA TODO
INT interrupt_configure(INT cmd, INT source, PTYPE adr)
{
  switch (cmd) {
  case CMD_INTERRUPT_ENABLE:
//    vme_bus->IrqInit( 0xF, true);
   if (verbose) std::cout << "inrun " << inRun << " interrupt enabling " << cmd << std::endl;
    break;
  case CMD_INTERRUPT_DISABLE:
   if (verbose) std::cout << "inrun " << inRun << " interrupt disabling " << cmd << std::endl;
//    vme_bus->IrqInit( 0xF, false);
    break;
  case CMD_INTERRUPT_ATTACH:
    //mvme_set_dmode(myvme, MVME_DMODE_D32);
    //vec = mvme_read_value(myvme, VLAM_BASE+0x10);
    //printf("Interrupt Attached to 0x%x for vector:0x%x\n", adr, vec&0xFF);
    printf("Interrupt Atach\n");
    break;
  case CMD_INTERRUPT_DETACH:
    printf("Interrupt Detach\n");
    break;
  }
  return SUCCESS;
}

//-- Event readout (polled)  -------------------------------------------------
INT read_trigger_event(char *pevent, INT off)
{
  static uint64_t counter = 0;
  static uint64_t prevTimeStamp = 0;

  CAEN_DGTZ_ErrorCode ret;
  //if (verbose) std::cout << " reading event ... " << std::endl;
  
  // DWORD *pidata; // MARIA  MARIA!!!!
  WORD *pidata; // MARIA short int

  // > THE LINE BELOW SHOULD BE HERE AND NOWHERE ELSE < !
  // Maria: initialie global bank header
  // OJO!!!!!
  // los bancos con 16 bits tienen tambien un tamanio maximo a 32KB
  // si quiero un banco ilimitado tengo que definirlo como de 32 bits 
  bk_init32(pevent); 

  uint32_t NumEvents = 0;
  if (BufferSize != 0) 
  {
    ret = CAEN_DGTZ_GetNumEvents(VMEhandle, buffer, BufferSize, &NumEvents);
    if (ret != CAEN_DGTZ_Success) 
    {
      std::cout << " error getting  number of events" << std::endl;
      frontend_exit();
      exit(1);
    }
  }
  else 
  {
    uint32_t lstatus;
    ret = CAEN_DGTZ_ReadRegister(VMEhandle, CAEN_DGTZ_ACQ_STATUS_ADD, &lstatus);
    if (ret != CAEN_DGTZ_Success) 
    {
      printf("Warning: Failure reading reg:%x (%d)\n", CAEN_DGTZ_ACQ_STATUS_ADD, ret);
      frontend_exit();
      exit(1);
    }
    else 
    {
      if (lstatus & (0x1 << 19)) 
      {
        std::cout << " ERR_OVERTEMP!!" << std::endl;
        frontend_exit();
        exit(1);
      }
    }
  }

  // MARIA VERBOSE
  //std::cout << " num events : " << NumEvents << " buffer size " <<  BufferSize <<  std::endl;

  int copied=0;
  for(int iev = 0; iev < (int)NumEvents; iev++) 
  {

    uint16_t flags = 0;

    ret = CAEN_DGTZ_GetEventInfo(VMEhandle, buffer, BufferSize, 0, &EventInfo, &EventPtr);
    if (ret != CAEN_DGTZ_Success) 
    {
      std::cout << " error getting event info. Ev number " << iev << " of total events " << NumEvents << " buffer size: " << BufferSize  << " mybuffer size " << myBufferSize << std::endl;
      flags = 1;
      return 0; // do not store event
  //    frontend_exit();
  //    exit(1);
    }
  // MARIA VERBOSE
  //std::cout <<" Pattern " << EventInfo.Pattern << " ChannelMask " << EventInfo.ChannelMask << " EventCounter " << EventInfo.EventCounter<< " TriggerTimeTag " << EventInfo.TriggerTimeTag << std::endl;
  // ChannelMask -> Mask of active channels


    ret = CAEN_DGTZ_DecodeEvent(VMEhandle, EventPtr, (void**)&Event16);
/*
    if (ret != CAEN_DGTZ_Success) 
    {
      std::cout << " error decoding event" << std::endl;
      frontend_exit();
      exit(1);
    }
*/

     // create bank and copy data
     char bankName [5];
     sprintf(bankName, "WF%02d",VMEBUS_BOARDNO); 
     bk_create(pevent, bankName, TID_UINT16,  (void **)&pidata); // reserva memeoria en pidata 

     // 1st word: channel mask
     *pidata++=channel_mask; 

     // second word: flags ; 0 OK, 1-> error reading
     *pidata++=flags; 

     // two words: samples per channel
     *((uint32_t*)pidata) = v1730_settings.recordlength;
     pidata+=2;

     // four words: time stamp
     // TODO aniadir check in clock. This is only valid for rate < 1 / 17 sec
     if (EventInfo.TriggerTimeTag<prevTimeStamp) counter++;
     uint64_t clock = (uint64_t)(counter<<31) + EventInfo.TriggerTimeTag;
     clock *= CLOCK2NS; 
     *((uint64_t*)pidata) = clock;
// DEB
//std::cout << clock << " tag: " << EventInfo.TriggerTimeTag << " prev: " << prevTimeStamp << " counter: " << counter << std::endl;
     prevTimeStamp = EventInfo.TriggerTimeTag;
     pidata+=4;

     for (int ch = 0; ch < (int32_t)BoardInfo.Channels; ch++)
     {
       if (v1730_settings.ch_enable[ch])
       {
         memcpy(pidata,Event16->DataChannel[ch], Event16->ChSize[ch]*sizeof(WORD));
         pidata += Event16->ChSize[ch]; // MARIA TODO
         //for (int jj=0; jj<Event16->ChSize[ch]; jj++) *pidata++ = Event16->DataChannel[ch][jj];
         //for (int jj=0; jj<Event16->ChSize[ch]; jj++) std::cout <<  Event16->DataChannel[ch][jj] << std::endl;
// VERBOSE
//std::cout << "ch " << ch << " data [0] " << Event16->DataChannel[ch][1] << std::endl;
         copied+=Event16->ChSize[ch]*sizeof(WORD); // MARIA TODO
       }

     }
     bk_close(pevent, pidata);
  }

  //VERBOSE
  //std::cout << "num ev " << NumEvents << " copied data " << copied <<  " ch size " << Event16->ChSize[0] <<  " size WORD " << sizeof(WORD) << std::endl;
  return bk_size(pevent);
}

/*******************
 * configure trigger
 ********************/
int configure_trigger()
{
  // FOR EVERY CHANNEL PAIR:
  // Reg 0x1n84 (for every pair) V1725_CHANNEL_TRIGGER_CONF
  // Bits[10] 00 AND
  //          01 ONLY0
  //          10 ONLY1
  //          11 OR
  // Bit[2]    0 programable width [when triggerWidthNs>0]
  //           1 trigger on as long as pulse above/below threshold
  // 
  // Reg 0x1n70  V1725_PULSE_WIDTH
  // Bits[7-0] width in clock units (8ns)
  //
  // GLOBAL:
  // Reg 0x810C V1725_TRIG_SRCE_EN_MASK
  // Bits [7-0] Request that take part of the global trigger
  // Bit[23-20] set the coincidence window (T TVAW ) linearly in steps of the Trigger clock (8ns)
  // Bit[26-24] set the Majority (i.e. Coincidence) level. Nreqest = 1 -> majority = 0)
  //            majority from 0 to 7
  // 
  // info in v1730_settings
  //  char      ch2_logic[8][32]; -> AND/ONLY0/ONLY1/OR/NONE
  //  INT32     triggerWidthNs[8];
  //  INT32     NRequestForCoincidence; = majority + 1
  //  INT32     coincidenceWindowNs;
  CAEN_DGTZ_ErrorCode ret;
  uint32_t request = 0;
  uint32_t majority = v1730_settings.NRequestForCoincidence - 1; 
  if (majority>7) majority = 7;
  uint32_t TVAW =  v1730_settings.coincidenceWindowNs/CLOCK2NS;
  // to be configured with 4 bits -> [0 - 15]
  if (TVAW>15) 
  {
    std::cout << " coincidence window requested " << v1730_settings.coincidenceWindowNs << " but maximum is " << 15*CLOCK2NS << " configuring " << 15*CLOCK2NS << std::endl;
    TVAW = 15;
  }


  std::cout << " configuring trigger logics............ " << std::endl;
  for(size_t i=0;i<BoardInfo.Channels/2;++i)
  {
    uint32_t valConf =0;
    uint32_t width =0;
    // 1. check if has to be included in request (check that it is different from NONE)
    if(strcmp(v1730_settings.ch2_logic[i],"NONE"))
    {
      // different from NONE! . If it is one of the allowed cases, activate bit in request
      // Bits[10] 00 AND
      //          01 ONLY0
      //          10 ONLY1
      //          11 OR
      if(!strcmp(v1730_settings.ch2_logic[i],"AND")) { valConf = 0; request |= (1<<i); }
      if(!strcmp(v1730_settings.ch2_logic[i],"ONLY0")) { valConf = 1; request |= (1<<i); }
      if(!strcmp(v1730_settings.ch2_logic[i],"ONLY1")) { valConf = 2; request |= (1<<i); }
      if(!strcmp(v1730_settings.ch2_logic[i],"OR")) { valConf = 3; request |= (1<<i); }
      // check programmable window
      //if (v1730_settings.triggerWidthNs[i]==0) valConf |=4; // rise bit 2
      //else width =  v1730_settings.triggerWidthNs[i]/CLOCK2NS;
      // MARIA 160222 TEST!!
      valConf |=4; // rise bit 2
      width =  v1730_settings.triggerWidthNs[i]/CLOCK2NS;
      if (width>255) width = 255; // codified in 1 Byte
    }
    ret = CAEN_DGTZ_WriteRegister(VMEhandle, V1725_CHANNEL_TRIGGER_CONF  + (2*i<<8), valConf);
    if (ret!=CAEN_DGTZ_Success) return ret;
    ret = CAEN_DGTZ_WriteRegister(VMEhandle, V1725_PULSE_WIDTH  + (2*i<<8), width);
    if (ret!=CAEN_DGTZ_Success) return ret;
 
    std::cout << " couple of channels " << 2*i << " " << 2*i+1 << " : conf mask " << valConf << " width " << width << std::endl;

  }
  // Reg 0x810C V1725_TRIG_SRCE_EN_MASK
  // Bits [7-0] Request that take part of the global trigger
  // Bit[23-20] set the coincidence window (T TVAW ) linearly in steps of the Trigger clock (8ns)
  // Bit[26-24] set the Majority (i.e. Coincidence) level. Nreqest = 1 -> majority = 0)
  //request ,  majority, TVAW 
  request |= (TVAW << 20);
  request |= (majority << 24);
  ret = CAEN_DGTZ_WriteRegister(VMEhandle, V1725_TRIG_SRCE_EN_MASK , request);
  std::cout << " global request:  " << std::hex << request << std::dec << std::endl;

  return ret;
}

/*! \fn set relative threshold
*/
int set_relative_Threshold()
{
	int ch = 0, i = 0;
	CAEN_DGTZ_ErrorCode ret;

	uint32_t custom_posttrg = 50;
	int baseline[MAX_CH] = { 0 }, size = 0;
	int rms[MAX_CH] = { 0 };
    int samples = 20; // TODO CONFIGURE IN DB
    int nEvents = 200; // TODO CONFIGURE IN DB

	///malloc
    
    if (!buffer) 
    {
      buffer = new char [myBufferSize]; 
      ret = CAEN_DGTZ_AllocateEvent(VMEhandle, (void**)&Event16);
      if (ret != CAEN_DGTZ_Success) 
      {  
        std::cout << " Failure allocating buffer!. Digitizer error code: " << ret << std::endl; 
        frontend_exit();
        exit(1);
      }
    }

	//some custom settings
	ret = CAEN_DGTZ_SetPostTriggerSize(VMEhandle, custom_posttrg);
	if (ret) {
		printf("Threshold calc failed. Error trying to set post trigger!!\n");
		return -1;
	}

	CAEN_DGTZ_ClearData(VMEhandle);
	CAEN_DGTZ_SWStartAcquisition(VMEhandle);
#ifdef _WIN32
	Sleep(300);
#else
	usleep(300000);
#endif

    for (int id=0; id<nEvents; id++) // loop in number of events 
    {
	  CAEN_DGTZ_SendSWtrigger(VMEhandle);

	  ret = CAEN_DGTZ_ReadData(VMEhandle, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer, &BufferSize);
	  if (ret || BufferSize==0) 
      {
        std::cout << " error reading data after software trigger " << std::endl;
        return -1;
	  }

	  ret = CAEN_DGTZ_GetEventInfo(VMEhandle, buffer, BufferSize, 0, &EventInfo, &EventPtr);
	  if (ret) 
      {
        std::cout << " error in get event info " << std::endl;
        return -1;
	  }
	  ret = CAEN_DGTZ_DecodeEvent(VMEhandle, EventPtr, (void**)&Event16);
	  if (ret) 
      {
        std::cout << " error in decode event " << std::endl;
        return -1;
	  }

	  for (ch = 0; ch < (int32_t)BoardInfo.Channels; ch++) 
      {
        if (v1730_settings.ch_enable[ch]) 
        {
		  size = Event16->ChSize[ch];
          double bsl=0;
          double auxrms=0;

		  //use some samples to calculate the baseline
		  for (i = 0; i < samples; i++) 
          {
            bsl += (int)(Event16->DataChannel[ch][i]);
            auxrms += (int)(Event16->DataChannel[ch][i])*(Event16->DataChannel[ch][i]);
          }
          auxrms/=samples;
          auxrms-=bsl*bsl/samples/samples;
          auxrms=sqrt(auxrms);
		  baseline[ch] += bsl / samples;
		  rms[ch] += auxrms;
        }
      }
    } // en loop number  of events

	CAEN_DGTZ_SWStopAcquisition(VMEhandle);

	//reset posttrigger
	ret = CAEN_DGTZ_SetPostTriggerSize(VMEhandle, v1730_settings.posttrigger); 
	if (ret) 
    {
      std::cout << " error setting posttrigger size " << std::endl;
      return -1;
	}

	CAEN_DGTZ_ClearData(VMEhandle);

    //CAEN_DGTZ_FreeReadoutBuffer(&buffer);
    //if (buffer) {delete [] buffer; buffer = 0;}
	//CAEN_DGTZ_FreeEvent(VMEhandle, (void**)&Event16);


   // set the threshold
   std::cout << " baseline summary for " << enabledChannels << " channels " << std::endl;
   for (ch = 0; ch < (int32_t)BoardInfo.Channels; ch++) 
   {
     if (v1730_settings.ch_enable[ch]) 
     {
        baseline[ch] /= nEvents;
        rms[ch] /= nEvents;
        int polarity = -1;
        if (!strcmp(&v1730_settings.pulsePolarity,"+")) polarity = +1;
    
        // MARIA TODO: convert from mV to ADC
        double thr = v1730_settings.ch_threshold[ch];
        thr=(DWORD)baseline[ch]  + polarity*thr;

        // checks
        if (thr<0) thr=0; 
	    size = (int)pow(2, (double)BoardInfo.ADC_NBits);
        if (thr>(uint32_t)size) thr=size; 

        ret = CAEN_DGTZ_SetChannelTriggerThreshold(VMEhandle, ch, thr);
        if (ret) 
        {
          std::cout << " error setting threshold of channel  "<< ch << std::endl;
          return -1;
        }
        std::cout << " ch " << ch <<  " threshold " << v1730_settings.ch_threshold[ch];
        std::cout << " baseline " <<  baseline[ch] << " threshold set to " << thr << " rms: " << rms[ch] << std::endl;
     } // end loop in channels
   }//end sw trigger event analysis

   return 0;
}

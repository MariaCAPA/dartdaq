/******************************************************************

  Name:         fe2730.c
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
#include <atomic>
#include <sys/time.h>
#include <sched.h>
#include <sys/resource.h>

#include <fstream>
#include <string>
#include <vector>
#include <memory>

#include "midas.h"
#include "msystem.h"
// MARIA 2730
//#include "CAENDigitizer.h"
//#include "CAENComm.h"
#include <CAEN_FELib.h>

#include "mfe.h"

typedef unsigned char BYTE;
typedef unsigned short int UINT16;
typedef short int INT16;
typedef unsigned int UINT32;
typedef int INT32;
//typedef long int INT64;

#include "experim.h"
// Max event size  supported (in bytes).
// up to 1ms per event ->
// 1000 us * 500 S/us * 2 B/S * 16 ch = 16MB
//#define V1730_MAX_EVENT_SIZE 16000000

// 20 us * 500 S/us * 2 B/S * 16 ch = 320KB 
// 20 us * 500 S/us * 2 B/S * 32 ch = 640KB  + security bytes
#define max_record_length 10000
#define V2730_MAX_EVENT_SIZE 641000

//#define MAXEV_SINGLEREADOUT 1024

// MARIA 2730
//#define MAXEVMIDASINBUFFER 100
#define MAXEVMIDASINBUFFER 1000

// MARIA 2730
//#define MAX_CH 16
#define MAX_CH 32
#define CLOCK2NS 8
#define TIMEOUT_MS	(100)
DWORD VMEBUS_BOARDNO = 0;
// MARIA 2730
// VME base address 
//DWORD V1730_BASE =   0; // 0x32100000; // 0-> optical link in module
//DWORD LINK = 1;
//std::string devicePath = "dig2://caendgtz-usb-51553";
std::string devicePath = "dig2://caendgtz-usb-52037";

WORD V2730EVENTID = 1;
WORD V2730TRIGGERMASK = 0;

// MARIA 2730
//int VMEhandle=-1;
uint64_t dev_handle; // DEVICE HANDLE
uint64_t ep_handle; // END POINT HANDLE 
// done MARIA 2730



int verbose = 1;

//-- Globals -------------------------------------------------------
// MARIA 2730
//CAEN_DGTZ_BoardInfo_t BoardInfo;
struct RunInfo 
{
  time_t startDate;
  time_t stopDate;
  uint16_t nChannels; 
  uint16_t nActiveChannels;
  uint16_t activeChannels [MAX_CH]; // number of the active channel from 0..nActiveChannels-1
  double   ADC2Volt [MAX_CH];
  uint16_t nSamples;
  uint16_t samplingRate; 
  uint32_t realTime;
  uint32_t deadTime;
  uint32_t nEvents;
};  
struct RunInfo runInfo;
// done MARIA 2730


// MARIA 2730
// TODO!!! the buffer is not longer necesary, 
// every call to ReadData will return one (and only one) event?
//CAEN_DGTZ_EventInfo_t       EventInfo;
//uint32_t BufferSize;
//char *auxBuffer = NULL;
//uint32_t auxBufferSize=0; 
//char *EventPtr = NULL;
//CAEN_DGTZ_UINT16_EVENT_t    *Event16 = NULL;
#define DATA_FORMAT " \
        [ \
                { \"name\" : \"TIMESTAMP\", \"type\" : \"U64\" }, \
                { \"name\" : \"TRIGGER_ID\", \"type\" : \"U32\" }, \
                { \"name\" : \"WAVEFORM\", \"type\" : \"U16\", \"dim\" : 2 }, \
                { \"name\" : \"WAVEFORM_SIZE\", \"type\" : \"SIZE_T\", \"dim\" : 1 }, \
                { \"name\" : \"EVENT_SIZE\", \"type\" : \"SIZE_T\" } \
        ] \
"
struct EventData
{
  uint64_t timestamp; // to be read by ReadData
  uint32_t trigger_id; // to be read by ReadData
  uint16_t** waveform; // to be read by ReadData
  size_t *nSamples; // to be read by ReadData
  size_t eventSize; // to be read by ReadData
  size_t nAllocatedSamples;
  size_t nChannels;
};
struct EventData * ptEvent=0;
// done MARIA 2730



// The frontend name (client name) as seen by other MIDAS clients   
const char *frontend_name = "fe2730Th";
// The frontend file name, don't change it 
const char *frontend_file_name = __FILE__;

// frontend_loop is called periodically if this variable is TRUE    
BOOL frontend_call_loop = TRUE; //FALSE;

// a frontend status page is displayed with this frequency in ms 
INT display_period = 000;

// MARIA 2730 BUFFER DIMENSIONS USED BY MIDAS
// dimensions of MIDAS buffer
// maximum event size produced by this frontend 
INT max_event_size = V2730_MAX_EVENT_SIZE;

// buffer size to hold up to MAXEVMIDASINBUFFER  events 
INT event_buffer_size = MAXEVMIDASINBUFFER * max_event_size + 10000;

// maximum event size for fragmented events (EQ_FRAGMENTED) 
INT max_event_size_frag = 5 * 1024 * 1024;

// dimensions of rb buffer
//  1 event can be as big as the whole 1730 internal buffer , plus header
INT rb_max_event_size = 2*5.12*1024*1024*16 + 1024*16; 
INT rb_event_buffer_size = 1024*1024*1024; // 1GB : MAX ALLOWED


// CLOCK. Maria 040723
uint64_t counter = 0;
uint64_t prevTimeStamp = 0;

// Hardware 
int  inRun = 0, missed=0;
int done=0, stop_req=0;
DWORD evlimit;

// Globals 
extern HNDLE hDB;  // FROM mfe.h
HNDLE hSet;
V2730_DATA00_SETTINGS v2730_settings;
// MARIA 2730
//uint16_t channel_mask; // Mask of enabled channels
uint32_t channel_mask; // Mask of enabled channels
int enabledChannels;
int rb_handle; // handle of the circular buffer
std::atomic<int> num_events_in_rb_;  //!< Number of events stored in ring buffer
int GetNumEventsInRB() {return num_events_in_rb_.load();}
void IncrementNumEventsInRB() { num_events_in_rb_++; }
void IncrementNumEventsInRB(int n) { num_events_in_rb_+=n;}
void DecrementNumEventsInRB() { num_events_in_rb_--; }
void DecrementNumEventsInRB(int n) { num_events_in_rb_-=n; }
void ResetNumEventsInRB() { num_events_in_rb_=0; }



// Multithread stuff
pthread_t tid;          //!< Thread ID
int thread_retval = 0;  //!< Thread return value
int thread_link=0;        //!< Link number associated with each thread
#define NBCORES           8   //!< Number of cpu cores, for process/thread locking

bool runInProgress = false; //!< run is in progress
bool stopRunInProgress = false; //!< stop run is in progress (empty buffers)



//-- Function declarations -----------------------------------------

int frontend_init();
INT frontend_exit();
INT begin_of_run(INT run_number, char *error);
INT end_of_run(INT run_number, char *error);
INT pause_run(INT run_number, char *error);
INT resume_run(INT run_number, char *error);
INT frontend_loop();
extern void interrupt_routine(void);

INT read_event_from_rb(char *pevent, INT off);

void register_cnaf_callback(int debug);
//int set_relative_Threshold();
int configureTrigger();
void * readThread(void * arg);
// MARIA 2730. This function is not used
//INT checkEvent();

INT readEvent(void * );

// 
// MARIA 2730
// New functions for board 2730
int getBoardInfo(); //  dev_handle is global
int configureEndPoint(uint64_t * ep_handle); // set data format, dev_handle and  ep_handle are globals
int allocateEvent();
void freeEvent();
int printLastError();
int setRelativeThreshold();
// done MARIA 2730



BOOL equipment_common_overwrite = TRUE;

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

   {"V2730_Data00",               // equipment name 
    {V2730EVENTID, V2730TRIGGERMASK,                   // event ID, trigger mask 
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
    read_event_from_rb,      // readout routine 
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
  // MARIA 2730
  //CAEN_DGTZ_ErrorCode ret;
  int ret;
 
std::cout << " FRONTEND INIT!!!! " << std::endl;
#ifdef USE_INT
std::cout << " interruptioon " << std::endl;
#else
std::cout << " polled " << std::endl;
#endif
  int size, status;
  char set_str[80];

  // Book Setting space 
  V2730_DATA00_SETTINGS_STR(v2730Settings_str);

  // Map /equipment/V2730_Data00/Settings for the sequencer 
  sprintf(set_str, "/Equipment/V2730_Data00/Settings");
  status = db_create_record(hDB, 0, set_str, strcomb(v2730Settings_str));
  status = db_find_key (hDB, 0, set_str, &hSet);
  if (status != DB_SUCCESS)
    cm_msg(MINFO,"FE","Key %s not found", set_str);
  
  // Enable hot-link on settings/ of the equipment 
  size = sizeof(V2730_DATA00_SETTINGS);
  if ((status = db_open_record(hDB, hSet, &v2730_settings, size, MODE_READ
                               , seq_callback, NULL)) != DB_SUCCESS) return status;

  ////////////////////////// v2730_settings contains the configuration

  ////////////////////////
  // Open VME interface, init link board_number
  // MARIA 2730
  /*
  ret = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_OpticalLink, LINK, VMEBUS_BOARDNO,V2730_BASE, &VMEhandle);
  //ret = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_USB, 0, 0,0, &VMEhandle);
  if (ret != CAEN_DGTZ_Success) 
  { 
    std::cout << " Error opening Digitizer. Digitizer error code: " << ret << std::endl;
    frontend_exit();
    exit(1);
  }
  */
  ret = CAEN_FELib_Open(devicePath.c_str(), &dev_handle);
  if (ret != CAEN_FELib_Success)
  {
    printLastError();
    return EXIT_FAILURE;
  }





  // MARIA 2730
  ///////////////////// reset
  /* 
  ret = CAEN_DGTZ_Reset(VMEhandle);            
  if (ret != CAEN_DGTZ_Success) 
  { 
    std::cout << " Error Resetting Digitizer. Digitizer error code: " << ret << std::endl;
    frontend_exit();
    exit(1);
  }
  */
   // reset digitizer
  printf("reset digitizer ...\t");
  ret = CAEN_FELib_SendCommand(dev_handle, "/cmd/reset");
  if (ret != CAEN_FELib_Success)
  {
    printLastError();
    return EXIT_FAILURE;
  }
  // done MARIA 2730

  // MARIA 2730
  /*
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
  */
  ret=getBoardInfo(); // get n channels
  if (ret != CAEN_FELib_Success)
  {
    printLastError();
    return EXIT_FAILURE;
  }
  // done MARIA 2730

  // TODO CALIBRATE PEDESTASLS

  // set main thread to core 0
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(0, &mask);  //Main thread to core 0
  if( sched_setaffinity(getpid(), sizeof(mask), &mask) < 0 )
  {
    printf("ERROR setting cpu affinity for main thread: %s\n", strerror(errno));
  }


  
  return SUCCESS;
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------
//-----------------------------------------------------------------
//-- Frontend Exit -------------------------------------------------

INT frontend_exit()
{
  // MARIA 2730
  /*
  CAEN_DGTZ_ErrorCode ret;
  ret = CAEN_DGTZ_CloseDigitizer(VMEhandle);
  if (ret != CAEN_DGTZ_Success) 
  { 
    std::cout << " Error closing digitizer. Digitizer error code: " << ret << std::endl;
  }
  */ 
  int ret = CAEN_FELib_Close(dev_handle);
  if (ret != CAEN_FELib_Success)
  {
    printLastError();
    return EXIT_FAILURE;
  }
  return SUCCESS;
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------
//-----------------------------------------------------------------
//-- Begin of Run --------------------------------------------------

INT begin_of_run(INT run_number, char *error)
{
  // MARIA 2730
  //CAEN_DGTZ_ErrorCode ret;
  int ret;

  runInProgress = true;

  // read V2730 settings 
  int status;
  int size = sizeof(V2730_DATA00_SETTINGS);
  if ((status = db_get_record (hDB, hSet, &v2730_settings, &size, 0)) != DB_SUCCESS) return status;

  // MARIA 2730
  /*
  // reset board
  ret = CAEN_DGTZ_Reset(VMEhandle);            
  if (ret != CAEN_DGTZ_Success) 
  { 
    std::cout << " Error Resetting Digitizer. Digitizer error code: " << ret << std::endl;
    frontend_exit();
    exit(1);
  }
  */
  // Maria: change reset to clear data
  printf("clear data in digitizer ...\t");
  ret = CAEN_FELib_SendCommand(dev_handle, "/cmd/cleardata");
  if (ret != CAEN_FELib_Success)
  {
    printLastError();
    return EXIT_FAILURE;
  }
  // done MARIA 2730

  ///////////////////// CONFIGURATION

  ///////////////// ENABLE CHANNELS
  channel_mask=0;
  enabledChannels=0;
  char par_name[256];
  char value [256];
  for(size_t i=0;i<MAX_CH;++i)
  {
    snprintf(par_name, sizeof(par_name), "/ch/%zu/par/ChEnable", i);
    if (v2730_settings.ch_enable[i]) 
    {
      ret = CAEN_FELib_SetValue(dev_handle, par_name, "true");
      if (ret != CAEN_FELib_Success) return ret;
      channel_mask|=(1<<i);
      if(verbose) printf(" Channel %d enabled \n", (int)i);
      // update runInfo
      runInfo.activeChannels[enabledChannels]=i;
      enabledChannels++;
    }
    else
      ret = CAEN_FELib_SetValue(dev_handle, par_name, "false");
  }
  // update runInfo
  runInfo.nActiveChannels=enabledChannels;

  // Maria 2730
  //ret = CAEN_DGTZ_SetChannelEnableMask(VMEhandle,channel_mask);
  if (verbose) std::cout <<  " channel mask " << std::hex << (int)channel_mask << std::dec << std::endl;

  ///////////////// RECORD LENGTH
  DWORD testVal;
  // MARIA 2730
  /*
  ret = CAEN_DGTZ_SetRecordLength(VMEhandle,v1730_settings.recordlength);     // Set the lenght of each waveform (in samples) 
  if (ret != CAEN_DGTZ_Success) 
    std::cout << " Error Cannot set record length to " << v1730_settings.recordlength << " Digitizer error code: " << ret << std::endl;
  ret = CAEN_DGTZ_GetRecordLength(VMEhandle, &v1730_settings.recordlength);
  if (ret != CAEN_DGTZ_Success) 
    std::cout << " Error Cannot set record length to " << v1730_settings.recordlength << " Digitizer error code: " << ret << std::endl;
  */
  if(v2730_settings.recordlength>max_record_length)v2730_settings.recordlength=max_record_length;
  snprintf(value, sizeof(value), "%u", v2730_settings.recordlength);
  ret = CAEN_FELib_SetValue(dev_handle, "/par/RecordLengthS", value);
  if (ret != CAEN_FELib_Success)
    std::cout << " Error Cannot set record length to " << v2730_settings.recordlength << " Digitizer error code: " << ret << std::endl;
  // READ AGAIN
  ret = CAEN_FELib_GetValue(dev_handle, "/par/RecordLengthS", value);
  if (ret != CAEN_FELib_Success) 
    std::cout << " Error Cannot set record length to " << v2730_settings.recordlength << " Digitizer error code: " << ret << std::endl;
  testVal = atoi(value);
  printf("record length set to :\t%d\n", testVal);
  if (testVal!=v2730_settings.recordlength) std::cout << " WARNING : RECORD LENGTH SET TO : " << testVal;
  v2730_settings.recordlength = testVal;
  runInfo.nSamples = testVal;
  // done MARIA 2730
  
  // MARIA 2730
  ///////////////// POSTTRIGGER. In percent of the whole acquisition window
  /*
  testVal = v1730_settings.posttrigger; 
  ret = CAEN_DGTZ_SetPostTriggerSize(VMEhandle,v1730_settings.posttrigger);  // Set the posttrigger for each waveform (in samples) 
  if (ret != CAEN_DGTZ_Success) 
    std::cout << " Error Cannot set posttrigger to " << v1730_settings.posttrigger << " Digitizer error code: " << ret << std::endl;
  ret = CAEN_DGTZ_GetPostTriggerSize(VMEhandle, &v1730_settings.posttrigger);
  if (ret != CAEN_DGTZ_Success) 
    std::cout << " Error Cannot set posttrigger to " << v1730_settings.posttrigger << " Digitizer error code: " << ret << std::endl;
  if (testVal!=v1730_settings.posttrigger) std::cout << " WARNING : POSTRIGGER  SET TO : " << v1730_settings.posttrigger;
  */
  snprintf(value, sizeof(value), "%u", v2730_settings.pretrigger);
  ret = CAEN_FELib_SetValue(dev_handle, "/par/PreTriggerS", value);
  if (ret != CAEN_FELib_Success)
    std::cout << " Error Cannot set pretrigger to " << v2730_settings.pretrigger << " Digitizer error code: " << ret << std::endl;
  // READ AGAIN
  ret = CAEN_FELib_GetValue(dev_handle, "/par/PreTriggerS", value);
  if (ret != CAEN_FELib_Success) 
    std::cout << " Error Cannot set pretrigger to " << v2730_settings.pretrigger << " Digitizer error code: " << ret << std::endl;
  testVal = atoi(value);
  printf("pretrigger set to :\t%d\n", testVal);
  if (testVal!=v2730_settings.pretrigger) std::cout << " WARNING : PRETRIGGER SET TO : " << testVal;
  v2730_settings.pretrigger = testVal;


  ////////////// NIM LEVELS
  // MARIA 2730
  /*
  ret = CAEN_DGTZ_SetIOLevel(VMEhandle, CAEN_DGTZ_IOLevel_NIM);
  if (ret != CAEN_DGTZ_Success) 
    std::cout << " Error Cannot set levels to NIM.  Digitizer error code: " << ret << std::endl;
  */
  ret = CAEN_FELib_SetValue(dev_handle, "/par/iolevel", "NIM");
  if (ret != CAEN_FELib_Success)
    std::cout << " Error Cannot set levels to NIM. Digitizer error code: " << ret << std::endl;

  //////////////// CHANNEL GAIN , OFFSET AND TRIGGER THRESHOLD
  // MARIA 2730
  //for(size_t i=0;i<BoardInfo.Channels;++i)
  for(size_t i=0;i<runInfo.nChannels;++i)
  {
    if (v2730_settings.ch_enable[i])
    {
       // MARIA 2730
       // DC Offset goes from 1 to 65535
       //uint32_t  dcoffset = v2730_settings.ch_bslpercent[i] * 655.35;
       //ret = CAEN_DGTZ_SetChannelDCOffset(VMEhandle, i, dcoffset);
       //ret = CAEN_DGTZ_GetChannelDCOffset(VMEhandle, i, &dcoffset);
       //ret = CAEN_DGTZ_SetChannelTriggerThreshold(VMEhandle, i, v1730_settings.ch_threshold[i]);
       // In 2730, offset in percentage
       if ( v2730_settings.ch_bslpercent[i]<0)  v2730_settings.ch_bslpercent[i]=0;
       if ( v2730_settings.ch_bslpercent[i]>100)  v2730_settings.ch_bslpercent[i]=100;
     
       snprintf(par_name, sizeof(par_name), "/ch/%zu/par/dcoffset", (size_t)i);
       snprintf(value, sizeof(value), "%u", v2730_settings.ch_bslpercent[i]);
       ret = CAEN_FELib_SetValue(dev_handle, par_name, value);
       if (ret != CAEN_FELib_Success) return ret;
       // READ AGAIN
       ret = CAEN_FELib_GetValue(dev_handle, par_name, value);
       if (ret != CAEN_FELib_Success) return ret;
       v2730_settings.ch_bslpercent[i] = atof(value);
       printf("dc offset ch %d set to :\t%u\n", (int)i, v2730_settings.ch_bslpercent[i] );

       // MARIA 2730
       //bool dyn = 0; // default +- 2V
       //if (v2730_settings.ch_dynamicrange[i]<2) dyn=1;
       //ret = CAEN_DGTZ_WriteRegister(VMEhandle, V1725_DYNAMIC_RANGE  + (i<<8), dyn);
       //std::cout << " channel " << i << " dynamic range: " << (dyn ? "+-0.5 V" : " +-2 V") << std::endl;
       double gain = v2730_settings.ch_gain[i];
       int decibel = round(20.*log10(gain));
       // limits
       if (decibel<0) decibel=0;
       else if (decibel>29) decibel=29;
       snprintf(par_name, sizeof(par_name), "/ch/%zu/par/chgain", (size_t)i);
       snprintf(value, sizeof(value), "%u", decibel);
       ret = CAEN_FELib_SetValue(dev_handle, par_name, value);
       if (ret != CAEN_FELib_Success) return ret;
       // READ AGAIN
       ret = CAEN_FELib_GetValue(dev_handle, par_name, value);
       if (ret != CAEN_FELib_Success) return ret;
       v2730_settings.ch_gain[i] = pow(10,atof(value)/20.);
       // READ ADCTOVOLT
       snprintf(par_name, sizeof(par_name), "/ch/%zu/par/adctovolts", (size_t)i);
       ret = CAEN_FELib_GetValue(dev_handle, par_name, value);
       if (ret != CAEN_FELib_Success) return ret;
       runInfo.ADC2Volt[i]=atof(value); 
       printf("gain ch %d set to :\t%f\n", (int)i, v2730_settings.ch_gain[i] );


       // MARIA 2730 TODO
       /*
       if (!strcmp(&v2730_settings.pulsePolarity,"+")) 
         ret = CAEN_DGTZ_SetTriggerPolarity(VMEhandle, i, CAEN_DGTZ_TriggerOnRisingEdge); 
       else ret = CAEN_DGTZ_SetTriggerPolarity(VMEhandle, i, CAEN_DGTZ_TriggerOnFallingEdge);
       */
    }
  }

  // MARIA 2730
  // TODO check if this is the correct place to setup the endpoint
  // CONFIGURE ENDPOINT
  // TO SET THE DATA FORMAT
  printf("configure end point...\t");
  ret = configureEndPoint(&ep_handle);
  if (ret != CAEN_FELib_Success)
  {
    printLastError();
    return EXIT_FAILURE;
  }

  std::cout << " end point configured. handle: " << ep_handle << std::endl;

  // MARIA 2730. It was after configuring trigger. TODO check that it is ok here
  // allocate stuff
  // do alwais AFTER configuring the nsamples and nchannels (configure_digitizer)
  ret =  allocateEvent();
  if (ret != SUCCESS) 
  {  
    std::cout << " Failure allocating buffer!. error code: " << ret << std::endl; 
    frontend_exit();
    exit(1);
  }
  // done MARIA 2730

  //////////////// CONFIGURE TRIGGER LOGIC
  ret = configureTrigger(); // MARIA BY NOW
  if (ret != CAEN_FELib_Success) std::cout << " Error configuring trigger logics. Digitizer error code: " << ret << std::endl; 
  else std::cout << "TRIGGER CONF OK" << std::endl;

  //ret = CAEN_DGTZ_SetSWTriggerMode(VMEhandle,CAEN_DGTZ_TRGMODE_ACQ_ONLY);         // Set the behaviour when a SW tirgger arrives 

 // set acquisition mode: starts by software. Other options are
 // CAEN_DGTZ_SW_CONTROLLED             = 0L,
 // CAEN_DGTZ_S_IN_CONTROLLED           = 1L,
 // CAEN_DGTZ_FIRST_TRG_CONTROLLED      = 2L,
 // CAEN_DGTZ_LVDS_CONTROLLED           = 3L,

 // MARIA 2730 CHECK IF IT IS NECESSARY TODO
 //ret = CAEN_DGTZ_SetAcquisitionMode(VMEhandle,CAEN_DGTZ_SW_CONTROLLED);   
 //if (ret != CAEN_DGTZ_Success) std::cout << " Error Cannot set acquisition sw controlled. Digitizer error code: " << ret << std::endl; 

  // MARIA 2730
  // TODO por ahora configurar tambien trigger externo. luego ya veremos
  // las opciones son CAEN_DGTZ_TRGMODE_DISABLED; CAEN_DGTZ_TRGMODE_ACQ_ONLY; CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT;
  //ret = CAEN_DGTZ_SetExtTriggerInputMode(VMEhandle, (v2730_settings.externaltrigger ? CAEN_DGTZ_TRGMODE_ACQ_ONLY: CAEN_DGTZ_TRGMODE_DISABLED));
  //if (ret != CAEN_DGTZ_Success) std::cout << " Error Cannot set external trigger. Digitizer error code: " << ret << std::endl; 
   if (v2730_settings.externaltrigger)
   {
     ret = CAEN_FELib_SetValue(dev_handle, "/par/AcqTriggerSource", "ITLA|TrgIn");
     // MARIA 041224 test
     //ret = CAEN_FELib_SetValue(dev_handle, "/par/AcqTriggerSource", "ITLA|TrgIn|SwTrg");
     if (ret != CAEN_FELib_Success) { printLastError(); return EXIT_FAILURE; }
   }
   else
   {
     ret = CAEN_FELib_SetValue(dev_handle, "/par/AcqTriggerSource", "ITLA");
     if (ret != CAEN_FELib_Success) { printLastError(); return EXIT_FAILURE; }
   }
   // done MARIA 2730

  std::cout << " active channels :  " << enabledChannels <<  " N samples: " <<  v2730_settings.recordlength << std::endl;

  // MARIA 2730. This is not necessary anymore
  /*
  uint32_t lstatus;
  ret = CAEN_DGTZ_ReadRegister(VMEhandle, V1725_BUFFER_ORGANIZATION, &lstatus);
  int NBUFFERS = pow(2,lstatus);
  std::cout << " buffer organization . Number of buffers: " << NBUFFERS  << std::endl;

  // Set the max number of events to transfer in a sigle readout
  ret = CAEN_DGTZ_SetMaxNumEventsBLT(VMEhandle,1023);  // MARIA 130422      

  // read channel trigger conf
  uint32_t boardCfg;
  ret = CAEN_DGTZ_ReadRegister(VMEhandle, V1725_BOARD_CONFIG, &boardCfg);
  std::cout << " board config: " << boardCfg << std::endl;
  uint32_t valConf;
  uint32_t width;
  // MARIA 2730
  //for(size_t i=0;i<BoardInfo.Channels/2;++i)
  for(size_t i=0;i<runInfo.nChannels/2;++i)
  {
    ret = CAEN_DGTZ_ReadRegister(VMEhandle, V1725_CHANNEL_TRIGGER_CONF  + (2*i<<8), &valConf);
    ret = CAEN_DGTZ_ReadRegister(VMEhandle, V1725_PULSE_WIDTH  + (2*i<<8), &width);
    std::cout << " channels pair " << i << " channel_trigger_conf: " << valConf << " widht: " << width << " (x8 to obntain ns ) "  << std::endl;
  }

  */

  // MARIA 2730
  // There is no buffer now. We can keep same memory for the whole session. 
  // Reserve in Open digitizer and free in close digitizer
  /*
  ret = CAEN_DGTZ_MallocReadoutBuffer(VMEhandle, &auxBuffer, &auxBufferSize);
  if (ret != CAEN_DGTZ_Success) 
  {  
    std::cout << " Failure allocating buffer!. Digitizer error code: " << ret << std::endl; 
    frontend_exit();
    exit(1);
  }

  // Allocate memory for event data and redout buffer
  ret = CAEN_DGTZ_AllocateEvent(VMEhandle, (void**)&Event16);
  if (ret != CAEN_DGTZ_Success) 
  {  
    std::cout << " Failure allocating buffer!. Digitizer error code: " << ret << std::endl; 
    frontend_exit();
    exit(1);
  }
  */

  

  std::cout <<  " Board configuration done " << std::endl;

  // SET RELATIVE THRESHOLD

  // MARIA 2730 this is done inside setRelativeThreshold
  /*
  // ACTIVATE ALSO SOFTWARE TRIGGER FOR BSL COMPUTING
  ret = CAEN_DGTZ_SetSWTriggerMode(VMEhandle, CAEN_DGTZ_TRGMODE_ACQ_ONLY);
  if (ret != CAEN_DGTZ_Success) std::cout << " Error Cannot set software trigger. Digitizer error code: " << ret << std::endl; 
  */

  // MARIA 2730
  // now it is call inside configureTrigger
  /*
  int retVal = set_relative_Threshold();
  if (retVal != 0) 
  {  
    std::cout << " Failure in setting relative threshold"  << std::endl; 
    frontend_exit();
    exit(1);
  }
  std::cout <<  " relative threshold done " << std::endl;
  */


  // ENABLE IRQ???

  printf("End of begin_of_run\n");


  // CREATE CIRCULAR BUFFER 
  status = rb_create(rb_event_buffer_size, rb_max_event_size, &rb_handle);
  ResetNumEventsInRB(); // set number to 0
  std::cout << " Circular buffer created with size " << rb_event_buffer_size << " and max ev size: " << rb_max_event_size << " and handle " << rb_handle << std::endl;
  if(status != DB_SUCCESS)
  {
      cm_msg(MERROR, "fe2730Th", "Failed to create circular buffer"); 
      exit(1);
  }

  // SET CLOCK COUNTERS TO 0. Maria 040723
  counter = 0;
  prevTimeStamp = 0;


  // MARIA 2730
  // ARM ACQUISITON
  //CAEN_DGTZ_ClearData(VMEhandle);
  //CAEN_DGTZ_SWStartAcquisition(VMEhandle);

  // START DAQ
 /*
  // MARIA 041224 pruebo a no hacer este clear data
  ret = CAEN_FELib_SendCommand(dev_handle, "/cmd/cleardata");
  if (ret != CAEN_FELib_Success) 
  {
    printLastError();
    return EXIT_FAILURE;
  }
*/
  ret = CAEN_FELib_SendCommand(dev_handle, "/cmd/armacquisition");
  if (ret != CAEN_FELib_Success) 
  {     
    printLastError();
    return EXIT_FAILURE;
  }
  ret = CAEN_FELib_SendCommand(dev_handle, "/cmd/swstartacquisition");
  if (ret != CAEN_FELib_Success) 
  {     
    printLastError();
    return EXIT_FAILURE;
  }     
  // done Maria 2730

  usleep(300000); // MARIA 130422

  // CREATE READ THREAD
  status = pthread_create(&tid, NULL, &readThread, (void*)&thread_link);
  if(status)
  {
      cm_msg(MERROR,"fe2730Th", "Couldn't create thread for read. Return code: %d", status);
      exit(1);
  }

  return SUCCESS;
}

//-- End of Run ----------------------------------------------------
INT end_of_run(INT run_number, char *error)
{
  int ret;
  int * status;
  if(runInProgress)
  { 
    runInProgress = false;  //Signal threads to quit

    // Do not quit parent before children processes, wait for the proper
    // child exit first.
    pthread_join(tid,(void**)&status);
    printf(">>> Thread %d joined, return code: %d\n", (int)tid, *status);

    std::cout << "stopping daq ..." << std::endl;
    // Maria 2730
    //CAEN_DGTZ_SWStopAcquisition(VMEhandle);
    //CAEN_DGTZ_ClearData(VMEhandle);
    ret = CAEN_FELib_SendCommand(dev_handle, "/cmd/disarmacquisition");
    if (ret != CAEN_FELib_Success) 
    {
      printLastError();
      return EXIT_FAILURE;
    }
    ret = CAEN_FELib_SendCommand(dev_handle, "/cmd/cleardata");
    if (ret != CAEN_FELib_Success) 
    {
      printLastError();
      return EXIT_FAILURE;
    }



    // MARIA 2730
    //CAEN_DGTZ_FreeReadoutBuffer(&auxBuffer);
    //std::cout << "free event ... " << std::endl;
    //if (Event16)  CAEN_DGTZ_FreeEvent(VMEhandle, (void**)&Event16);
    freeEvent();

    // free circular buffer
    rb_delete(rb_handle);
    rb_handle=-1;
    ResetNumEventsInRB();


    //Reset all IRQs    
  }

  return SUCCESS;
}

//
//----------------------------------------------------------------------------
void * readThread(void * arg)
{
  int link = *(int*)arg;
  std::cout << "Started thread for link " << link << " out of " << NBCORES << " cores" << std::endl;

  //Lock each thread to a different cpu core
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET((link + 1), &mask);


  if( sched_setaffinity(getpid(), sizeof(mask), &mask) < 0 )
  {
    printf("ERROR setting cpu affinity for thread %d: %s\n", link, strerror(errno));
  }

  void *wp; // write pointer
  int status;
  int rb_level;

  while (runInProgress)
  {


    // Check if event in hardware to read
    // MARIA TODO IMPLEMENT stopRunInProgress to empty buffer
    if (!stopRunInProgress) //  && checkEvent())
    {
      /* If we've reached 75% of the ring buffer space, don't read
       * the next event.  Wait until the ring buffer level goes down.
       * It is better to let the v1725 buffer fill up instead of
       * the ring buffer, as this the v1725 will generate the HW busy to the DTM.
       * MARIA TODO
       */
      rb_get_buffer_level(rb_handle, &rb_level);
//if (verbose) std::cout << " buffer handle " << rb_handle << " level " << rb_level << std::endl;
      // VERBOSE
      //std::cout << " buffer level " << rb_level << std::endl;
      if(rb_level > (int)(rb_event_buffer_size*0.75))  // 0.75!!
      {
        // VERBOSE
        //std::cout << " buffer level " << rb_level << std::endl;
        continue;
      }

      // Ok to read data
      // VERBOSE
      //std::cout << " n events in circular buffer: " << GetNumEventsInRB() << std::endl;
      status = rb_get_wp(rb_handle, &wp, 100); // timeout in ms
      if (status == DB_TIMEOUT) 
      {
        cm_msg(MERROR,"readThread", "Got wp timeout for thread %d . wp is %d . number of events in rb: %d . Is the ring buffer full?", link , wp, GetNumEventsInRB());
        cm_msg(MERROR,"readThread", "Exiting thread %d with error", link);
        thread_retval = -1;
        pthread_exit((void*)&thread_retval);
      }

      // Read data and store in circular buffer
      if(!readEvent(wp)) 
      {
        cm_msg(MERROR,"readThread", "Readout routine error on thread %d", link);
        cm_msg(MERROR,"readThread", "Exiting thread %d with error", link);
        thread_retval = -1;
        pthread_exit((void*)&thread_retval);
      }
    } // end CheckEvent

    // TODO MARIA check
    // Sleep for 5us to avoid hammering the board too much
    //usleep(1);

  }

  std::cout << "Exiting thread " << link << " clean " << std::endl;
  thread_retval = 0;
  pthread_exit((void*)&thread_retval);
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
   bool evtReady = true;

  if (GetNumEventsInRB() == 0) evtReady=false;

  if (evtReady && !test) return 1;
  //usleep(20); // MARIA TEST CFC 080422
  //usleep(20); // MARIA TODO
  return 0;

}

// MARIA 2730
// this function is not used
/*
INT checkEvent()
{

  ///////// CHECK EVENT IS READY
   // MARIA DAQ ZGZ TEST APRIL22
  DWORD vmeStat;
  CAENComm_ErrorCode sCAEN = CAENComm_Read32(VMEhandle, V1725_VME_STATUS, &vmeStat);
  return (vmeStat & 0x1);

}
*/

// MARIA 2730
// The function ReadData from FeLib returns only one event per call
// Change the whole function
/*
INT readEvent(void * wp)
{
  CAEN_DGTZ_ErrorCode ret;

  // VERVOSE
  //if (verbose) std::cout << " reading event ... " << std::endl;
  
  CAEN_DGTZ_ReadData(VMEhandle, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, auxBuffer, &BufferSize);

  uint32_t NumEvents = 0;
  if (BufferSize != 0) 
  {
    ret = CAEN_DGTZ_GetNumEvents(VMEhandle, auxBuffer, BufferSize, &NumEvents);
    if (ret != CAEN_DGTZ_Success) 
    {
      cm_msg(MERROR,"ReadEvent", "Communication error: %d", ret);
      std::cout << " error getting  number of events" << std::endl;
      return (ret == CAEN_DGTZ_Success);
    }
  }
  else 
  {
    uint32_t lstatus;
    ret = CAEN_DGTZ_ReadRegister(VMEhandle, CAEN_DGTZ_ACQ_STATUS_ADD, &lstatus);
    if (ret != CAEN_DGTZ_Success) 
    {
      printf("Warning: Failure reading reg:%x (%d)\n", CAEN_DGTZ_ACQ_STATUS_ADD, ret);
      cm_msg(MERROR,"ReadEvent", "Communication error: %d", ret);
      return (ret == CAEN_DGTZ_Success);
    }
    else 
    {
      if (lstatus & (0x1 << 19)) 
      {
        std::cout << " ERR_OVERTEMP!!" << std::endl;
        cm_msg(MERROR,"ReadEvent", "ERR_OVERTEMP: %d", ret);
        return (ret == CAEN_DGTZ_Success);
      }
    }
  }

  ////////////////////////// HERE!
//VERBOSE 
//if (BufferSize>0) std::cout << " Buffer size " << BufferSize << " reserved " << auxBufferSize << " NEV: " << NumEvents << std::endl;
  int copied=0;
  WORD *pidata = (WORD*)wp; // MARIA treat wp as WORD array (short int)
  for(int iev = 0; iev < (int)NumEvents; iev++) 
  {

    uint16_t flags = 0;
    copied+=8*sizeof(WORD); // header

    ret = CAEN_DGTZ_GetEventInfo(VMEhandle, auxBuffer, BufferSize, iev, &EventInfo, &EventPtr);
    if (ret != CAEN_DGTZ_Success) 
    {
      std::cout << " error getting event info. Ev number " << iev << " of total events " << NumEvents << " buffer size: " << BufferSize  << " mybuffer size " << BufferSize << std::endl;
        cm_msg(MERROR,"ReadEvent", "error getting event info. Event number: %d, error: %d", iev, ret);
        return (ret == CAEN_DGTZ_Success);
    }
    ret = CAEN_DGTZ_DecodeEvent(VMEhandle, EventPtr, (void**)&Event16);

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
    prevTimeStamp = EventInfo.TriggerTimeTag;
    pidata+=4;

    // MARIA 2730
    //for (int ch = 0; ch < (int32_t)BoardInfo.Channels; ch++)
    for (int ch = 0; ch < (int32_t)runInfo.nChannels; ch++)
    {
      if (v1730_settings.ch_enable[ch])
      {
//VERBOSE
//std::cout << " ch " << ch << " chsize " << Event16->ChSize[ch] << " data[0]: " << Event16->DataChannel[ch][0] << std::endl;
        memcpy(pidata,Event16->DataChannel[ch], Event16->ChSize[ch]*sizeof(WORD));
        pidata += Event16->ChSize[ch]; 
        copied+=Event16->ChSize[ch]*sizeof(WORD);
      }
    }
  }

  if (copied==0) return (ret==CAEN_DGTZ_Success);
  /////////// increment circular buffer 
  rb_increment_wp(rb_handle, copied);

  IncrementNumEventsInRB(NumEvents); //atomic

  // VERBOSE
  //if (verbose) std::cout << " copied in cb " << copied << " bytes for " << NumEvents << " events. Events in buffer: " << GetNumEventsInRB() << std::endl;

  return (ret == CAEN_DGTZ_Success);

}
*/

INT readEvent(void * wp)
{
  int ret;

  // VERVOSE
  //if (verbose) std::cout << " reading event ... " << std::endl;
  // DEB
  //std::cout << " reading event .ep handle: " << ep_handle << std::endl;

  // TEST
  //ret = CAEN_FELib_SendCommand(dev_handle, "/cmd/sendswtrigger");
  //  if (ret != CAEN_FELib_Success) printLastError();

  
  // MARIA 2730
  // the array waveform must have dimensions MAX_CHANNELS.
  // the unactive channels are at 0. If channels 0 and 4 are active,
  // waveform will contain: data_0 0 0 0 data_4 ....
  ret = CAEN_FELib_ReadData(ep_handle,
            TIMEOUT_MS,
            &ptEvent->timestamp,
            &ptEvent->trigger_id,
            ptEvent->waveform,
            ptEvent->nSamples,
            &ptEvent->eventSize);

  if (ret==CAEN_FELib_Timeout) return 1; // there are no events, no error
  else if (ret==CAEN_FELib_Stop) 
  {
    // BOARD STOPPED, return error
    cm_msg(MERROR,"ReadEvent", "Communication error: %d", ret);
    return 0;
  }
  else if (ret==CAEN_FELib_Success)
  {
    // with the current ReadData function, NumEvents is always 1
    // when reading formated data.
    // keep the loop in events in case we decide to go to read raw data
    int NumEvents = 1; 
    int copied=0;
    WORD *pidata = (WORD*)wp; // MARIA treat wp as WORD array (short int)
    for(int iev = 0; iev < (int)NumEvents; iev++) 
    {
      uint16_t flags = 0;
      copied+=8*sizeof(WORD); // header
  
      // MARIA 2730
      // two words for channel mask

      // 1st word: channel mask
      //*pidata++=channel_mask; 
     
      // second word: flags ; 0 OK, 1-> error reading
      //*pidata++=flags; 
      *((uint32_t*)pidata) = channel_mask;
      pidata+=2;
      // MARIA done 2730
  
      // two words: samples per channel
      *((uint32_t*)pidata) = v2730_settings.recordlength;
      pidata+=2;
  
      // four words: time stamp
      // TODO aniadir check in clock.
      // 2730 -> clock in 48 bits (8 ns precission)-> 26 days
      // keep the software extension anyway
      if (ptEvent->timestamp<prevTimeStamp) counter++;
      uint64_t clock = (uint64_t)(counter<<47) + ptEvent->timestamp;
      clock *= CLOCK2NS; 
      *((uint64_t*)pidata) = clock;
      prevTimeStamp = ptEvent->timestamp;
      pidata+=4;
  
      for (int ch = 0; ch < (int32_t)runInfo.nChannels; ch++)
      {
        if (v2730_settings.ch_enable[ch])
        {
  //VERBOSE
  //std::cout << " ch " << ch << " chsize " << ptEvent->nSamples[ch] << " data[0]: " << ptEvent->waveform[ch][0] << std::endl;
          memcpy(pidata, ptEvent->waveform[ch], ptEvent->nSamples[ch]*sizeof(WORD)); 
          pidata += ptEvent->nSamples[ch]; 
          copied+=ptEvent->nSamples[ch]*sizeof(WORD);
        }
      }
    } // end loop in events
  
    /////////// increment circular buffer 
    rb_increment_wp(rb_handle, copied);
  
    IncrementNumEventsInRB(NumEvents); //atomic
  
    // VERBOSE
    //if (verbose) std::cout << " copied in cb " << copied << " bytes for " << NumEvents << " events. Events in buffer: " << GetNumEventsInRB() << std::endl;
  
    return 1;
  } // end CAEN_FELib_Success
  std::cout << " not recognized ret value: " << ret << std::endl; 
  return 0; // unknown error. Should never be here
  
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
//  read event from circular buffer
INT read_event_from_rb(char *pevent, INT off)
{
  // VERBOSE
  //std::cout << " read_from_rb: nEvInRB: " << GetNumEventsInRB() << std::endl;

  if (!runInProgress || GetNumEventsInRB()==0) return 0; // MARIA TODO tengo que hacer algo (matar thread) si numEv==0?


  WORD *dest; // MARIA short int

  WORD *src=NULL;

  int status = rb_get_rp(rb_handle, (void**)&src, 500); // 500 is a timeout in ms


  if (status == DB_TIMEOUT) 
  {
    cm_msg(MERROR,"read_event_from_rb", "Got rp timeout for module");
    printf("### num events: %d\n", GetNumEventsInRB());
    return false;
  }


  // MARIA TODO . check header to verify this is ok?

  // > THE LINE BELOW SHOULD BE HERE AND NOWHERE ELSE < !
  // Maria: initialie global bank header
  // OJO!!!!!
  // los bancos con 16 bits tienen tambien un tamanio maximo a 32KB
  // si quiero un banco ilimitado tengo que definirlo como de 32 bits 
  bk_init32(pevent);

  // create bank and copy data
  char bankName [5];
  sprintf(bankName, "WF%02d",VMEBUS_BOARDNO); 
  bk_create(pevent, bankName, TID_UINT16,  (void **)&dest); // reserva memeoria en dest 

  // copy data 
  int num = enabledChannels*v2730_settings.recordlength + 8; // header: mask(1)+flags(1)+lengh(2)+timestamp(4)
  memcpy(dest, src, num*sizeof(WORD));
  dest +=num;

  // VERBOSE!!!
  //if (verbose) std::cout << " channel mask : " << dest[0] << std::endl;
  //if (verbose) std::cout << " flags : " << dest[1] << std::endl;
  //if (verbose) std::cout << " pulse lenght : " << *(uint32_t*)&dest[2] << std::endl;
  //if (verbose) std::cout << " timestamp : " << *(uint64_t*)&dest[4] << std::endl;

  bk_close(pevent, dest);

  // VERBOSE!!
  //if (verbose) std::cout << " read from cb " << num*sizeof(WORD) << " bytes . Events in buffer: " << GetNumEventsInRB() <<  " size bank: " << bk_size(pevent) << std::endl;

  DecrementNumEventsInRB(); //atomic
  rb_increment_rp(rb_handle, num*sizeof(WORD)); // MARIA TODO is this number equal to memcpy??

  return bk_size(pevent); 

}

/*******************
 *******************
 *******************
 *******************
 * configure trigger
 ********************/
// MARIA 2730 
// do from scratch
/*
int configureTrigger()
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
  uint32_t majority = v2730_settings.NRequestForCoincidence - 1; 
  if (majority>7) majority = 7;
  uint32_t TVAW =  v2730_settings.coincidenceWindowNs/CLOCK2NS;
  // to be configured with 4 bits -> [0 - 15]
  if (TVAW>15) 
  {
    std::cout << " coincidence window requested " << v2730_settings.coincidenceWindowNs << " but maximum is " << 15*CLOCK2NS << " configuring " << 15*CLOCK2NS << std::endl;
    TVAW = 15;
  }


  std::cout << " configuring trigger logics............ " << std::endl; 
  // MARIA 2730
  //for(size_t i=0;i<BoardInfo.Channels/2;++i)
  for(size_t i=0;i<runInfo.nChannels/2;++i)
  {
    uint32_t valConf =0;
    uint32_t width =0;
    // 1. check if has to be included in request (check that it is different from NONE)
    if(strcmp(v2730_settings.ch2_logic[i],"NONE"))
    {
      // different from NONE! . If it is one of the allowed cases, activate bit in request
      // Bits[10] 00 AND
      //          01 ONLY0
      //          10 ONLY1
      //          11 OR
      if(!strcmp(v2730_settings.ch2_logic[i],"AND")) { valConf = 0; request |= (1<<i); }
      if(!strcmp(v2730_settings.ch2_logic[i],"ONLY0")) { valConf = 1; request |= (1<<i); }
      if(!strcmp(v2730_settings.ch2_logic[i],"ONLY1")) { valConf = 2; request |= (1<<i); }
      if(!strcmp(v2730_settings.ch2_logic[i],"OR")) { valConf = 3; request |= (1<<i); }
      // check programmable window
      //if (v1730_settings.triggerWidthNs[i]==0) valConf |=4; // rise bit 2
      //else width =  v1730_settings.triggerWidthNs[i]/CLOCK2NS;
      // MARIA 160222 TEST!!
      valConf |=4; // rise bit 2
      width =  v2730_settings.triggerWidthNs[i]/CLOCK2NS;
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
*/
int configureTrigger()
{
  char par_name[256];
  char value [256];
  int ret;
   
  // MARIA 2730
  // Previously this call was after setting the threshold
  // TODO check that it works here
  ret = setRelativeThreshold();

  int polarity = -1;
  if (!strcmp(&v2730_settings.pulsePolarity,"+")) polarity = +1;
  
  if (ret != CAEN_FELib_Success) { std::cout << " Error Cannot set relative threshold. " << std::endl; return ret;}

  
  // MAIN LOGIC -> MAJORITY
  ret = CAEN_FELib_SetValue(dev_handle, "/par/itlamainlogic","Majority");
  if (ret != CAEN_FELib_Success) std::cout << " Error Cannot set main logic to majority. Digitizer error code: " << ret << std::endl;

  // MAJORITY LEVEL: NRequestForCoincidence
  if (v2730_settings.NRequestForCoincidence<0) v2730_settings.NRequestForCoincidence=0;
  else if (v2730_settings.NRequestForCoincidence>32) v2730_settings.NRequestForCoincidence=32;
  snprintf(value, sizeof(value), "%u", v2730_settings.NRequestForCoincidence);
  ret = CAEN_FELib_SetValue(dev_handle, "/par/itlamajoritylev",value);
  if (ret != CAEN_FELib_Success) std::cout << " Error Cannot set majority level. Digitizer error code: " << ret << std::endl;

  // DO NOT USE PAIRS
  ret = CAEN_FELib_SetValue(dev_handle, "/par/itlapairlogic","NONE");
  if (ret != CAEN_FELib_Success) std::cout << " Error Cannot set pair logic to false. Digitizer error code: " << ret << std::endl;

  for(size_t i=0;i<runInfo.nChannels;++i)
  {
    if (v2730_settings.ch_enable[i] && v2730_settings.ch_threshold[i]>0)
    {
      // add to itla
      snprintf(par_name, sizeof(par_name), "/ch/%zu/par/itlconnect", (size_t)i);
      ret = CAEN_FELib_SetValue(dev_handle, par_name, "ITLA");
      if (ret != CAEN_FELib_Success) std::cout << " Error Cannot set ch " << i << " to ITLA Digitizer error code: " << ret << std::endl;

      // MARIA 2730
      // threshold is set in set_relative_threshold
      //int polarity = -1;
      //if (!strcmp(&v2730_settings.pulsePolarity,"+")) polarity = +1;
      //int threshold = v2730_settings.ch_threshold[i]*1000./runInfo.ADC2Volt[i]*polarity;
      //snprintf(value, sizeof(value), "%d", threshold);
      //snprintf(par_name, sizeof(par_name), "/ch/%zu/par/triggerthr", (size_t)i);
      //ret = CAEN_FELib_SetValue(dev_handle, par_name, value);
      //if (ret != CAEN_FELib_Success) std::cout << " Error Cannot set threshold for ch " << i << " to " << value << " . Digitizer error " << ret << std::endl;

      // Mode: relative to dc offset
      snprintf(par_name, sizeof(par_name), "/ch/%zu/par/triggerthrmode", (size_t)i);
      //ret = CAEN_FELib_SetValue(dev_handle, par_name, "Relative");
      ret = CAEN_FELib_SetValue(dev_handle, par_name, "Absolute");
      if (ret != CAEN_FELib_Success) std::cout << " Error Cannot set threshold for ch " << i << " relative. Digitizer error"  << ret << std::endl;
      // falling /rising
      snprintf(par_name, sizeof(par_name), "/ch/%zu/par/selftriggeredge", (size_t)i);
      if (polarity == 1) ret = CAEN_FELib_SetValue(dev_handle, par_name, "RISE");
      else ret = CAEN_FELib_SetValue(dev_handle, par_name, "FALL");
      if (ret != CAEN_FELib_Success) std::cout << " Error Cannot set threshold for ch " << i << " edge. Digitizer error"  << ret << std::endl;
      // widht  
      if (v2730_settings.triggerWidthNs[i]>524280) v2730_settings.triggerWidthNs[i]=524280;
      snprintf(value, sizeof(value), "%d", v2730_settings.triggerWidthNs[i]);
      snprintf(par_name, sizeof(par_name), "/ch/%zu/par/selftriggerwidth", (size_t)i);
      ret = CAEN_FELib_SetValue(dev_handle, par_name, value);
      if (ret != CAEN_FELib_Success) std::cout << " Error Cannot set trigger width for ch " << i << " to " << value << " . Digitizer error " << ret << std::endl;
    }
    else 
    {
      snprintf(par_name, sizeof(par_name), "/ch/%zu/par/itlconnect", (size_t)i);
      ret = CAEN_FELib_SetValue(dev_handle, par_name, "Disabled");
      if (ret != CAEN_FELib_Success) std::cout << " Error Cannot set ch " << i << " to Disabled. Digitizer error code: " << ret << std::endl;
    }
  }
  return ret;
}
// MARIA 2730
/*
int set_relative_Threshold()
{
	int ch = 0, i = 0;
	CAEN_DGTZ_ErrorCode ret;

	uint32_t custom_posttrg = 50;
	int baseline[MAX_CH] = { 0 }, size = 0;
	int rms[MAX_CH] = { 0 };
    int samples = 20; // TODO CONFIGURE IN DB
    int nEvents = 200; // TODO CONFIGURE IN DB


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

	  ret = CAEN_DGTZ_ReadData(VMEhandle, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, auxBuffer, &BufferSize);
	  if (ret || BufferSize==0) 
      {
        std::cout << " error reading data after software trigger " << std::endl;
        return -1;
	  }

	  ret = CAEN_DGTZ_GetEventInfo(VMEhandle, auxBuffer, BufferSize, 0, &EventInfo, &EventPtr); 
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

          // MARIA 2730
	  //for (ch = 0; ch < (int32_t)BoardInfo.Channels; ch++) 
	  for (ch = 0; ch < (int32_t)runInfo.nChannels; ch++) 
      {
        if (v2730_settings.ch_enable[ch]) 
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
	ret = CAEN_DGTZ_SetPostTriggerSize(VMEhandle, v2730_settings.posttrigger); 
	if (ret) 
    {
      std::cout << " error setting posttrigger size " << std::endl;
      return -1;
	}

	CAEN_DGTZ_ClearData(VMEhandle);


   // set the threshold
   std::cout << " baseline summary for " << enabledChannels << " channels " << std::endl;
   // MARIA 2730
   //for (ch = 0; ch < (int32_t)BoardInfo.Channels; ch++) 
   for (ch = 0; ch < (int32_t)runInfo.nChannels; ch++) 
   {
     if (v2730_settings.ch_enable[ch]) 
     {
        baseline[ch] /= nEvents;
        rms[ch] /= nEvents;
        int polarity = -1;
        if (!strcmp(&v2730_settings.pulsePolarity,"+")) polarity = +1;
    
        // MARIA TODO: convert from mV to ADC
        double thr = v2730_settings.ch_threshold[ch];
        thr=(DWORD)baseline[ch]  + polarity*thr;

        // checks
        if (thr<0) thr=0; 
	    //size = (int)pow(2, (double)BoardInfo.ADC_NBits);
	    size = (int)pow(2, (double)14);  // MARIA 2730 provisional, change to threshold in mV
        if (thr>(uint32_t)size) thr=size; 

        ret = CAEN_DGTZ_SetChannelTriggerThreshold(VMEhandle, ch, thr);
        if (ret) 
        {
          std::cout << " error setting threshold of channel  "<< ch << std::endl;
          return -1;
        }
        std::cout << " ch " << ch <<  " threshold " << v2730_settings.ch_threshold[ch];
        std::cout << " baseline " <<  baseline[ch] << " threshold set to " << thr << " rms: " << rms[ch] << std::endl;
     } // end loop in channels
   }//end sw trigger event analysis

   return 0;
}
*/

// MARIA 2730 new functions
/*******************
 *******************
 *******************
 *******************
 * getBoardInfo
 ********************/
int getBoardInfo()
{
  int ret;
  char value[256];
  ret = CAEN_FELib_GetValue(dev_handle, "/par/ModelName", value);
  if (ret != CAEN_FELib_Success) return ret;
  printf("Model name:\t%s\n", value);
    
  ret = CAEN_FELib_GetValue(dev_handle, "/par/SerialNum", value);
  if (ret != CAEN_FELib_Success) return ret;
  printf("Serial number:\t%s\n", value);
  
  ret = CAEN_FELib_GetValue(dev_handle, "/par/ADC_Nbit", value);
  if (ret != CAEN_FELib_Success) return ret;
  printf("ADC bits:\t%d\n", atoi(value));
  
  ret = CAEN_FELib_GetValue(dev_handle, "/par/cupver", value);
  if (ret != CAEN_FELib_Success) return ret;
  printf("CUP version:\t%s\n", value);

  ret = CAEN_FELib_GetValue(dev_handle, "/par/NumCh", value);
  if (ret != CAEN_FELib_Success) return ret;
  runInfo.nChannels= atoi(value);
  printf("Channels:\t%d\n", runInfo.nChannels);

  ret = CAEN_FELib_GetValue(dev_handle, "/par/ADC_SamplRate", value);
  if (ret != CAEN_FELib_Success) return ret;
  runInfo.samplingRate= atoi(value);
  printf("ADC rate:\t%d Msps\n", runInfo.samplingRate);

  return ret;
}

/*******************
 *******************
 *******************
 *******************
 * getBoardInfo
 ********************/
int configureEndPoint(uint64_t * ep_handle)
{
  int ret;
  // conigure endpoint
  ret = CAEN_FELib_GetHandle(dev_handle, "/endpoint/scope", ep_handle);
  
  uint64_t ep_folder_handle;
  ret = CAEN_FELib_GetParentHandle(*ep_handle, NULL, &ep_folder_handle);
  if (ret != CAEN_FELib_Success) return ret;
  
  ret = CAEN_FELib_SetValue(ep_folder_handle, "/par/activeendpoint", "scope");
  if (ret != CAEN_FELib_Success) return ret;

  ret = CAEN_FELib_SetReadDataFormat(*ep_handle, DATA_FORMAT);
  if (ret != CAEN_FELib_Success) return ret;
  return ret;
}

/*******************
 *******************
 *******************
 *******************
 * allocateEvent
 ********************/
int allocateEvent()
{
  ptEvent = (struct EventData*)malloc(sizeof(*ptEvent));
  if (ptEvent == NULL) 
  {
    fprintf(stderr, "malloc failed");
    frontend_exit();
    exit(1);
  }
  // MARIA 2730
  // even if there are a few active channels, it is necesary to 
  // reserve memory to all of them
  //ptEvent->nChannels = runInfo->nActiveChannels;
  ptEvent->nChannels = runInfo.nChannels;
  ptEvent->nAllocatedSamples = runInfo.nSamples;

  // nsamples to be read by ReadData, must be an array
  ptEvent->nSamples = (size_t*)malloc(ptEvent->nChannels * sizeof(*ptEvent->nSamples));
  if (ptEvent->nSamples == NULL) 
  {
    fprintf(stderr, "malloc failed");
    frontend_exit();
    exit(1);
  }

  // waveform
  ptEvent->waveform = (uint16_t**)malloc(ptEvent->nChannels * sizeof(*ptEvent->waveform));
  if (ptEvent->waveform == NULL) 
  {
    fprintf(stderr, "malloc failed");
    frontend_exit();
    exit(1);
  }
  for (size_t i = 0; i < ptEvent->nChannels; ++i) 
  {
    ptEvent->waveform[i] = (uint16_t*)malloc(ptEvent->nAllocatedSamples * sizeof(*ptEvent->waveform[i]));
    if (ptEvent->waveform[i] == NULL) 
    {
      fprintf(stderr, "malloc failed");
      frontend_exit();
      exit(1);
    }
  }
//std::cout << " allocated memory for " << ptEvent->nChannels << " channels of nsamples: " << ptEvent->nAllocatedSamples << std::endl;
std::cout << " allocated memory for " << ptEvent->nChannels << " channels of nsamples: " << runInfo.nSamples << std::endl;
  return SUCCESS;
}

/*******************
 *******************
 *******************
 *******************
 * freeEvent
 ********************/
void freeEvent()
{
  if (ptEvent==0) return;
  for (size_t i = 0; i < ptEvent->nChannels; ++i)
    free(ptEvent->waveform[i]);
  free(ptEvent->waveform);
  free(ptEvent->nSamples);
  free(ptEvent);
}
/*******************
 *******************
 *******************
 *******************
 * printLastError
 ********************/
int printLastError(void)
{
  char msg[1024];
  int ec = CAEN_FELib_GetLastError(msg);
  if (ec != CAEN_FELib_Success)
  {
    fprintf(stderr, "%s failed\n", __func__);
    return ec;
  }
  fprintf(stderr, "last error: %s\n", msg);
  return ec;
}


/*******************
 *******************
 *******************
 *******************
 * setRelativeThreshold
 ********************/
int setRelativeThreshold()
{
  char par_name[256];
  char value [256];
  double baseline[MAX_CH] = { 0 };
  double rms[MAX_CH] = { 0 };
  double bsl=0;
  double auxrms=0;
  int bslSamples = v2730_settings.recordlength/5.; // reduce probability of chance event
  int nEvents = 200; // TODO CONFIGURE IN DB
  int ret;
  int polarity = -1;
  if (!strcmp(&v2730_settings.pulsePolarity,"+")) polarity = +1;
 
  // SET SOFTWARE TRIGGER
  printf("Set software trigger...\t");
  ret = CAEN_FELib_SetValue(dev_handle, "/par/AcqTriggerSource", "SwTrg");
  if (ret != CAEN_FELib_Success)
  {
    printLastError();
    return EXIT_FAILURE;
  }

  // START DAQ
  std::cout << " ARMING DAQ FOR BSL READOUT " << std::endl;
  ret = CAEN_FELib_SendCommand(dev_handle, "/cmd/armacquisition");
  if (ret != CAEN_FELib_Success)
  {
    printLastError();
    return EXIT_FAILURE;
  }
  ret = CAEN_FELib_SendCommand(dev_handle, "/cmd/swstartacquisition");
  if (ret != CAEN_FELib_Success)
  {
    printLastError();
    return EXIT_FAILURE;
  }
  usleep(300000);
  printf("Starting bsl measurement...\t");

  for (int id=0; id<nEvents; id++) // loop in number of events
  {
    //std::cout << " sending trigger " << id << std::endl;
    ret = CAEN_FELib_SendCommand(dev_handle, "/cmd/sendswtrigger");
    if (ret != CAEN_FELib_Success) printLastError();
  
    // read event
    ret = CAEN_FELib_ReadData(ep_handle, 
            TIMEOUT_MS, 
            &ptEvent->timestamp, 
            &ptEvent->trigger_id, 
            ptEvent->waveform, 
            ptEvent->nSamples,
            &ptEvent->eventSize);

    switch (ret) 
    {
      case CAEN_FELib_Success: 
// DEB
//for (int ich = 0; ich < runInfo.nChannels; ich++) std::cout <<  evt->waveform[ich][0] << " " ;
//std::cout << std::endl;

        for (int ich = 0; ich < runInfo.nActiveChannels; ich++)
        {
          int ch=runInfo.activeChannels[ich];
          bsl=0;
          auxrms=0;

          // calculate bsl
          for (int i = 0; i < bslSamples; i++)
          {
            bsl += ptEvent->waveform[ch][i];
            auxrms += ptEvent->waveform[ch][i]*ptEvent->waveform[ch][i];
          }
          auxrms/=bslSamples;
          auxrms-=bsl*bsl/bslSamples/bslSamples;
          auxrms=sqrt(auxrms);

          baseline[ch] += bsl / bslSamples;
          rms[ch] += auxrms;
        } 
        break;
      case CAEN_FELib_Timeout:
        printf("timeout\n");
        break;
      case CAEN_FELib_Stop:
	printf("\nStop received.\n");
	return 0;
      default:
	printLastError();
	break;
    } // end switch 
  } // end loop in n_events

  ret = CAEN_FELib_SendCommand(dev_handle, "/cmd/disarmacquisition");
  if (ret != CAEN_FELib_Success) 
  {
    printLastError();
    return EXIT_FAILURE;
  }
  printf("stop bsl measurement. clear data in digitizer ...\t");
  ret = CAEN_FELib_SendCommand(dev_handle, "/cmd/cleardata");
  if (ret != CAEN_FELib_Success)
  {
    printLastError();
    return EXIT_FAILURE;
  }

  // SET THRESHOLD
  for (int ich = 0; ich < runInfo.nActiveChannels; ich++)
  {
    int ch=runInfo.activeChannels[ich];
    baseline[ch]/=nEvents; 
    rms[ch]/=nEvents;
    float threshold = (float) baseline[ch] + v2730_settings.ch_threshold[ch]/1000./runInfo.ADC2Volt[ch]*polarity;
    snprintf(value, sizeof(value), "%d", (int)threshold);
    snprintf(par_name, sizeof(par_name), "/ch/%zu/par/triggerthr", (size_t)ch);
    ret = CAEN_FELib_SetValue(dev_handle, par_name, value);
    if (ret != CAEN_FELib_Success) std::cout << " Error Cannot set threshold for ch " << ch << " to " << value << " . Digitizer error " << ret << std::endl;
    std::cout << " channel " << ch << " baseline: " << baseline[ch] << " rms: " << rms[ch] << " threshold in mv " << v2730_settings.ch_threshold[ch] << " threshold in ADC counts : " << (int)threshold << " ADC2Volt: " << runInfo.ADC2Volt[ch] << std::endl;
  }

  return ret;
} 

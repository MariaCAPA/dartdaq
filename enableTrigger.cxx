/******************************************************************

  Name:         enableTrigger.c
  Created by:   Maria Martinez

  Contents:    Write 1 in GPO to enable Trigger
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
#include "CAENDigitizer.h"
#include "CAENComm.h"
#include "v1725Raw.h"


typedef unsigned char BYTE;
typedef unsigned short int UINT16;
typedef short int INT16;
typedef unsigned int UINT32;
typedef int INT32;
//typedef long int INT64;


// VME base address 
DWORD V1730_BASE =   0; // 0x32100000; // 0-> optical link in module
DWORD VMEBUS_BOARDNO = 0;
DWORD LINK = 0; // MARIA, ERA 1
int VMEhandle=-1;



int main()
{
//-- Globals -------------------------------------------------------

  ////////////////////////
  // Open VME interface, init link board_number
  int ret = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_OpticalLink, LINK, VMEBUS_BOARDNO,V1730_BASE, &VMEhandle);
  int status;
  uint32_t request = 0;

  //ret = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_USB, 0, 0,0, &VMEhandle);
  if (ret != CAEN_DGTZ_Success) 
  { 
    std::cout << " Error opening Digitizer. Digitizer error code: " << ret << std::endl;
    exit(1);
  }

  // MARIA write 1 in GPO register
  // To start synchornized DAQ with ANAIS
  // -> write 0 en 0x8110 and write 0xc000 in 0x811c
  request = 0;
  status = CAEN_DGTZ_WriteRegister(VMEhandle, 0x8110, request);
  if (status != CAEN_DGTZ_Success) 
  {  
    std::cout << " Failure writing GPO register (0x8110) to " << request << ". Digitizer error code: " << status << std::endl; 
    exit(1);
  }
  // 0x811C
  // write 1 in bit [14] (force trg-out to 1)
  // write 1 in bit [15] (test logic level)
  // -> 0xC000
  request = 0xC000;

  status = CAEN_DGTZ_WriteRegister(VMEhandle, V1725_FP_IO_CONTROL, request); // 0x811C
  if (status != CAEN_DGTZ_Success) 
  {  
    std::cout << " Failure writing GPO register (0x811C) to " << request << ". Digitizer error code: " << status << std::endl; 
    exit(1);
  }
  ret = CAEN_DGTZ_ReadRegister(VMEhandle, V1725_FP_IO_CONTROL, &request);
  if (ret != CAEN_DGTZ_Success) 
  {  
    std::cout << " Failure reading FP_IO register (0x811C). Digitizer error code: " << ret << std::endl; 
    exit(1);
  }
  std::cout << " FP_IO CONTROL REGISTER (0x811C) : " << request << std::endl;
}

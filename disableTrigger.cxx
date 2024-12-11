/******************************************************************

  Name:         disableTrigger.c
  Created by:   Maria Martinez

  Contents:    Write 0 in GPO to enable Trigger
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

#include <CAEN_FELib.h>


//std::string devicePath = "dig2://caendgtz-usb-51553";
std::string devicePath = "dig2://caendgtz-usb-52037";


int main()
{
  char msg[1024];
  uint64_t dev_handle; // DEVICE HANDLE
  int  ret = CAEN_FELib_Open(devicePath.c_str(), &dev_handle);
  if (ret != CAEN_FELib_Success)
  {
    CAEN_FELib_GetLastError(msg);
    std::cout << " Error opening Digitizer. Digitizer error code: " << msg << std::endl;
    return EXIT_FAILURE;
  }

  ret = CAEN_FELib_SetValue(dev_handle, "/par/gpiomode", "Fixed0");
  if (ret != CAEN_FELib_Success) 
  {
    std::cout << " Error Cannot set 0 to GPIO. Trigger will not be disabled "<< std::endl;
    CAEN_FELib_GetLastError(msg);
    std::cout << "Digitizer error: " << msg << std::endl;
  }
  else  std::cout << " TRIGGER DISABLED (0 in GPIO)" << std::endl;
}

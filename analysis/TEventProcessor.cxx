#include "TEventProcessor.hxx"
#include "TV1730RawData.hxx"
#include "math.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>

using namespace std;
const int VMEBUS_BOARDNO = 0;

// MARIA 070622
#include "midas.h"
#include "msystem.h"
HNDLE hDB;


TEventProcessor* TEventProcessor::s_instance = 0;
std::vector<double> TEventProcessor::tini;

TEventProcessor* TEventProcessor::instance() 
{
  if (!s_instance)
  {
    s_instance = new TEventProcessor();

    // MARIA 070622
    int status = cm_get_experiment_database(&hDB, NULL);
    if (status != CM_SUCCESS) 
    {
      hDB =0;
    }

  }


  return s_instance;
}

void TEventProcessor::Reset() 
{
}

int TEventProcessor::ProcessMidasEvent(TDataContainer& dataContainer) 
{
  fAEvent->Reset();

  fAEvent->run=fRun;
  fAEvent->eventNumber= dataContainer.GetMidasEvent().GetSerialNumber();
  fAEvent->midasEventNumber= dataContainer.GetMidasEvent().GetSerialNumber();
  fAEvent->bankNumber= 0;
  fAEvent->time = dataContainer.GetMidasEvent().GetTimeStamp();

  char name[100];
  sprintf(name, "WF%02d", VMEBUS_BOARDNO); // Check for module-specific data
  TV1730RawData *V1730 = dataContainer.GetEventData<TV1730RawData>(name);
  // if there are no channels, return 
  if (!V1730 || V1730->GetNChannels()==0) 
  {
    printf("Didn't see bank %s or there are no channels \n", name);
    return 0;
  }
  int retval = ProcessMidasEvent(V1730);
  return retval;
}

int TEventProcessor::ProcessMidasEvent(TV1730RawData * V1730) 
{
  //static ULong64_t prevTime = 0;
  static double prevTime = 0;

  fAEvent->timeNs = V1730->GetHeader().timeStampNs;
  fAEvent->eventCounter = V1730->GetHeader().eventCounter;
  fAEvent->trigger_mask = V1730->GetHeader().trigger_mask;
  int nCh =  V1730->GetNChannels();

  // loop in channels
  float bsl[16]; // MARIA 070622
  float rms[16]; // MARIA 070622
  for (int i=0; i<nCh; i++) 
  {
    // MARIA 070622
    bsl[i]=0;
    rms[i]=0;
    int theCh = V1730->GetChannelData(i).GetChannelNumber();
    AnalyzeAChannel(V1730->GetChannelData(i));
    bsl[i]=fAEvent->channel[i].bsl;
    rms[i]=fAEvent->channel[i].rms;
  }
  // MARIA 070622 TODO !! write in DB to read it in history
  if (hDB) 
  {
    db_set_value(hDB, 0, "/Equipment/V1730_Data00/Variables/bsl/bsl", bsl, 16*sizeof(float), 16, TID_FLOAT);
    db_set_value(hDB, 0, "/Equipment/V1730_Data00/Variables/rms/rms", rms, 16*sizeof(float), 16, TID_FLOAT);
  }

  fAEvent->mult = fAEvent->channel.size();
  double area=0;
  for (unsigned int ch=0; ch<fAEvent->channel.size(); ch++) area += fAEvent->channel[ch].area;
  fAEvent->areaS=area;
  fAEvent->dt = fAEvent->timeNs-prevTime;
  prevTime = fAEvent->timeNs;
  
  return 1;
}

int TEventProcessor::AnalyzeAChannel(TV1730RawChannel& channelData)
{
  TAEvent::TACh dch;
  //GetBasicParam(channelData, dch.bsl, dch.bmax, dch.bmaxp, dch.bmin, dch.bminp, dch.bimax, dch.rms, dch.max, dch.t0, dch.tMax, dch.charge, dch.min, dch.tMin);
  GetBasicParam(channelData, dch.area, dch.bsl, dch.rms, dch.max, dch.min, dch.t0, dch.tMax, dch.tMin);
  dch.ch=channelData.GetChannelNumber();

  fAEvent->channel.push_back(dch);
  return 1;
}

int TEventProcessor::GetBasicParam(TV1730RawChannel& channelData, double & area, double & bsl, double & rms, double & max, double & min, double & t0, double & tMax, double & tMin)
{
  int nSamples = channelData.GetNSamples();
  ////////////////////
  // TODO CONFIGURE FROM DB
  const int nBslSamples = 100;
  const double polarity = -1.; 
  const int nRMS = 5;
  //////////////////

  area=0;
  bsl=0;
  rms=0;
  max=0;
  min=0; 
  t0=0;
  tMax=0;
  tMin=0;
   
  //baseline and rms calculation at the beginning of the wf
  int samp=0;
  for (; samp < nBslSamples; samp++) 
  { 
      double adc = channelData.GetADCSample(samp);
      bsl += adc;
      rms += adc*adc;
  }
  bsl /= (float)nBslSamples;
  rms /= (float)nBslSamples;
  rms -= bsl*bsl;
  rms = sqrt(rms);


  samp=0;

  // look for trigger time
  double threshold = bsl + polarity*nRMS*rms;
  //double threshold_n = bsl - polarity*nRMS*rms;
  //double threshold = bsl + 40;

  while (samp<nSamples) 
  {
    if (polarity*channelData.GetADCSample(samp) > polarity*threshold) break;
    samp++;
  }

  if (samp==nSamples) return 0; // no trigger

  //t0 FOUND!
  t0=samp;
  
  // look for maximum and maximum time
  max= polarity*(channelData.GetADCSample(samp)  - bsl);
  min=max;
  tMax=samp;
  tMin=samp;
  //min= polarity*(channelData.GetADCSample(samp)  - bsl);
  //tMin=samp;
  
  //look for max and pos max
  for (; samp < nSamples ; samp++) 
  { 
    double signal = channelData.GetADCSample(samp);
    double test = polarity*(signal  - bsl);
    if (test>max) {max=test; tMax = samp;}
    if (test<min) {min=test; tMin = samp;}

    area += test;
  }


  return 1;
}

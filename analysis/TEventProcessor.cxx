#include "TEventProcessor.hxx"
#include "TV1730RawData.hxx"
#include "math.h"
#include <algorithm>
#include <iostream>

using namespace std;
const int VMEBUS_BOARDNO = 0;

TEventProcessor* TEventProcessor::s_instance = 0;

TEventProcessor* TEventProcessor::instance() 
{
  if (!s_instance)
    s_instance = new TEventProcessor();
  return s_instance;
}

void TEventProcessor::Reset() 
{
}

int TEventProcessor::ProcessMidasEvent(TDataContainer& dataContainer) 
{
  static int prevTime = 0;
  fDartEvent->Reset();

  fDartEvent->run=fRun;
  fDartEvent->eventNumber= dataContainer.GetMidasEvent().GetSerialNumber();
  fDartEvent->time = dataContainer.GetMidasEvent().GetTimeStamp();

  char name[100];
  sprintf(name, "WF%02d", VMEBUS_BOARDNO); // Check for module-specific data
  TV1730RawData *V1730 = dataContainer.GetEventData<TV1730RawData>(name);

  fDartEvent->timeNs = V1730->GetHeader().timeStampNs;

  // if there are no channels, return 
  if (!V1730 || V1730->GetNChannels()==0) 
  {
    printf("Didn't see bank %s or there are no channels \n", name);
    return 0;
  }

  int nCh =  V1730->GetNChannels();

  // loop in channels
  for (int i=0; i<nCh; i++) 
  {
    int theCh = V1730->GetChannelData(i).GetChannelNumber();
    if (IsDart(theCh)) AnalyzeDartChannel(V1730->GetChannelData(i));
    else AnalyzeVetoChannel(V1730->GetChannelData(i));
  }

  fDartEvent->vetoMult = fDartEvent->vetoChannel.size();
  double area=0;
  for (unsigned int ch=0; ch<fDartEvent->dartChannel.size(); ch++) area += fDartEvent->dartChannel[ch].charge;
  fDartEvent->totCharge=area;
  for (unsigned int ch=0; ch<fDartEvent->vetoChannel.size(); ch++) area += fDartEvent->vetoChannel[ch].Vcharge;
  fDartEvent->vetoCharge=area;
  fDartEvent->dt = fDartEvent->timeNs-prevTime;
  prevTime = fDartEvent->timeNs;
  
  return 1;
}
bool TEventProcessor::IsDart(int ch)
{
  // TODO READ FROM DB
  // BY NOW, DART CHANNELS ARE 0 & 1
  if (ch==0 || ch==1) return true;
  return false;
}
int TEventProcessor::AnalyzeDartChannel(TV1730RawChannel& channelData)
{
  TDartEvent::TDartCh dch;
  GetBasicParam(channelData,dch.bsl, dch.rms, dch.high, dch.t0, dch.tMax, dch.charge);
  dch.ch=channelData.GetChannelNumber();
  fDartEvent->dartChannel.push_back(dch);
  return 1;
}

int TEventProcessor::AnalyzeVetoChannel(TV1730RawChannel& channelData)
{
  TDartEvent::TVetoCh vch;
  vch.Vch=channelData.GetChannelNumber();
  GetBasicParam(channelData,vch.Vbsl, vch.Vrms, vch.Vhigh, vch.Vt0, vch.VtMax, vch.Vcharge);
  if (vch.Vt0>0) fDartEvent->vetoChannel.push_back(vch); // only it pass software threshold
  return 1;
}

int TEventProcessor::GetBasicParam(TV1730RawChannel& channelData, double &bsl, double &rms, double &high, double & t0, double &tMax, double &area)
{
  int nSamples = channelData.GetNSamples();
  ////////////////////
  // TODO CONFIGURE FROM DB
  const int nBslSamples = 200;
  const int polarity = -1; 
  const int nRMS = 5;
  //const int thr = 8; // in ADC samples
  //////////////////

  bsl=0;
  rms=0;
  high=0;
  t0=0;
  tMax=0;
  area=0;
   
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

  // look for trigger
  int test = bsl + polarity*nRMS*rms;
  while (samp<nSamples) 
  {
    if (polarity*channelData.GetADCSample(samp) > polarity*test) break;
    samp++;
  }

  if (samp==nSamples) return 0; // no trigger
  t0=samp;
  // integrate from t0
  for (; samp < nSamples ; samp++) 
  {
    test = polarity*(channelData.GetADCSample(samp)  - bsl);
    if (test>high) {high=test; tMax = samp;}
    area += test;
  }

  return 1;
}

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
  GetBasicParam(channelData,dch.bsl, dch.rms, dch.high, dch.t0, dch.tMax, dch.charge, dch.min, dch.tMin);
  dch.ch=channelData.GetChannelNumber();

  int nSamples = channelData.GetNSamples();
  ////////////////////
  // TODO CONFIGURE FROM DB
  const int nBslSamples = 200;
  const double polarity = 1.; 
  const double SR=0.5; // GS/s
  //////////////////
  double bslEnd=0;
  double rmsEnd=0;
  double area90=0;
  double area640=0;
   
  int samp=1;
  for (; samp <= nBslSamples; samp++) 
  {
      double adc = channelData.GetADCSample(nSamples-samp);
      bslEnd += adc;
      rmsEnd += adc*adc;
  }
  bslEnd /= (float)nBslSamples;
  rmsEnd /= (float)nBslSamples;
  rmsEnd -= bslEnd*bslEnd;
  rmsEnd = sqrt(rmsEnd);

  // integrate from t0
  samp=dch.t0;
  // 90 ns => 90*SR points
  int max90 = samp + 90*SR;
  double test;
  for (; samp < max90 ; samp++) 
  {
    test = polarity*(channelData.GetADCSample(samp) - dch.bsl);
    area90 += test;
    area640 += test;
  }
  int max640 = samp + (640-90)*SR;
  for (; samp < max640 ; samp++) 
  {
    test = polarity*(channelData.GetADCSample(samp) - dch.bsl);
    area640 += test;
  }

  dch.bslEnd = bslEnd;
  dch.rmsEnd = rmsEnd;
  dch.charge90 = area90;
  dch.charge640 = area640;

//std::cout << " area90 " << area90 << " area640 " << area640 << std::endl;


  fDartEvent->dartChannel.push_back(dch);
  return 1;
}

int TEventProcessor::AnalyzeVetoChannel(TV1730RawChannel& channelData)
{
  TDartEvent::TVetoCh vch;
  vch.Vch=channelData.GetChannelNumber();
  GetBasicParam(channelData,vch.Vbsl, vch.Vrms, vch.Vhigh, vch.Vt0, vch.VtMax, vch.Vcharge, vch.Vmin, vch.VtMin);
  if (vch.Vt0>0) fDartEvent->vetoChannel.push_back(vch); // only when it pass software threshold
  return 1;
}

int TEventProcessor::GetBasicParam(TV1730RawChannel& channelData, double &bsl, double &rms, double &high, double & t0, double &tMax, double &area, double &min, double &tMin)
{
  int nSamples = channelData.GetNSamples();
  ////////////////////
  // TODO CONFIGURE FROM DB
  const int nBslSamples = 200;
  const double polarity = 1.; 
  const int nRMS = 5;
  //const int thr = 8; // in ADC samples
  //////////////////

  bsl=0;
  rms=0;
  high=0;
  t0=0;
  tMax=0;
  area=0;
  min=0; 
  tMin=0;
   
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
  double test = bsl + polarity*nRMS*rms;
  while (samp<nSamples) 
  {
    if (polarity*channelData.GetADCSample(samp) > polarity*test) break;
    samp++;
  }

  if (samp==nSamples) return 0; // no trigger
  t0=samp;
  // integrate from t0
  high= polarity*(channelData.GetADCSample(samp)  - bsl);
  tMax=samp;
  min= polarity*(channelData.GetADCSample(samp)  - bsl);
  tMin=samp;
  for (; samp < nSamples ; samp++) 
  {
    test = polarity*(channelData.GetADCSample(samp)  - bsl);
    if (test>high) {high=test; tMax = samp;}
    if (test<min) {min=test; tMin = samp;}
    area += test;
  }

  return 1;
}

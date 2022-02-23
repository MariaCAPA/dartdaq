#include "TEventProcessor.hxx"
#include "TV1730RawData.hxx"
#include "math.h"
#include <algorithm>
#include <iostream>
#include <fstream>

using namespace std;
const int VMEBUS_BOARDNO = 0;

TEventProcessor* TEventProcessor::s_instance = 0;
std::vector<double> TEventProcessor::tini;

TEventProcessor* TEventProcessor::instance() 
{
  if (!s_instance)
  {
    s_instance = new TEventProcessor();

  }


  return s_instance;
}

void TEventProcessor::Reset() 
{
}

int TEventProcessor::ProcessMidasEvent(TDataContainer& dataContainer) 
{
  static ULong64_t prevTime = 0;
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
  GetBasicParam(channelData, dch.bsl, dch.bmax, dch.bmaxp, dch.bmin, dch.bminp, dch.bimax, dch.rms, dch.max, dch.t0, dch.tMax, dch.charge, dch.min, dch.tMin);
  dch.ch=channelData.GetChannelNumber();

  int nSamples = channelData.GetNSamples();
  ////////////////////
  // TODO CONFIGURE FROM DB
  const int nBslSamples = 2000;
  const double polarity = 1.; 
  const double SR=0.5; // GS/s
  //////////////////
  double bslEnd=0;
  double rmsEnd=0;
  double area90=0;
  double area640=0;
 
  double bsl=0; 
  double rms=0; 
  double nRMS=5; 

  // LUDO: average 100 first events for channel i in 
  //TEventProcessor::tini[i]

  //Calculation of baseline and rms at the beginning of the wf
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

  // thresholds for the integral calculation
  double threshold = bsl + polarity*nRMS*rms;
  //double threshold_n = bsl - polarity*nRMS*rms;
  //double threshold = bsl + 40;

  //Calculation of baseline and rms at the end of the wf  
  /*samp=1;
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
*/

  // integrate from tMax - n_pre
  samp=dch.t0;
  //samp=dch.t0-100;
  //samp=dch.tMax-100;
  int n_pre = 10;
  //samp=dch.tMax - n_pre;

  int max640 = samp + 640*SR;
  //int max640 = samp + 640*SR+100;
  double test;

  for (; samp < max640 ; samp++) 
  {
    if (samp >= 6e3 ) break;
    double signal = channelData.GetADCSample(samp);
    //if (signal > threshold) {
    test = polarity*(signal - bsl);
    //} else if (signal < threshold_n) {
        //test = polarity*(signal - threshold_n);
    //} else { continue; }

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
  GetBasicParam(channelData,vch.Vbsl, vch.Vbmax, vch.Vbmaxp, vch.Vbmin, vch.Vbminp, vch.Vbimax, vch.Vrms, vch.Vmax, vch.Vt0, vch.VtMax, vch.Vcharge, vch.Vmin, vch.VtMin);
  if (vch.Vt0>0) fDartEvent->vetoChannel.push_back(vch); // only when it pass software threshold
  return 1;
}

int TEventProcessor::GetBasicParam(TV1730RawChannel& channelData, double &bsl, double &bmax, double &bmaxp, double &bmin, double &bminp, double &bimax, double &rms, double &max, double & t0, double &tMax, double &area, double &min, double &tMin)
{
  int nSamples = channelData.GetNSamples();
  ////////////////////
  // TODO CONFIGURE FROM DB
  const int nBslSamples = 2000;
  const double polarity = 1.; 
  const int nRMS = 5;
  //const int thr = 8; // in ADC samples
  //////////////////

  bsl=0;
  bmax=0;
  bmin=0;
  bmaxp=0;
  bminp=0;
  bimax=0;
  rms=0;
  max=0;
  t0=0;
  tMax=0;
  area=0;
  min=0; 
  tMin=0;
   
  //baseline and rms calculation at the beginning of the wf
  int samp=0;
  for (; samp < nBslSamples; samp++) 
  { 
      double adc = channelData.GetADCSample(samp);
      if (adc > bmax) {bmax = adc ; bmaxp = samp ;}
      bsl += adc;
      rms += adc*adc;
  }
  bsl /= (float)nBslSamples;
  rms /= (float)nBslSamples;
  rms -= bsl*bsl;
  rms = sqrt(rms);
  bmax=bmax-bsl;

//////////////////////////////// Baseline maximum and minimum calculation ///////////////////////////

samp=0;
/*bmax=0;
for (; samp < nBslSamples; samp++)
  {
      double adc = channelData.GetADCSample(samp) - bsl;
      if (adc > bmax) {bmax = adc ; bmaxp = samp ;}
      if (adc < bmin) {bmin = adc ; bminp = samp ;}
  }

  //Calculation of integral around maximum of the baseline (1 sample before and 2 after)
  for (int samp=0; samp < nBslSamples; samp++)
  {
  	  if ( samp == bmaxp ) {
	   double adc_1 = channelData.GetADCSample(samp-1);
	   double adc_2 = channelData.GetADCSample(samp);
       double adc_3 = channelData.GetADCSample(samp+1);
       double adc_4 = channelData.GetADCSample(samp+2);

		bimax += (adc_1 + adc_2 + adc_3 + adc_4 - 4.*bsl);
      }
  } 
*/

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
  //t0=2876;

  
  // look for maximum and maximum time
  max= polarity*(channelData.GetADCSample(samp)  - bsl);
  tMax=samp;
  //min= polarity*(channelData.GetADCSample(samp)  - bsl);
  //tMin=samp;
  
  //look for max and pos max
  for (; samp < nSamples ; samp++) 
  { 
    double signal = channelData.GetADCSample(samp);
    //if(signal > threshold) { 
    double test = polarity*(signal  - bsl);
    if (test>max) {max=test; tMax = samp;}
       //if (test<min) {min=test; tMin = samp;}
    //} else { continue; }

    //area += TEST;
  }

//Charge integration
  double TEST;
  int n_pre = 10;
  //samp=tMax - n_pre;
  samp = t0;
  //samp = tMax-100;
  //int roi_to = 3500 + 100;
  for (; samp < nSamples ; samp++)
  //for (; samp < roi_to ; samp++)
  {
    double signal = channelData.GetADCSample(samp);
    //if(signal > threshold) {
    TEST = polarity*(signal  - bsl);
    //} else if (signal < threshold_n) { 
    //TEST = polarity*(signal  - threshold_n);
    //} else { continue; }

    area += TEST;
  }

  return 1;
}

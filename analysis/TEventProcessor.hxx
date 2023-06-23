#ifndef TEventProcessor_h
#define TEventProcessor_h

#include <vector>
#include "TDataContainer.hxx"
#include "TAEvent.hxx"
#include "TV1730RawData.hxx"

class TEventProcessor
{
public:
  
  static TEventProcessor *instance();

  // Delete hits from last event.
  void Reset();

  TAEvent * GetAEvent(){return fAEvent;}
  int GetRun() {return fRun;}
 
  int ProcessMidasEvent(TDataContainer& dataContainer);
  int ProcessMidasEvent(TV1730RawData * V1730);
  void SetRun(int run) {fRun=run;} 

  static void tini_push_back(double val) {tini.push_back(val);}

  int GetBasicParam(TV1730RawChannel& channelData, double & area, double & bsl, double & rms, double & max, double & min, double & t0, double & tMax, double & tMin);


private:
  
  // pointer to global object
  static TEventProcessor  *s_instance;
  static std::vector<double> tini; // ini time (average max pos first 100 events) for every channel


  int AnalyzeAChannel(TV1730RawChannel& channelData);

  TAEvent *fAEvent;
  int fRun;
  // ...

  // hidden private constructor
  TEventProcessor() {fAEvent=new TAEvent();}
};

#endif


#ifndef TEventProcessor_h
#define TEventProcessor_h

#include <vector>
#include "TDataContainer.hxx"
#include "TDartEvent.hxx"
#include "TV1730RawData.hxx"

class TEventProcessor
{
public:
  
  static TEventProcessor *instance();

  // Delete hits from last event.
  void Reset();

  TDartEvent * GetDartEvent(){return fDartEvent;}
 
  int ProcessMidasEvent(TDataContainer& dataContainer);
  void SetRun(int run) {fRun=run;} 

private:
  
  // pointer to global object
  static TEventProcessor  *s_instance;


  bool IsDart(int ch);
  int AnalyzeDartChannel(TV1730RawChannel& channelData);
  int AnalyzeVetoChannel(TV1730RawChannel& channelData);
  int GetBasicParam(TV1730RawChannel& channelData, double &bsl, double &rms, double &high, double & t0, double &tMax, double &area);

  TDartEvent *fDartEvent;
  int fRun;

  // hidden private constructor
  TEventProcessor() {fDartEvent=new TDartEvent();}
};

#endif


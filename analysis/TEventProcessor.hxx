#ifndef TEventProcessor_h
#define TEventProcessor_h

#include <vector>
#include "TDataContainer.hxx"
#include "TDartEvent.hxx"
#include "TV2730RawData.hxx"

class TEventProcessor
{
public:
  
  static TEventProcessor *instance();

  // Delete hits from last event.
  void Reset();

  TDartEvent * GetDartEvent(){return fDartEvent;}
  int GetRun() {return fRun;}
 
  int ProcessMidasEvent(TDataContainer& dataContainer);
  int ProcessMidasEvent(TV2730RawData * V2730);
  void SetRun(int run) {fRun=run;} 

  static void tini_push_back(double val) {tini.push_back(val);}

  int GetBasicParam(TV2730RawChannel& channelData, double &bsl, double &bmax, double &bmaxp, double &bmin, double &bminp, double &bimax, double &rms, double &max, double & t0, double &tMax, double &area, double &min, double &tMin);

private:
  
  // pointer to global object
  static TEventProcessor  *s_instance;
  static std::vector<double> tini; // ini time (average max pos first 100 events) for every channel


  bool IsDart(int ch);
  int AnalyzeDartChannel(TV2730RawChannel& channelData);
  int AnalyzeVetoChannel(TV2730RawChannel& channelData);

  TDartEvent *fDartEvent;
  int fRun;
  // ...

  // hidden private constructor
  TEventProcessor() {fDartEvent=new TDartEvent();}
};

#endif


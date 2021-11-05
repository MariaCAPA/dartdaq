#ifndef __DART_UTILS__H__
#define __DART_UTILS__H__

#include <string>
#include <vector>

class TChain;
class TDartEvent;
class TV1730Waveform;
class TMReaderInterface;

class TDartReadRun
{
public:
  TMReaderInterface* fReader;
  std::string fRootBaseName;
  std::string fDataBaseName;
  int fRun;
  TChain * fTree;
  TDartEvent * fCurrentEv;
  std::vector<int> fSelectedEvents;

  TDartReadRun(int run, std::string rootBasename="output", std::string dataBaseName="/storage/online/run");
  virtual ~TDartReadRun() {};
  TV1730Waveform *GetWaveform(int evNo);
  TV1730Waveform *GetWaveformFromSelectedEvents(int index);
  int SetSelection (std::string);


};


#endif


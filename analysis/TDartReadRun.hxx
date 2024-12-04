#ifndef __DART_UTILS__H__
#define __DART_UTILS__H__

#include <string>
#include <vector>

class TChain;
class TDartEvent;
class TV2730Waveform;
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
  TV2730Waveform *GetWaveform(int evNo, bool draw=true, bool dump=true);
  // Maria 240322 this auxiliar function contains the original code, 
  // when the midas event contain only one waveform per channel
  TV2730Waveform *GetWaveformAuxiliar(int evNo, bool draw=true, bool dump=true);
  TV2730Waveform *GetWaveformFromSelectedEvents(int index, bool draw=true, bool dump=true);
  TV2730Waveform *GetAverageFromSelectedEvents(bool draw=true, bool dump=false);
  int SetSelection (std::string);


};


#endif


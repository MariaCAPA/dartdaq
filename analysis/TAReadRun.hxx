#ifndef __A_UTILS__H__
#define __A_UTILS__H__

#include <string>
#include <vector>

class TChain;
class TAEvent;
class TV1730Waveform;
class TMReaderInterface;

class TAReadRun
{
public:
  TMReaderInterface* fReader;
  std::string fRootBaseName;
  std::string fDataBaseName;
  int fRun;
  TChain * fTree;
  TAEvent * fCurrentEv;
  std::vector<int> fSelectedEvents;

  TAReadRun(int run, std::string rootBasename="output", std::string dataBaseName="/storage/online/run");
  virtual ~TAReadRun() {};
  TV1730Waveform *GetWaveform(int evNo, bool draw=true, bool dump=true);
  // Maria 240322 this auxiliar function contains the original code, 
  // when the midas event contain only one waveform per channel
  TV1730Waveform *GetWaveformAuxiliar(int evNo, bool draw=true, bool dump=true);
  TV1730Waveform *GetWaveformFromSelectedEvents(int index, bool draw=true, bool dump=true);
  TV1730Waveform *GetAverageFromSelectedEvents(bool draw=true, bool dump=false);
  int SetSelection (std::string);


};


#endif


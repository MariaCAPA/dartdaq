#ifndef __DART_VISU__H__
#define __DART_VISU__H__

#include <string>
#include <vector>
#include <TRootanaDisplay.hxx>
class TChain;
class TH1;
class TV2730Waveform;
class TDartEvent;

//TChain * readDartRun(int run, std::string baseName="output");
//TV2730Waveform *GetWaveform(std::string fname, int evNo);
class TV2730WaveformM;
class TDataContainer;
class TV2730RawData;

class TDartVisu: public TRootanaDisplay {
public:
  // An analysis manager.  Define and fill histograms in
  // analysis manager.
  TV2730Waveform* fWf;
  TDartEvent * fCurrentEv;

  TDartVisu();
  void AddAllCanvases() ;
  virtual ~TDartVisu() {};
  void ResetHistograms(){}
  void UpdateHistograms(TDataContainer& dataContainer);
  void UpdateHistograms(TV2730RawData * V2730);
  void PlotCanvas();
  void PlotCanvas(TV2730Waveform * wf);

  // Maria 150222
  TCanvas * GetMainCanvas();
  TCanvas * fMainCanvas;

};

#endif


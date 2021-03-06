#ifndef __DART_VISU__H__
#define __DART_VISU__H__

#include <string>
#include <vector>
#include <TRootanaDisplay.hxx>
class TChain;
class TH1;
class TV1730Waveform;
class TDartEvent;

//TChain * readDartRun(int run, std::string baseName="output");
//TV1730Waveform *GetWaveform(std::string fname, int evNo);
class TV1730WaveformM;
class TDataContainer;
class TV1730RawData;

class TDartVisu: public TRootanaDisplay {
public:
  // An analysis manager.  Define and fill histograms in
  // analysis manager.
  TV1730Waveform* fWf;
  TDartEvent * fCurrentEv;

  TDartVisu();
  void AddAllCanvases() ;
  virtual ~TDartVisu() {};
  void ResetHistograms(){}
  void UpdateHistograms(TDataContainer& dataContainer);
  void UpdateHistograms(TV1730RawData * V1730);
  void PlotCanvas();
  void PlotCanvas(TV1730Waveform * wf);

  // Maria 150222
  TCanvas * GetMainCanvas();
  TCanvas * fMainCanvas;

};

#endif


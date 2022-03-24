#ifndef TV1730Waveform_hxx
#define TV1730Waveform_hxx

#include <string>
#include "TFancyHistogramCanvas.hxx"
#include "THistogramArrayBase.h"
#include "TEventProcessor.hxx"

/// Class for making histograms of raw V1730 waveforms;
/// right now is only for raw data.
class TV1730Waveform : public THistogramArrayBase {
public:
  TV1730Waveform();
  TV1730Waveform(std::string name);
  virtual ~TV1730Waveform(){};

  void Initialize();
  void UpdateHistograms(TDataContainer& dataContainer) override;
  void UpdateHistograms(TV1730RawData * V1730);
  void AddHistogramsChannel(TDataContainer &dataContainer, int ch);
  void AddWaveform(TV1730Waveform* wf);
  void NormalizeWaveform(TV1730Waveform* wf, int Nev);

  /// Getters/setters
  int GetNsecsPerSample() { return nanosecsPerSample; }
  void SetNanosecsPerSample(int nsecsPerSample) { this->nanosecsPerSample = nsecsPerSample; }
 
  int GetNChannels() {return fNChannels;}
  int GetChannelNumber(int index) {if ((unsigned int)index<fChannels.size()) return fChannels[index]; return -1;}
  int GetNSamples() {return fNSamples;}

  // Reset the histograms; needed before we re-fill each histo.
  void Reset();

  TCanvasHandleBase* CreateCanvas() override 
  {
    fCanvas = new TFancyHistogramCanvas(this, GetSubTabName(),2,true); // numberchannelsIngroups =2, disable AutoUpdate
    return fCanvas;
  }

  void CreateHistograms() override;
  void DeleteHistograms();


private:
  int nanosecsPerSample;
  int fNSamples;
  int fNChannels;
  std::vector<int> fChannels;
  TFancyHistogramCanvas* fCanvas;
  std::string fName;
};


#endif



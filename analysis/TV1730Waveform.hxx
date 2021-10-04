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
  virtual ~TV1730Waveform(){};

  void UpdateHistograms(TDataContainer& dataContainer) override;

  /// Getters/setters
  int GetNsecsPerSample() { return nanosecsPerSample; }
  void SetNanosecsPerSample(int nsecsPerSample) { this->nanosecsPerSample = nsecsPerSample; }

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
};


#endif



#ifndef TDartAnaManager_h
#define TDartAnaManager_h

// Use this list here to decide which type of equipment to use.

#include "THistogramArrayBase.h"
#include "TV2730Waveform.hxx"
#include "mvodb.h"

// manager class; holds different set of his
class TDartAnaManager  {
public:
  TDartAnaManager();
  virtual ~TDartAnaManager(){};

  /// Processes the midas event, fills histograms, etc.
  int ProcessMidasEvent(TDataContainer& dataContainer);

  /// Update those histograms that only need to be updated when we are plotting.
  void UpdateForPlotting();

  void Initialize();

  bool CheckOption(std::string option);

  void BeginRun(int transition,int run,int time, MVOdb* odb);
  void EndRun(int transition,int run,int time) {};

  // Add a THistogramArrayBase object to the list
  void AddHistogram(THistogramArrayBase* histo);

  // Little trick; we only fill the transient histograms here (not cumulative), since we don't want
  // to fill histograms for events that we are not displaying.
  // It is pretty slow to fill histograms for each event.
  void UpdateTransientPlots(TDataContainer& dataContainer);

  // Get the histograms
  std::vector<THistogramArrayBase*> GetHistograms() {
    return fHistos;
  }

private:

  std::vector<THistogramArrayBase*> fHistos;

  // Special histo that needs to be told how many boards there are at BOR
  TV2730Waveform* fVXhisto;

};



#endif



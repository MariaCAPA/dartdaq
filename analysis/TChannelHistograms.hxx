#ifndef TChannelHistograms_h
#define TChannelHistograms_h

#include <string>
#include "THistogramArrayBase.h"
#include "TSimpleHistogramCanvas.hxx"
//////////////////// CHARGE OF EVERY CHANNEL
class THistoCharges : public THistogramArrayBase {
public:
  THistoCharges();

  void UpdateHistograms(TDataContainer& dataContainer);  // update histograms

  void CreateHistograms(); // create histograms

  TCanvasHandleBase* CreateCanvas();
  
};

//////////////////// CHARGE OF EVERY CHANNEL
class THistoHigh : public THistogramArrayBase {
public:
  THistoHigh();

  void UpdateHistograms(TDataContainer& dataContainer);  // update histograms

  void CreateHistograms(); // create histograms

  TCanvasHandleBase* CreateCanvas();
  
};

////////////////////  SUM CHARGE DART & VETO
class THistoChargeSummary : public THistogramArrayBase {
public:
  THistoChargeSummary();

  void UpdateHistograms(TDataContainer& dataContainer);  // update histograms

  void CreateHistograms(); // create histograms

  TCanvasHandleBase* CreateCanvas();
};

///////////////// BSL VS TIME
/*
class THistoBslVsTime : public THistogramArrayBase {
public:
  THistoBslVsTime();

  void UpdateHistograms(TDataContainer& dataContainer); // update histograms

  void CreateHistograms(); // create histograms
  
  // Create a simple canvas out of this, since we only expect one.
  TCanvasHandleBase* CreateCanvas();
  
};
*/



#endif



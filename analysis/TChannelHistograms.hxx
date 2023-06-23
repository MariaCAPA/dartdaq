#ifndef TChannelHistograms_h
#define TChannelHistograms_h

#include <string>
#include "THistogramArrayBase.h"
#include "TSimpleHistogramCanvas.hxx"
//////////////////// CHARGE OF EVERY CHANNEL
class THistoArea : public THistogramArrayBase {
public:
  THistoArea();

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

////////////////////  SUM CHARGE 
class THistoAreaSummary : public THistogramArrayBase {
public:
  THistoAreaSummary();

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



#include <stdio.h>
#include <iostream>

#include "TRootanaDisplay.hxx"
#include "TFancyHistogramCanvas.hxx"
#include "TTree.h"

#include "TDartAnaManager.hxx"

class V2730Loop: public TRootanaDisplay {

public:

  TTree * fTree;
  // An analysis manager.  Define and fill histograms in
  // analysis manager.
  TDartAnaManager *anaManager;

  V2730Loop() {
    SetOutputFilename("v2730wfdisplay");
    DisableRootOutput(true);
    anaManager = new TDartAnaManager();
  }

  void AddAllCanvases() {
    // Set up tabbed canvases

    // Set up all the listed canvases from the AnaManager list of THistogramArrayBases
    std::vector<THistogramArrayBase*> histos = anaManager->GetHistograms();

    for (unsigned int i = 0; i < histos.size(); i++) {
      TCanvasHandleBase* canvas = histos[i]->CreateCanvas();

      if (canvas) {
        AddSingleCanvas(canvas, histos[i]->GetTabName());
      }
    }

    SetDisplayName("V2730 Display");
  }

  virtual ~V2730Loop() {};

  void BeginRun(int transition,int run,int time) 
  {
    anaManager->BeginRun(transition, run, time, GetODB());
    fTree = new TTree("td",Form("MIDAS data run %d",run));
    TDartEvent * dev = TEventProcessor::instance()->GetDartEvent();
    fTree->Branch("DartEvent",dev);

  }

  void EndRun(int transition,int run,int time) {
    anaManager->EndRun(transition, run, time);
  }

  void ResetHistograms(){}

  void UpdateHistograms(TDataContainer& dataContainer){
    // Update the cumulative histograms here
    anaManager->ProcessMidasEvent(dataContainer);
    fTree->Fill();
  }

  void PlotCanvas(TDataContainer& dataContainer){
    // Update the transient (per-event) histograms here.
    // saves CPU to not update them always when not being used.
    anaManager->UpdateTransientPlots(dataContainer);
  }
};


int main(int argc, char *argv[])
{
  V2730Loop::CreateSingleton<V2730Loop>();
  return V2730Loop::Get().ExecuteLoop(argc, argv);
}


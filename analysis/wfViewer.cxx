#include <stdio.h>
#include <iostream>

#include "TRootanaDisplay.hxx"
#include "TFancyHistogramCanvas.hxx"
#include "TTree.h"

#include "TAAnaManager.hxx"

class V1730Loop: public TRootanaDisplay {

public:

  TTree * fTree;
  // An analysis manager.  Define and fill histograms in
  // analysis manager.
  TAAnaManager *anaManager;

  V1730Loop() {
    SetOutputFilename("v1730wfdisplay");
    DisableRootOutput(true);
    anaManager = new TAAnaManager();
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

    SetDisplayName("V1730 Display");
  }

  virtual ~V1730Loop() {};

  void BeginRun(int transition,int run,int time) 
  {
    anaManager->BeginRun(transition, run, time, GetODB());
    fTree = new TTree("td",Form("MIDAS data run %d",run));
    TAEvent * dev = TEventProcessor::instance()->GetAEvent();
    fTree->Branch("AEvent",dev);

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
  V1730Loop::CreateSingleton<V1730Loop>();
  return V1730Loop::Get().ExecuteLoop(argc, argv);
}


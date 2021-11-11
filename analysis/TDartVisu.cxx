#include "TDartVisu.hxx"
#include "TV1730Waveform.hxx"
#include <iostream>
#include "midasio.h"


#include <TChain.h>
#include <TMidasEvent.h>
#include <TDataContainer.hxx>
#include <THistogramArrayBase.h>
#include <TRootanaDisplay.hxx>

#include <stdio.h>

#include "TFancyHistogramCanvas.hxx"
#include "TTree.h"
#include "TLine.h"

TDartVisu::TDartVisu()
{
  fWf = new TV1730Waveform();
}

void TDartVisu::AddAllCanvases()
{
  AddSingleCanvas("WF");
}

void TDartVisu::UpdateHistograms(TDataContainer& dataContainer)
{
  fWf->UpdateHistograms(dataContainer);
}

void TDartVisu::PlotCanvas(TDataContainer& dataContainer)
{
  if(GetDisplayWindow()->GetCurrentTabName().compare("WF") == 0)
  {
    TCanvas* c1 = GetDisplayWindow()->GetCanvas("WF");
    c1->Clear();
    c1->Divide(1,2);
    for (int i=0; i<2; i++)
    {
      c1->cd(i+1);
      fWf->GetHistogram(i)->Draw();
      // t0
      double t0 = fWf->GetHistogram(i)->GetBinCenter(fCurrentEv->dartChannel[i].t0);
      double tMax = fWf->GetHistogram(i)->GetBinCenter(fCurrentEv->dartChannel[i].tMax);
      double tMin = fWf->GetHistogram(i)->GetBinCenter(fCurrentEv->dartChannel[i].tMin);
      TLine * l=new TLine(t0, fCurrentEv->dartChannel[i].bsl+fCurrentEv->dartChannel[i].min, t0, fCurrentEv->dartChannel[i].bsl+fCurrentEv->dartChannel[i].high);
      l->SetLineColor(kGreen); l->SetLineWidth(3); l->Draw();
      // tmax
      l=new TLine(tMax, fCurrentEv->dartChannel[i].bsl+fCurrentEv->dartChannel[i].min, tMax, fCurrentEv->dartChannel[i].bsl+fCurrentEv->dartChannel[i].high);
      l->SetLineColor(kRed); l->Draw();
      // tmin
      l=new TLine(tMin, fCurrentEv->dartChannel[i].bsl+fCurrentEv->dartChannel[i].min, tMin, fCurrentEv->dartChannel[i].bsl+fCurrentEv->dartChannel[i].high);
      l->SetLineColor(kYellow); l->Draw();
      // bsl
      l=new TLine(fWf->GetHistogram(i)->GetBinCenter(1), fCurrentEv->dartChannel[i].bsl, fWf->GetHistogram(i)->GetBinCenter(fWf->GetHistogram(i)->GetNbinsX()-1), fCurrentEv->dartChannel[i].bsl);
      l->SetLineColor(kCyan); l->Draw();
      // threshold
      l=new TLine(fWf->GetHistogram(i)->GetBinCenter(1), fCurrentEv->dartChannel[i].bsl+5*fCurrentEv->dartChannel[i].rms, fWf->GetHistogram(i)->GetBinCenter(fWf->GetHistogram(i)->GetNbinsX()-1), fCurrentEv->dartChannel[i].bsl+5*fCurrentEv->dartChannel[i].rms);
      l->SetLineColor(kMagenta); l->Draw();
    }
    c1->Modified();
    c1->Update();
  }
}

void TDartVisu::PlotCanvas(TV1730Waveform * wf)
{
  if(GetDisplayWindow()->GetCurrentTabName().compare("WF") == 0)
  {
    TCanvas* c1 = GetDisplayWindow()->GetCanvas("WF");
    c1->Clear();
    c1->Divide(1,2);
    for (int i=0; i<2; i++)
    {
      c1->cd(i+1);
      wf->GetHistogram(i)->Draw();
    }
    c1->Modified();
    c1->Update();
  }
}

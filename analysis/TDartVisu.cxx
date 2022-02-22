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
  // Maria 150222
  fMainCanvas = new TCanvas("mainDartVisu","Waveform viewer");

}

TCanvas * TDartVisu::GetMainCanvas() 
{
  if (fMainCanvas) return fMainCanvas;
  fMainCanvas = new TCanvas("mainDartVisu","Waveform viewer");
  return fMainCanvas;
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
    int trigger = 669*2; //ns
    int offset = 100*2; //ns
    int roi = 3500*2; //ns
    int s = 20; //ADCc

    //TCanvas* c1 = GetDisplayWindow()->GetCanvas("WF"); // MARIA 150222
    TCanvas* c1 = GetMainCanvas();
    c1->Clear();
    c1->Divide(1,2);
    for (int i=0; i<2; i++)
    {
      c1->cd(i+1);
      fWf->GetHistogram(i)->Draw();

      //double max = fWf->GetHistogram(i)->GetBinCenter(fCurrentEv->dartChannel[i].max);
      //double rms = fWf->GetHistogram(i)->GetBinCenter(fCurrentEv->dartChannel[i].rms);

      // bsl
      TLine * l=new TLine(fWf->GetHistogram(i)->GetBinCenter(1), fCurrentEv->dartChannel[i].bsl, fWf->GetHistogram(i)->GetBinCenter(fWf->GetHistogram(i)->GetNbinsX()-1), fCurrentEv->dartChannel[i].bsl);
      l->SetLineColor(kRed);
      //l->SetLineWidth(3);
      l->Draw();

      l=new TLine(fWf->GetHistogram(i)->GetBinCenter(1), fCurrentEv->dartChannel[i].bsl, trigger - 400, fCurrentEv->dartChannel[i].bsl);
      l->SetLineColor(kGreen);
      l->SetLineWidth(3);
      //l->Draw();

      l=new TLine(trigger - offset, fCurrentEv->dartChannel[i].bsl, fWf->GetHistogram(i)->GetBinCenter(fWf->GetHistogram(i)->GetNbinsX()-1), fCurrentEv->dartChannel[i].bsl);
      l->SetLineColor(kYellow); 
      l->SetLineWidth(3);
      //l->Draw();

      //trigger time
      l=new TLine(trigger, fCurrentEv->dartChannel[i].bsl - s, trigger, fCurrentEv->dartChannel[i].bsl + s);
      l->SetLineColor(kMagenta); 
      l->SetLineWidth(3);
      //l->Draw();

      //limit sx ROI 
      l=new TLine(trigger - offset, fCurrentEv->dartChannel[i].bsl - s, trigger - offset, fCurrentEv->dartChannel[i].bsl + s);
      l->SetLineColor(kYellow); 
      l->SetLineWidth(3);
      //l->Draw();

      //limit dx ROI 
      l=new TLine(roi + offset, fCurrentEv->dartChannel[i].bsl - s, roi + offset, fCurrentEv->dartChannel[i].bsl + s);
      l->SetLineColor(kYellow); 
      l->SetLineWidth(3);
      //l->Draw();

      //limit sx baseline 
      l=new TLine(trigger-400, fCurrentEv->dartChannel[i].bsl - s, trigger-400, fCurrentEv->dartChannel[i].bsl + s);
      l->SetLineColor(kGreen); 
      l->SetLineWidth(3);
      //l->Draw();

      //limit dx baseline 
      l=new TLine(1, fCurrentEv->dartChannel[i].bsl - s, 1, fCurrentEv->dartChannel[i].bsl - s);
      l->SetLineColor(kGreen);
      l->SetLineWidth(3);
      //l->Draw();

      // t0
      double t0 = fWf->GetHistogram(i)->GetBinCenter(fCurrentEv->dartChannel[i].t0);
      double tMax = fWf->GetHistogram(i)->GetBinCenter(fCurrentEv->dartChannel[i].tMax);
      double tMin = fWf->GetHistogram(i)->GetBinCenter(fCurrentEv->dartChannel[i].tMin);
      l=new TLine(t0, fCurrentEv->dartChannel[i].bsl+fCurrentEv->dartChannel[i].min, t0, fCurrentEv->dartChannel[i].bsl+fCurrentEv->dartChannel[i].max);
      l->SetLineColor(kGreen); l->SetLineWidth(3); l->Draw();

      // tmax
      l=new TLine(tMax, fCurrentEv->dartChannel[i].bsl+fCurrentEv->dartChannel[i].min, tMax, fCurrentEv->dartChannel[i].bsl+fCurrentEv->dartChannel[i].max);
      l->SetLineColor(kMagenta); l->Draw();

      // tmin
      l=new TLine(tMin, fCurrentEv->dartChannel[i].bsl+fCurrentEv->dartChannel[i].min, tMin, fCurrentEv->dartChannel[i].bsl+fCurrentEv->dartChannel[i].max);
      l->SetLineColor(kYellow); //l->Draw();


      // l_sx
      l=new TLine(tMax, fCurrentEv->dartChannel[i].bsl+fCurrentEv->dartChannel[i].min, tMax-5, fCurrentEv->dartChannel[i].bsl+fCurrentEv->dartChannel[i].max);
      l->SetLineColor(kMagenta); //l->Draw();

       //threshold +
      l=new TLine(fWf->GetHistogram(i)->GetBinCenter(1), fCurrentEv->dartChannel[i].bsl+5*fCurrentEv->dartChannel[i].rms, fWf->GetHistogram(i)->GetBinCenter(fWf->GetHistogram(i)->GetNbinsX()-1), fCurrentEv->dartChannel[i].bsl+5*fCurrentEv->dartChannel[i].rms);
      l->SetLineColor(kGreen); 
      //l->Draw();

      l=new TLine(fWf->GetHistogram(i)->GetBinCenter(1), fCurrentEv->dartChannel[i].bsl+40, fWf->GetHistogram(i)->GetBinCenter(fWf->GetHistogram(i)->GetNbinsX()-1), fCurrentEv->dartChannel[i].bsl+40);
      l->SetLineColor(kMagenta); 
      //l->Draw();
     
  /*    // threshold -
      l=new TLine(fWf->GetHistogram(i)->GetBinCenter(1), fCurrentEv->dartChannel[i].bsl-5*fCurrentEv->dartChannel[i].rms, fWf->GetHistogram(i)->GetBinCenter(fWf->GetHistogram(i)->GetNbinsX()-1), fCurrentEv->dartChannel[i].bsl-5*fCurrentEv->dartChannel[i].rms);
      l->SetLineColor(kMagenta); //l->Draw();*/

      // MARIA PROVISIONAL 150222
      l=new TLine (5750,500,5750,2000); l->SetLineColor(kMagenta); //l->Draw();
      l=new TLine(fWf->GetHistogram(i)->GetBinCenter(1), fCurrentEv->dartChannel[i].bsl + 40, fWf->GetHistogram(i)->GetBinCenter(fWf->GetHistogram(i)->GetNbinsX()-1), fCurrentEv->dartChannel[i].bsl + 40);
      l->SetLineColor(kCyan);//l->Draw();
    }
    c1->Modified();
    c1->Update();
  }
}

void TDartVisu::PlotCanvas(TV1730Waveform * wf)
{
  if(GetDisplayWindow()->GetCurrentTabName().compare("WF") == 0)
  {
    //TCanvas* c1 = GetDisplayWindow()->GetCanvas("WF"); // MARIA 150222
    TCanvas* c1 = GetMainCanvas();
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


#include "TChannelHistograms.hxx"
#include "TCanvasHandleBase.hxx"
#include "TFancyHistogramCanvas.hxx"

#include "TDirectory.h"
#include "TH2D.h"
#include "TEventProcessor.hxx"
#include "TROOT.h"


const int numberChannelPerModule = 32;

/////////////////////// HISTO CHARGES

THistoArea::THistoArea(){

  SetTabName("Area");
  SetSubTabName("Channel Area ");
  SetNumberChannelsInGroup(numberChannelPerModule);
  CreateHistograms();

}

void THistoArea::CreateHistograms()
{

  // check if we already have histogramss.
  if(size() != 0){
    char tname[100];
    sprintf(tname,"HistoArea_0");
    
    TH1D *tmp = (TH1D*)gDirectory->Get(tname);
    if (tmp) return;
  }

  // Otherwise make histograms
  clear();

  for(int i = 0; i < numberChannelPerModule; i++)
  { // loop over 32 channels
      
      char name[100];
      char title[100];
      sprintf(name,"HistoArea_%i",i);
      TH1D *otmp = (TH1D*)gDirectory->Get(name);
      if (otmp) delete otmp;      
 
      sprintf(title,"Histo Area channel=%i",i);
      
      TH1D *tmp = new TH1D(name, title, 1000, 0,3e6);
      tmp->SetXTitle("Area (ADC*sample value)");
      tmp->SetYTitle("Counts");
      
      push_back(tmp);
  }
}
void THistoArea::UpdateHistograms(TDataContainer& dataContainer)
{
  TAEvent * dev = TEventProcessor::instance()->GetAEvent();

  for(unsigned int i = 0; i < dev->channel.size(); i++)
  {
    GetHistogram(dev->channel[i].ch)->Fill(dev->channel[i].area);
    if (gROOT)
    {
        TCanvas * aux = (TCanvas*)(gROOT->FindObject(Form("Canvas_1_0_%d",i+1)));
        if (aux) aux->SetLogy(1);
    }
  }
}

THistoHigh::THistoHigh(){

  SetTabName("High");
  SetSubTabName("Channel High ");
  SetNumberChannelsInGroup(numberChannelPerModule);
  CreateHistograms();

}
void THistoHigh::CreateHistograms()
{

  // check if we already have histogramss.
  if(size() != 0){
    char tname[100];
    sprintf(tname,"HistoHigh_0");
    
    TH1D *tmp = (TH1D*)gDirectory->Get(tname);
    if (tmp) return;
  }

  // Otherwise make histograms
  clear();

  for(int i = 0; i < numberChannelPerModule; i++)
  { // loop over 32 channels
      
      char name[100];
      char title[100];
      sprintf(name,"HistoHigh_%i",i);
      TH1D *otmp = (TH1D*)gDirectory->Get(name);
      if (otmp) delete otmp;      
 
      sprintf(title,"Histo High channel=%i",i);
      
      TH1D *tmp = new TH1D(name, title, 1000, 0,16400);
      tmp->SetXTitle("High (ADC)");
      tmp->SetYTitle("Counts");
      
      push_back(tmp);
  }
}
void THistoHigh::UpdateHistograms(TDataContainer& dataContainer)
{
  TAEvent * dev = TEventProcessor::instance()->GetAEvent();

  for(unsigned int i = 0; i < dev->channel.size(); i++)
  {
    GetHistogram(dev->channel[i].ch)->Fill(dev->channel[i].max);
  }
}

TCanvasHandleBase* THistoArea::CreateCanvas()
{
//  return new TSimpleHistogramCanvas(GetHistogram(0),"Area Channel", "COLZ");
   TFancyHistogramCanvas * canvas = new TFancyHistogramCanvas(this, GetSubTabName(),2,true); // 2 groups, disable auto updte
    canvas->SetGroupName("Group");
    canvas->SetChannelName("Channel");
    //canvas->HandleChangedNumberOfHistos();
    return canvas;

}
TCanvasHandleBase* THistoHigh::CreateCanvas()
{
//  return new TSimpleHistogramCanvas(GetHistogram(0),"High Channel", "COLZ");
   TFancyHistogramCanvas * canvas = new TFancyHistogramCanvas(this, GetSubTabName(),2,true); // 2 groups, disable auto updte
    canvas->SetGroupName("Group");
    canvas->SetChannelName("Channel");
    //canvas->HandleChangedNumberOfHistos();

    return canvas;

}

///////////////////////////////////////////////////
///////////////////////////////////////////////////

THistoAreaSummary::THistoAreaSummary()
{

  SetTabName("Area");
  SetSubTabName("Area Summary");
  CreateHistograms();
}

void THistoAreaSummary::CreateHistograms()
{
  char name[100];
  sprintf(name,"AreaSummary");

  // check if we already have histogramss.
  if(size() != 0){

    TH1D *tmp = (TH1D*)gDirectory->Get(name);
    if (tmp) return;
  }

  // Otherwise make histograms
  clear();

  char title[100];
  sprintf(title," Area Summary");
  TH1D *tmp = new TH1D(name, title, 5000, 0,500000);
  tmp->SetXTitle("Area (ADC*sample value)");
  tmp->SetYTitle("Counts");
  push_back(tmp);

}

void THistoAreaSummary::UpdateHistograms(TDataContainer& dataContainer)
{
  TAEvent * dev = TEventProcessor::instance()->GetAEvent();

  GetHistogram(0)->Fill(dev->areaS);
    if (gROOT)
    {
      TCanvas * aux = (TCanvas*)(gROOT->FindObject("Canvas_1_0"));
      if (aux) aux->SetLogy(1);
    }

}

TCanvasHandleBase* THistoAreaSummary::CreateCanvas()
{
  return new TSimpleHistogramCanvas(GetHistogram(0),"Area Summary", "COLZ");
}



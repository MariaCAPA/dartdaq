#include "TChannelHistograms.hxx"
#include "TCanvasHandleBase.hxx"
#include "TFancyHistogramCanvas.hxx"

#include "TDirectory.h"
#include "TH2D.h"
#include "TEventProcessor.hxx"


const int numberChannelPerModule = 16;

/////////////////////// HISTO CHARGES

THistoCharges::THistoCharges(){

  SetTabName("Charge");
  SetSubTabName("Channel Charge ");
  SetNumberChannelsInGroup(numberChannelPerModule);
  CreateHistograms();

}

void THistoCharges::CreateHistograms()
{

  // check if we already have histogramss.
  if(size() != 0){
    char tname[100];
    sprintf(tname,"HistoCharge_0");
    
    TH1D *tmp = (TH1D*)gDirectory->Get(tname);
    if (tmp) return;
  }

  // Otherwise make histograms
  clear();

  for(int i = 0; i < numberChannelPerModule; i++)
  { // loop over 16 channels
      
      char name[100];
      char title[100];
      sprintf(name,"HistoCharge_%i",i);
      TH1D *otmp = (TH1D*)gDirectory->Get(name);
      if (otmp) delete otmp;      
 
      sprintf(title,"Histo Charge channel=%i",i);
      
      TH1D *tmp = new TH1D(name, title, 5000, 0,500000);
      tmp->SetXTitle("Charge (ADC*sample value)");
      tmp->SetYTitle("Counts");
      
      push_back(tmp);
  }
}
void THistoCharges::UpdateHistograms(TDataContainer& dataContainer)
{
  TDartEvent * dev = TEventProcessor::instance()->GetDartEvent();
  //TEventProcessor::instance()->ProcessMidasEvent(dataContainer); // MARIA  done by TDartAnaManager before calling UpdateHistograms

  for(unsigned int i = 0; i < dev->dartChannel.size(); i++)
  {
    GetHistogram(dev->dartChannel[i].ch)->Fill(dev->dartChannel[i].charge);
  }
  for(unsigned int i = 0; i < dev->vetoChannel.size(); i++)
  {
    GetHistogram(dev->vetoChannel[i].Vch)->Fill(dev->vetoChannel[i].Vcharge);
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
  { // loop over 16 channels
      
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
  TDartEvent * dev = TEventProcessor::instance()->GetDartEvent();
  //TEventProcessor::instance()->ProcessMidasEvent(dataContainer); // MARIA  done by TDartAnaManager before calling UpdateHistograms

  for(unsigned int i = 0; i < dev->dartChannel.size(); i++)
  {
    GetHistogram(dev->dartChannel[i].ch)->Fill(dev->dartChannel[i].high);
  }
  for(unsigned int i = 0; i < dev->vetoChannel.size(); i++)
  {
    GetHistogram(dev->vetoChannel[i].Vch)->Fill(dev->vetoChannel[i].Vhigh);
  }
}

TCanvasHandleBase* THistoCharges::CreateCanvas()
{
//  return new TSimpleHistogramCanvas(GetHistogram(0),"Charge Channel", "COLZ");
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

THistoChargeSummary::THistoChargeSummary()
{

  SetTabName("Charge");
  SetSubTabName("Charge Summary");
  CreateHistograms();
}

void THistoChargeSummary::CreateHistograms()
{
  char name[100];
  sprintf(name,"ChargeSummaryDart");

  // check if we already have histogramss.
  if(size() != 0){

    TH1D *tmp = (TH1D*)gDirectory->Get(name);
    if (tmp) return;
  }

  // Otherwise make histograms
  clear();

  char title[100];
  sprintf(title,"Dart Charge Summary");
  TH1D *tmp = new TH1D(name, title, 5000, 0,500000);
  tmp->SetXTitle("Charge (ADC*sample value)");
  tmp->SetYTitle("Counts");
  push_back(tmp);

  sprintf(title,"Veto Charge Summary");
  tmp = new TH1D(name, title, 5000, 0,500000);
  tmp->SetXTitle("Charge (ADC*sample value)");
  tmp->SetYTitle("Counts");
  push_back(tmp);
}

void THistoChargeSummary::UpdateHistograms(TDataContainer& dataContainer)
{
  TDartEvent * dev = TEventProcessor::instance()->GetDartEvent();
  //TEventProcessor::instance()->ProcessMidasEvent(dataContainer); // MARIA  done by TDartAnaManager before calling UpdateHistograms

  GetHistogram(0)->Fill(dev->totCharge);
  GetHistogram(1)->Fill(dev->vetoCharge);

}

TCanvasHandleBase* THistoChargeSummary::CreateCanvas()
{
  return new TSimpleHistogramCanvas(GetHistogram(0),"Charge Summary", "COLZ");
}



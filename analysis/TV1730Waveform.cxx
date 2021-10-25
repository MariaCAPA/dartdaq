#include "TV1730Waveform.hxx"
#include "TV1730RawData.hxx"
#include "TH1D.h"
#include "TDirectory.h"
#include <iostream>

using namespace std;

const int VMEBUS_BOARDNO = 0;


/// Reset the histograms for this canvas
TV1730Waveform::TV1730Waveform() 
{
  fCanvas = NULL;

  SetTabName("V1730");
  SetSubTabName("Waveforms");
  SetUpdateOnlyWhenPlotted(false); // So visible on web

  SetNanosecsPerSample(2); // 500 MHz
  fNSamples = 4000; // default
  fNChannels=16; // default
  fChannels.clear();
  for (int i=0; i<fNChannels; i++) fChannels.push_back(i);

  CreateHistograms();
  SetNumberChannelsInGroup(fNChannels);

  if (fCanvas) 
  {
    fCanvas->SetGroupName("Group");
    fCanvas->SetChannelName("Channel");
    //fCanvas->HandleChangedNumberOfHistos();
  }
}

void TV1730Waveform::DeleteHistograms() 
{
  // Delete existing histograms
  for (int i = 0; i < fNChannels; i++) 
  { // loop over channels
    char tname[100];
    sprintf(tname, "V1730_%i", fChannels[i]);
    TH1D *tmp = (TH1D*) gDirectory->Get(tname);
    delete tmp;
  }
  clear();

}

void TV1730Waveform::CreateHistograms() 
{
  // check if we already have histograms.
  if (size() != 0) 
  {
    char tname[100];
    sprintf(tname, "V1730_%i", fChannels[0]);

    TH1D *tmp = (TH1D*) gDirectory->Get(tname);
    if (tmp)
      return;
  }

  int WFLength = fNSamples * nanosecsPerSample; // Need a better way of detecting this...

  // Otherwise make histograms
  clear();

  for (int i = 0; i < fNChannels; i++) 
  { // loop over channels

    char name[100];
    char title[100];
    sprintf(name, "V1730_%i", fChannels[i]);
    TH1D *otmp = (TH1D*) gDirectory->Get(name);
    if (otmp) 
    {
      delete otmp;
    }

    sprintf(title, "V1730 Waveform for channel=%i", fChannels[i]);

    TH1D *tmp = new TH1D(name, title, fNSamples, 0, WFLength);
    tmp->SetXTitle("ns");
    tmp->SetYTitle("ADC value");

    push_back(tmp);
  }
}

void TV1730Waveform::UpdateHistograms(TDataContainer &dataContainer) 
{
  //int fe_idx = dataContainer.GetMidasEvent().GetTriggerMask();

  TDartEvent * dev = TEventProcessor::instance()->GetDartEvent(); // updated in TDartAnaManager

  char name[100];
  sprintf(name, "WF%02d", VMEBUS_BOARDNO); // Check for module-specific data
  TV1730RawData *V1730 = dataContainer.GetEventData<TV1730RawData>(name);

  //int base_idx = fFEAndBoardToHistoOrder[std::make_pair(fe_idx, board_idx)] * fNChannels;

  // if there are no channels, return 
  if (!V1730 || V1730->GetNChannels()==0) 
  {
    printf("Didn't see bank %s or there are no channels \n", name);
    return ;
  }

  // if channels or number of samples have changed, force new histograms
  TV1730RawChannel& channelData = V1730->GetChannelData(0);
  if (fNChannels != V1730->GetNChannels() || fNSamples!=channelData.GetNSamples() )
  {
    DeleteHistograms();
    fNSamples = channelData.GetNSamples();
    std::cout << "V1730Waveforms: updating n channels " << V1730->GetNChannels() <<  " and waveform size " << fNSamples << " bins." << std::endl;
    fChannels.clear(); 
    fNChannels = V1730->GetNChannels();
    for (int i=0; i<fNChannels; i++) fChannels.push_back(V1730->GetChannelData(i).GetChannelNumber());
 
    CreateHistograms();

  }

/*
    if (V1730->GetHeader().format != 0x10) 
    {
      // Not scope-data format
std::cout << "not scope data format " << std::endl;
      continue;
    }
*/

  // loop over channels
  for (int index = 0; index < fNChannels; index++) 
  {
    TV1730RawChannel& channelData = V1730->GetChannelData(index);
   
    for (int samp = 0; samp < channelData.GetNSamples(); samp++) 
    {
      double adc = channelData.GetADCSample(samp);
      GetHistogram(index)->SetBinContent(samp + 1, adc);
    }
  }
}

void TV1730Waveform::AddHistogramsChannel(TDataContainer &dataContainer, int ch) 
{
  char name[100];
  sprintf(name, "WF%02d", VMEBUS_BOARDNO); // Check for module-specific data
  TV1730RawData *V1730 = dataContainer.GetEventData<TV1730RawData>(name);

  // if there are no channels, return 
  if (!V1730 || V1730->GetNChannels()==0) 
  {
    printf("Didn't see bank %s or there are no channels \n", name);
    return ;
  }

  // if channels or number of samples have changed, force new histograms
  TV1730RawChannel& channelData = V1730->GetChannelData(0);
  if (fNChannels != V1730->GetNChannels() || fNSamples!=channelData.GetNSamples() )
  {
    DeleteHistograms();
    fNSamples = channelData.GetNSamples();
    std::cout << "V1730Waveforms: updating n channels " << V1730->GetNChannels() <<  " and waveform size " << fNSamples << " bins." << std::endl;
    fChannels.clear(); 
    fNChannels = V1730->GetNChannels();
    for (int i=0; i<fNChannels; i++) fChannels.push_back(V1730->GetChannelData(i).GetChannelNumber());
    CreateHistograms();
  }

  // look for channel index
  int index=0;
  for (; index < fNChannels; index++) if (fChannels[index]==ch) break;
  if (index==fNChannels) { std::cout << "Channel " << ch << " not found! " << std::endl; return;}

  // add channel
  channelData = V1730->GetChannelData(index);
  for (int samp = 0; samp < channelData.GetNSamples(); samp++) 
  {
    double adc = channelData.GetADCSample(samp);
    GetHistogram(index)->AddBinContent(samp + 1, adc);
  }
}


void TV1730Waveform::Reset() 
{
  // loop over channels
  for (int index = 0; index < fNChannels; index++) 
  { 
    // Reset the histogram...
    for (int ib = 0; ib < GetHistogram(index)->GetNbinsX(); ib++) 
    {
      GetHistogram(index)->SetBinContent(ib, 0);
    }

    GetHistogram(index)->Reset();
  }
}

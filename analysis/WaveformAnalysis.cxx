// Example Program for converting MIDAS format to ROOT format.
#include <TH1D.h>
#include <TH2D.h>
#include <TF1.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TCanvas.h>
#include <TPaveText.h>
#include <TMath.h>
#include <vector>
#include <TH1F.h>
#include <TStyle.h> //añadido

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include <vector>

#include "TRootanaEventLoop.hxx"
#include "TFile.h"
#include "TTree.h"
#include "TLine.h" //añadido
#include "TDataContainer.hxx"
#include "TDartEvent.hxx"
#include "TV1730RawData.hxx"

#include "TDartAnaManager.hxx"
#include "TEventProcessor.hxx"
#include "TEventMusic.hxx"

#include <TMidasEvent.h>
#include <TDataContainer.hxx>
#include <THistogramArrayBase.h>

#include "midasio.h"

const int VMEBUS_BOARDNO = 0;

int main(int argc, char *argv[])
{
  
  //We load the MIDAS data file
  const char * file = argv[1];
   
  TMReaderInterface * reader = TMNewReader(file);
  if (reader->fError)
 	{
    printf("Cannot open input file \"%s\"\n",file);
 		delete reader;
    return -1;
 	}
   
  //We prepare the output
  int nSamples=250;
  int nChannels=8;
  
  std::string nameConfig = std::string(argv[2]);
  
  std::string fileConfig = Form("%s.txt",nameConfig.c_str());
  
  std::fstream fConfig (fileConfig.c_str(), std::ios::in);
  
	fConfig >> nSamples;
	fConfig >> nChannels;

  
  std::string filenew = Form("../analyzed/ANAISplusWaveforms/%s_waveforms.root",file);
  
  TFile * fnew = new TFile(filenew.c_str(), "recreate");
  
  TTree * t2 = new TTree("ta","");
   
  TH1F ** hP = new TH1F * [nChannels]; //Pulses histograms

  for(int j = 0; j < nChannels; j++) 
  {
    hP[j] = new TH1F(Form("hP_%d",j),Form("hP_%d",j),nSamples,0,nSamples);
    t2->Branch(Form("pulse%d",j),"TH1F",&hP[j],32000,0);
  }
  
  //Time of adquisition
  double timeFile;
  
  t2->Branch("timeFile",&timeFile,"&timeFile/D");
  
  //We analyze each event
  int ev=0;
  
  TMidasEvent event;
  
  char name[100];
	sprintf(name, "WF%02d", VMEBUS_BOARDNO);
  
 	while (TMReadEvent(reader, &event))
	{
 
   if ((event.GetEventId() & 0xFFFF) == 0x8000) continue; // begin of run
   
   TDataContainer dataContainer;
   
   if(event.GetEventId()>1) break;
   
		// Set the midas event pointer in the physics event
		dataContainer.SetMidasEventPointer(event);
   
		TV1730RawData * V1730 = dataContainer.GetEventData<TV1730RawData>(name);
   
    if(V1730->GetNChannels()!=nChannels)
    {
      printf("Incorrect number of channels in config file \n");
      
      return -1;
    }
   
		if (!V1730 || V1730->GetNChannels()==0)
		{
			printf("Didn't see bank %s or there are no channels \n", name);

			return -1;
		}
   
    if(ev%100 == 0) {std::cout << ev << " " << file << std::endl;}
    
    //Time of adquisition
    
    timeFile = V1730->GetHeader().timeStampNs;
    
    //We build the waveform histograms
    
    for(int j = 0; j < nChannels; j++) 
    {
      TV1730RawChannel channelData = V1730->GetChannelData(j);
      
      if(channelData.GetNSamples()!=nSamples)
      {
        printf("Incorrect number of samples in config file \n");
        
        return -1;
      }
      
      for(int i = 0; i < nSamples; i++) hP[j]->SetBinContent(i+1,1.0*channelData.GetADCSample(i));
    }
    
    t2->Fill();
    
		if(ev%1000 == 0) t2->AutoSave();

		ev++; 
  }
  
  t2->Print();
	t2->Write();

  fnew->Close();

	return 0;
}

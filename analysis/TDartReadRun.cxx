#include "TDartReadRun.hxx"
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
#include "TDartVisu.hxx"

TV1730Waveform *TDartReadRun::GetWaveform(int evNo, bool draw, bool dump)
{
  TDartVisu & visu = (TDartVisu&)(TDartVisu::Get());
  //manager.AddHistogram(wf);
  //display.AddSingleCanvas(wf->CreateCanvas(), wf->GetTabName());

  if (fReader) delete fReader; 

  // DUmp event
  fTree->GetEntry(evNo);
  if(dump) fCurrentEv->Dump();
  visu.fCurrentEv = fCurrentEv;

  // look for partial and open it
  int partial = fTree->GetTreeNumber();
  std::string filename = fDataBaseName + Form("%05d_%02d.mid.lz4", fRun, partial);
  fReader = TMNewReader(filename.c_str());
  if (fReader->fError) 
  {
    printf("Cannot open input file \"%s\"\n",filename.c_str());
    delete fReader;
    return 0;
  }

  TMidasEvent event;
  while (TMReadEvent(fReader, &event))
  {
    if ((event.GetEventId() & 0xFFFF) == 0x8000) continue; // begin of run
    if (event.GetSerialNumber()==(unsigned int)evNo)
    {
      event.SetBankList();
      //event.Print();
      TDataContainer dataContainer;
      // Set the midas event pointer in the physics event.
      dataContainer.SetMidasEventPointer(event);
      visu.UpdateHistograms(dataContainer);
      //visu.ProcessMidasEvent(dataContainer);
      if (draw) visu.PlotCanvas(dataContainer);
      //manager.UpdateTransientPlots(dataContainer);
      return visu.fWf;
    }
  }
  return 0;
}


TDartReadRun::TDartReadRun(int run, std::string rootBaseName, std::string dataBaseName)
{
  TDartVisu::CreateSingleton<TDartVisu>();
   TDartVisu::Get().InitializeRAD();
  //TDartVisu & visu = (TDartVisu&)(TDartVisu::Get());
  fRootBaseName=rootBaseName;
  fDataBaseName=dataBaseName;

  fRun = run;
  fTree = new TChain("td");
  fReader=0;
  int maxPartial = 99;
  for (int parcial=0; parcial<maxPartial; parcial++)
  {
    std::string filename = rootBaseName + Form("_%06d_%04d.root", run, parcial);
    std::cout << " adding " << filename.c_str() ;
    if (access(filename.c_str(), F_OK)==-1) { std::cout << " .. not found. Stop" << std::endl; break;}
    fTree->Add(filename.c_str(), -1);
    std::cout << std::endl;
  }
  fTree->SetEstimate(fTree->GetEntries());
  
  fTree->SetBranchAddress("DartEvent",&fCurrentEv);
}

TV1730Waveform *TDartReadRun::GetWaveformFromSelectedEvents(int index, bool draw, bool dump)
{
  if ((unsigned int)index<fSelectedEvents.size())
  return GetWaveform(fSelectedEvents[index], draw, dump);
  
  return 0;

}


int TDartReadRun::SetSelection (std::string cut)
{
  fSelectedEvents.clear();
  int nsel = fTree->Draw("Entry$", cut.c_str(), "goff");
  for (int i=0; i<nsel; i++) fSelectedEvents.push_back(fTree->GetV1()[i]);
  std::cout << " selected " << nsel << " events " << std::endl;
  return nsel;
}

TV1730Waveform *TDartReadRun::GetAverageFromSelectedEvents(bool draw, bool dump)
{
  TV1730Waveform * average = new TV1730Waveform("average");

  TDartVisu & visu = (TDartVisu&)(TDartVisu::Get());
  TMidasEvent event;

  if (fReader) delete fReader; 
  int prevPartial = -1;
  int partial = 0;
  int evNo;
  bool found=0;

  for (unsigned int index=0; index<fSelectedEvents.size(); index++)
  {
if (index%100==0) std::cout << " processed " << index << " events " << std::endl;

   found = 0;
    evNo = fSelectedEvents[index];
    fTree->GetEntry(evNo);
    // look for partial
    partial = fTree->GetTreeNumber();
    // if it has changed, open new file 
    if (partial!=prevPartial)
    {
      std::string filename = fDataBaseName + Form("%05d_%02d.mid.lz4", fRun, partial);
      fReader = TMNewReader(filename.c_str());
      if (fReader->fError) 
      {
        printf("Cannot open input file \"%s\"\n",filename.c_str());
        delete fReader;
        return 0;
      }
      prevPartial=partial;
    }
    while (!found && TMReadEvent(fReader, &event))
    {
      if ((event.GetEventId() & 0xFFFF) == 0x8000) continue; // begin of run
      if (event.GetSerialNumber()==(unsigned int)evNo)
      {
        event.SetBankList();
        TDataContainer dataContainer;
        // Set the midas event pointer in the physics event.
        dataContainer.SetMidasEventPointer(event);
        visu.UpdateHistograms(dataContainer);
        average->AddWaveform(visu.fWf);
        found=1;
      }
    } // end while event not found

  } // end  selected events loop
  if (draw)
  {
    visu.PlotCanvas(average);
  }
  return average;
   
}

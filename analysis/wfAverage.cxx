#include <stdio.h>
#include <iostream>
#include <time.h>
#include <vector>

#include "TRootanaEventLoop.hxx"
#include "TFile.h"
#include "TTree.h"

#include "TAAnaManager.hxx"
#include "TEventProcessor.hxx"


class WfAverage: public TRootanaEventLoop {

public:

  // Histograms to fill
  TV1730Waveform* fAverage;

  // A event that is being processed
  TAEvent * fDEv;

  // channels to average
  std::vector<int> fChannels;

  // file name
  std::string fFileName;


  WfAverage() 
  {
    fAverage = new TV1730Waveform();
    fDEv = 0;
    fFileName = "average"; 
  }

  virtual ~WfAverage() 
  {
    if (fAverage) delete fAverage;
  }

  void Initialize()
  {
    UseBatchMode();
  
  }
  
  
  void BeginRun(int transition,int run,int time)
  {
    TEventProcessor::instance()->SetRun(run);
    //CreateOutputFile("test");
    // Create a TTree
    fDEv = TEventProcessor::instance()->GetAEvent();
  }   

  void EndRun(int transition,int run,int time)
  {
    printf("end run %d\n",run);
    // save average to file
    TFile * file = new TFile(Form("%s_%06d.root",fFileName.c_str(), run), "update");
    for (unsigned int index = 0; index<fChannels.size(); index++) fAverage->GetHistogram(index)->Write();
    file->Close();
  }

  
  
  // Main work here; create ttree events for every sequenced event in 
  // Lecroy data packets.
  bool ProcessMidasEvent(TDataContainer& dataContainer)
  {
    // update fDEv with event in dataContainer. 
    TEventProcessor::instance()->ProcessMidasEvent(dataContainer);

    // loop in channels
    for (unsigned int ind=0; ind<fChannels.size(); ind++)
    {
      int ch=fChannels[ind];
      // TODO Check if it pass the cuts
      //if () // this channel pass conditions, add wf
      {    
        fAverage->AddHistogramsChannel(dataContainer, ch);
      }
      
    }
    return true;

  };
  

}; 


int main(int argc, char *argv[])
{
  WfAverage::CreateSingleton<WfAverage>();
  WfAverage & wf = (WfAverage&)(WfAverage::Get());

  // TODO channels to average
  wf.fChannels.push_back(0);
  wf.fChannels.push_back(1);


  // TODO configure cuts

  // TODO configure file name
  
  return wf.ExecuteLoop(argc, argv);

}


// Example Program for converting MIDAS format to ROOT format.
#include <stdio.h>
#include <iostream>
#include <time.h>
#include <vector>

#include "TRootanaEventLoop.hxx"
#include "TFile.h"
#include "TTree.h"

#include "TDartAnaManager.hxx"
#include "TEventProcessor.hxx"


class Analyzer: public TRootanaEventLoop {

public:

  // An analysis manager.  Define and fill histograms in 
  // analysis manager.
  TDartAnaManager *anaManager;

  // The tree to fill.
  TTree *fTree;

  int timestamp;
  int serialnumber;
#ifdef USE_V792
  // CAEN V792 tree variables
  int nchannels;
  int adc_value[32];
#endif


  Analyzer() {};

  virtual ~Analyzer() {};

  void Initialize()
  {
    std::cout << " initialize in batch mode " << std::endl;
    UseBatchMode();
  }
  
  
  void BeginRun(int transition,int run,int time)
  {
std::cout << " is offline : " << IsOffline() << std::endl;
    TEventProcessor::instance()->SetRun(run);
    //CreateOutputFile("test");
    // Create a TTree
    fTree = new TTree("td",Form("MIDAS data run %d",run));
    TDartEvent * dev = TEventProcessor::instance()->GetDartEvent();
    fTree->Branch("DartEvent",dev);
  }   

  void EndRun(int transition,int run,int time)
  {
        printf("end run %d\n",run);
    fTree->Write();
    CloseRootFile();
  }

  
  
  // Main work here; create ttree events for every sequenced event in 
  // Lecroy data packets.
  bool ProcessMidasEvent(TDataContainer& dataContainer)
  {
//std::cout << " processing event " << dataContainer.GetMidasEvent().GetSerialNumber() << std::endl;
    TEventProcessor::instance()->ProcessMidasEvent(dataContainer);

    fTree->Fill();

    return true;

  };
  
  // Complicated method to set correct filename when dealing with subruns.
  std::string SetFullOutputFileName(int run, std::string midasFilename)
  {
    char buff[128]; 
    Int_t in_num = 0, part = 0;
    Int_t num[2] = { 0, 0 }; // run and subrun values
    // get run/subrun numbers from file name
    for (int i=0; ; ++i) {
      char ch = midasFilename[i];
        if (!ch) break;
        if (ch == '/') {
          // skip numbers in the directory name
          num[0] = num[1] = in_num = part = 0;
        } else if (ch >= '0' && ch <= '9' && part < 2) {
          num[part] = num[part] * 10 + (ch - '0');
          in_num = 1;
        } else if (in_num) {
          in_num = 0;
          ++part;
        }
    }
    if (part == 2) {
      if (run != num[0]) {
        std::cerr << "File name run number (" << num[0]
                  << ") disagrees with MIDAS run (" << run << ")" << std::endl;
        exit(1);
      }
      sprintf(buff,"output_%.6d_%.4d.root", run, num[1]);
      printf("Using filename %s\n",buff);
    } else {
      sprintf(buff,"output_%.6d.root", run);
    }
    return std::string(buff);
  };





}; 


int main(int argc, char *argv[])
{

  Analyzer::CreateSingleton<Analyzer>();
  return Analyzer::Get().ExecuteLoop(argc, argv);

}


// Example Program for converting MIDAS format to ROOT format.
#include <stdio.h>
#include <iostream>
#include <time.h>
#include <vector>

#include "TRootanaEventLoop.hxx"
#include "TFile.h"
#include "TTree.h"

#include "TAAnaManager.hxx"
#include "TEventProcessor.hxx"

#include <TMidasEvent.h>
#include <TDataContainer.hxx>
#include <THistogramArrayBase.h>

#include "midasio.h"


const int VMEBUS_BOARDNO = 0;

// provisional mientras tenga que hacer los banks a mano
static int cont=0;


class Analyzer: public TRootanaEventLoop {

public:

  // An analysis manager.  Define and fill histograms in 
  // analysis manager.
  TAAnaManager *anaManager;

  // The tree to fill.
  TTree *fTree;

  // tree vars
    const Int_t kMaxCh = 16;
   Int_t run;
   Int_t mult;
   std::vector<Int_t> triggered;
   std::vector<Float_t> area;
   std::vector<Float_t> bsl;
   std::vector<Float_t> rms;
   std::vector<Float_t> t0;
   std::vector<Float_t> tMin;
   std::vector<Float_t> tMax;
   std::vector<Float_t> min;
   std::vector<Float_t> max;
   std::vector< std::vector < int > > pulse;
   Double_t time;
   Double_t timeNs;
   Double_t dt;

  // pointer to the event
   TAEvent * fEv;
  // other vars
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
    fTree->Branch("run",&run);
    fTree->Branch("mult",&mult);
    fTree->Branch("triggered",&triggered);
    fTree->Branch("pulse",&pulse);
    fTree->Branch("area",&area);
    fTree->Branch("bsl",&bsl);
    fTree->Branch("rms",&rms);
    fTree->Branch("t0",&t0);
    fTree->Branch("tMin",&tMin);
    fTree->Branch("tMax",&tMax);
    fTree->Branch("min",&min);
    fTree->Branch("max",&max);
    fTree->Branch("time",&time);
    fTree->Branch("timeNs",&timeNs);
    fTree->Branch("dt",&dt);

    fEv = TEventProcessor::instance()->GetAEvent();





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
    TMidasEvent & event = dataContainer.GetMidasEvent();
    int nbk = event.SetBankList();
// VERBOSE
//std::cout << " nbk: " << nbk << std::endl;
     // Maria 160322 instead of:
    //TEventProcessor::instance()->ProcessMidasEvent(dataContainer);

    // do manually
    TMidas_BANK32 *pbk32 = NULL;
    char *pdata = NULL;
    for (int bk=0; bk<nbk; bk++)
    {
      //TEventProcessor::instance()->GetAEvent()->Reset();
      //TEventProcessor::instance()->GetAEvent()->run=TEventProcessor::instance()->GetRun();
      //TEventProcessor::instance()->GetAEvent()->eventNumber= cont++;
      //TEventProcessor::instance()->GetAEvent()->midasEventNumber= event.GetSerialNumber();
      //TEventProcessor::instance()->GetAEvent()->bankNumber= bk;
      //TEventProcessor::instance()->GetAEvent()->time = event.GetTimeStamp();
      fEv->Reset();
      fEv->run=TEventProcessor::instance()->GetRun();
      fEv->eventNumber= cont++;
      fEv->midasEventNumber= event.GetSerialNumber();
      fEv->bankNumber= bk;
      fEv->time = event.GetTimeStamp();

      //void ** pdata;
      //event.IterateBank32(&pbk32, (char**)pdata);
      event.IterateBank32(&pbk32, &pdata);
      if (pbk32 == NULL) break;
      TV1730RawData *bank = new TV1730RawData(pbk32->fDataSize,pbk32->fType,pbk32->fName, pdata);
      //std::cout << " bank " << bk << " name " << pbk32->fName << " size " << pbk32->fDataSize << " type: " << pbk32->fType << std::endl;
      TEventProcessor::instance()->ProcessMidasEvent(bank);


      // Copy info to tree vars
      run=fEv->run;
      mult=fEv->mult;
      for (int i=0; i<mult; i++)
      {
        triggered.push_back(fEv->channel[i].ch);
        area.push_back(fEv->channel[i].area);
        bsl.push_back(fEv->channel[i].bsl);
        rms.push_back(fEv->channel[i].rms);
        t0.push_back(fEv->channel[i].t0);
        tMin.push_back(fEv->channel[i].tMin);
        tMax.push_back(fEv->channel[i].tMax);
        min.push_back(fEv->channel[i].min);
        max.push_back(fEv->channel[i].max);

        TV1730RawChannel& channelData = bank->GetChannelData(i);
        int nSamples = channelData.GetNSamples();
        std::vector<int> aux;
        for (int j=0; j<nSamples; j++)
        {
          aux.push_back(channelData.GetADCSample(j));
        }
        pulse.push_back(aux);
      }

      time=(double)fEv->time;
      timeNs=(double)fEv->timeNs;
      dt= (double)fEv->dt;

    }

    fTree->Fill();
    triggered.clear();
    area.clear();
    bsl.clear();
    rms.clear();
    t0.clear();
    tMin.clear();
    tMax.clear();
    min.clear();
    max.clear();
    pulse.clear();

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

int Initializetmin(const char *file)
{
/*
  // read  number of channels

  double dummy0, dummy1, dummy2, dummy3, dummy4, dummy5, dummy6, dummy7, dummy8, dummy9, dummy10, dummy11, tMax;

  TMReaderInterface * reader = TMNewReader(file);
  if (reader->fError)
  {
    printf("Cannot open input file \"%s\"\n",file);
    delete reader;
    return -1;
  }

  TMidasEvent event;
  char name[100];
  sprintf(name, "WF%02d", VMEBUS_BOARDNO); // Check for module-specific data
  int ev=0;
  int nCh=0;

  std::vector<double> tini_aux; // auxiliar array
  std::vector<double> nEv; // auxiliar array
  // READ FIRST 100 events
  while (TMReadEvent(reader, &event) && ev<100)
  {
    if ((event.GetEventId() & 0xFFFF) == 0x8000) continue; // begin of run
    int nbk = event.SetBankList();

    //event.Print();
    TDataContainer dataContainer;
    // Set the midas event pointer in the physics event.
    dataContainer.SetMidasEventPointer(event);

    TV1730RawData *V1730 = dataContainer.GetEventData<TV1730RawData>(name);
    // if there are no channels, return 
    if (!V1730 || V1730->GetNChannels()==0)
    {
      printf("Didn't see bank %s or there are no channels \n", name);
      return -1;
    }

    // for first event, initialize nchannels and vectors
    if (nCh==0)
    {
      nCh =  V1730->GetNChannels();
      // initialize to 0 auxliar array
      for (int i=0; i<nCh; i++) tini_aux.push_back(0);
      for (int i=0; i<nCh; i++) nEv.push_back(100);
    } // end if nch==0

    // loop in channels
    for (int i=0; i<nCh; i++)
    {
      TEventProcessor::instance()->GetBasicParam(V1730->GetChannelData(i), dummy0, dummy1, dummy2, dummy3, dummy4, dummy5, dummy6, dummy7, dummy8, tMax, dummy9, dummy10, dummy11);
      if (tMax>0) tini_aux[i]+=tMax;
      else nEv[i]--;
    }
    ev++;
  }
  
  // set tini vals
  for (int i=0; i<nCh; i++) {TEventProcessor::tini_push_back(tini_aux[i]/nEv[i]); std::cout << " ch " << i << " tini: " << tini_aux[i]/nEv[i] << std::endl;}
  
*/
  return 0;
}


int main(int argc, char *argv[])
{

  // MARIA NOT BY NOW
  //int error = Initializetmin(argv[1]);
  //if (error) return error;

  Analyzer::CreateSingleton<Analyzer>();
  return Analyzer::Get().ExecuteLoop(argc, argv);

}


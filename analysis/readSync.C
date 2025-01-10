#include <TFile.h>
#include <TTree.h>
#include <TGraph.h>

//////////////////// ANAIS
TChain * tA;
//TTree * tA;
int runAnais;

/////////////////// Anod
TChain * td;
int NDetAnod = 10; // CHANGE HERE
int runAnod;
std::vector<TH1S> * pul;
//std::string AnodBaseName = "./Anod112DM";
//std::string AnodBaseName = "/media/storage/data/Anod112DMtest/Anod112DM";
//std::string AnodBaseName = "/media/storage/data/Anod112DM_analyzed/Anod112DM";
std::string AnodBaseName = "/media/storage4/Anod112DM/Anod112DM";
std::string SyncAnodBaseName = "/media/storage/data/Anod112DM_analyzed/Anod112DM";
std::string AnaisBaseName = "/media/storage/data/A112DM/A112DM";
std::string AnaisBaseNameCal = "/media/storage/data/A112DM/A112DMcal";
TCanvas * cPulsesN=0;


//double CORRECTION_FACTOR = 0.99999788; // set to 1 for same clock
double CORRECTION_FACTOR = 1;

TTree * readAnod(int run, std::string anodbasename=AnodBaseName)
{
  td = new TChain("td");
  int maxPartial = 9999;
  for (int parcial=0; parcial<maxPartial; parcial++)
  {
    std::string filename = anodbasename + Form("_%06d_%04d.root", run, parcial);
    std::cout << " adding " << filename.c_str() ;
    if (access(filename.c_str(), F_OK)==-1) { std::cout << " .. not found. Stop" << std::endl; break;}
    td->Add(filename.c_str(), -1);
    std::cout << std::endl;
  }
  td->SetEstimate(td->GetEntries());

  td->SetBranchAddress("pulse",&pul);
  td->GetEntry(0);
  std::cout << " to visualize pulses use drawPulsesAnod(i++) " << std::endl;
  return td;
}

void drawPulsesAnodch(int i, int ch=0)
{
  td->GetEntry(i);
  (*pul)[ch].Draw();

}
void drawPulsesAnod(int i)
{
  if (!cPulsesN) {cPulsesN=new TCanvas(); cPulsesN->Divide(2,NDetAnod);}
  for (int jj=0; jj<NDetAnod*2; jj++)
  {
    cPulsesN->cd(jj+1);
    drawPulsesAnodch(i, jj);
  }
}
// look for pulse syncronized with i and draw both
void drawPulsesAnaSyn(int i)
{
  int nn = tA->Draw("AnodEvent",Form("Entry$==%d",i),"goff");
  if (nn==0 || tA->GetV1()[0]==-1) {std::cout << " not found. " << std::endl; return;}
  int evN = tA->GetV1()[0];
  drawPulses(i);
  drawPulsesAnod(evN);
  std::cout << " anais event: " << i << std::endl;
  std::cout << " new DAQ event: " << evN << std::endl;

}

//IVAN 09012024
TTree * readAnais(int run, std::string anaisbasename="")
{
  anaisbasename = (run%2 ? AnaisBaseName : AnaisBaseNameCal);
  if(run>=9000) anaisbasename=AnaisBaseName;
  tA = new TChain("T");
  int maxPartial = 9999;
  for (int parcial=0; parcial<maxPartial; parcial++)
  {
    std::string filename = Form("%s.%04d.%02d.a.root",anaisbasename.c_str(), run, parcial);
    std::cout << " adding " << filename.c_str();
    if (access(filename.c_str(), F_OK)==-1) { std::cout << " .. not found. Stop" << std::endl; break;}
    tA->Add(filename.c_str(), -1);
    std::cout << std::endl;
  }
  tA->SetEstimate(tA->GetEntries());
  std::cout << " to visualize pulses use drawPulses(i++) " << std::endl;
  return tA;
}

/*
TTree * readAnais(int run, std::string anaisbasename=AnaisBaseName)
{
  std::string fn = Form("%s.%04d.*.a.root",AnaisBaseName.c_str(), run);
  std::cout << " reading " << fn.c_str() << std::endl;
  tA = load(fn.c_str(),1,0);
  tA->SetEstimate(tA->GetEntries());
  std::cout << " to visualize events use drawPulses(i++)"<<std::endl;
  return tA;
}
*/

void sync()
{
  // open info file
  //std::string outfile = AnodBaseName + Form("_%06d_sync.dat", runAnod);
  //fstream fq(outfile.c_str(), ios::out);
  TNtuple * tinfo = new TNtuple("tinfo","sync info","AnaisEv:AnodEv:timeAnais:timeAnod:delta:delta_deltaIni"); // for every new daq event, AEvent is the corresponding ANAIS event

  tA->Draw("RT0*50.","","goff");
  td->Draw("timeNs","","goff");
  Double_t * timeA = tA->GetV1();
  Double_t * timeN = td->GetV1();

  TNtuple * tdAux = new TNtuple("tdAux","ident anais events","AEvent:timeAnais"); // for every new daq event, AEvent is the corresponding ANAIS event
  TNtuple * tAAux = new TNtuple("tAAux","ident anod events","AnodEvent:timeAnod"); // for every ANAIS event, AEvent is the corresponding anod event

  int iA = 0;
  int iN = 0;
  double deltaIni = timeA[iA]-timeN[iN]/CORRECTION_FACTOR;
  double delta = deltaIni; // allow to vary along the file to adjust loss of ticks
  tdAux->Fill(iA,timeA[iA]); 
  tAAux->Fill(iN,timeN[iN]/CORRECTION_FACTOR+deltaIni); 
  iN++;

  //double epsilon = 100; //  ns max different among coincident events (2 ticks)
  //double epsilon = 1000; //  acquisition window of Anod = 8000, there should not be differences smaller than this
  double epsilon = 8000; //  acquisition window of Anod = 8000, there should not be differences smaller than this
  //double epsilon = 16000; //  acquisition window of Anod = 8000, there should not be differences smaller than this

  int nA = tA->GetEntries();
  int nN = td->GetEntries();
  
  cout << " iA: " << iA << " delta: " << delta << " timeA: " << timeA[0] << " timeN: " << timeN[0] << endl;

  bool stop = false;

  for (iA=1; iA<nA; iA++)
  {
    cout << " iA: " << iA << " delta: " << delta << " timeA: " << timeA[iA] << " timeN: " << timeN[iA] << endl;
    cout << "1st if: " << timeA[iA]-epsilon << " > " << delta+timeN[iN]/CORRECTION_FACTOR << endl;
    cout << "2nd if: " << delta+timeN[iN]/CORRECTION_FACTOR << " > " << timeA[iA]+epsilon << endl;
    cout << "3rd if: " << stop << endl;
    
    //while (!stop && fabs(timeA[iA]-deltaIni-timeN[iN]/CORRECTION_FACTOR) > epsilon) 
    while (!stop && timeA[iA]-epsilon>delta+timeN[iN]/CORRECTION_FACTOR)  // timeN should be larger than timeA
    {
      tdAux->Fill(-1,-1);
      iN++; 
      if (iN==nN) stop = 1;
    }

    // check that it is a coincident event, or there is none
    if (!stop && delta+timeN[iN]/CORRECTION_FACTOR>timeA[iA]+epsilon)
    //if (!stop && fabs(timeA[iA] - deltaIni - timeN[iN]/CORRECTION_FACTOR) > epsilon)
    {
      //fq << "no event found in anod for " << iA << " ANAIS event . time_A = " << timeA[iA] << " delta time with anod: " << timeA[iA]- deltaIni-timeN[iN]/CORRECTION_FACTOR <<  std::endl;
      //fq << std::flush;
      //tinfo->Fill(iA, -1, timeA[iA], timeN[iN]/CORRECTION_FACTOR, timeA[iA]- deltaIni-timeN[iN]/CORRECTION_FACTOR, delta-deltaIni);
      tinfo->Fill(iA, -1, timeA[iA], timeN[iN]/CORRECTION_FACTOR, delta, delta-deltaIni);
      tAAux->Fill(-1,-1);
    }
    else if (!stop)
    {
      tAAux->Fill(iN,timeN[iN]/CORRECTION_FACTOR+deltaIni);
      tdAux->Fill(iA,timeA[iA]);
      // readjust delta for the search of coincidences, to allow for progressive tick loss
      delta = timeA[iA]-timeN[iN]/CORRECTION_FACTOR;
      //fq << " Anais event " << iA << " -->  anod event " << iN << " timeAnais: " << timeA[iA] << " delta - deltaIni: " << delta-deltaIni << std::endl;
      //tinfo->Fill(iA, iN, timeA[iA], timeN[iN]/CORRECTION_FACTOR, timeA[iA]- deltaIni-timeN[iN]/CORRECTION_FACTOR, delta-deltaIni);
      tinfo->Fill(iA, iN, timeA[iA], timeN[iN]/CORRECTION_FACTOR, delta, delta-deltaIni);

      iN++;
      if (iN==nN) stop = 1;
    }
    else break;
  }

  // fill the tree that is not finish yet
  while (iA<nA) {iA++; tAAux->Fill(-1,-1);}
  while (iN<nN) {iN++; tdAux->Fill(-1,-1);}

  // write to disk
  std::string filenameN = SyncAnodBaseName + Form("_%06d_AnaInfo.root", runAnod);
  TFile * fN = new TFile(filenameN.c_str(), "recreate");
  tdAux->Write();
  fN->Close();

  std::string filenameA = AnaisBaseName + Form(".%04d_AnodInfo.root",runAnais);
  TFile * fA = new TFile(filenameA.c_str(), "recreate");
  tAAux->Write();
  fA->Close();

  std::string outfile = SyncAnodBaseName + Form("_%06d_sync.root", runAnod);
  TFile * finfo = new TFile(outfile.c_str(), "recreate");
  tinfo->Write();
  finfo->Close();

 // fq.close();
  
}
///////////////////////// SYNC

void readSync(int rAna, int rNew, std::string anaisbasename="", std::string anodbasename="")
{
  gSystem->Load("libAnod"); //IVAN 19122023
  
  anaisbasename = (rAna%2 ? AnaisBaseName : AnaisBaseNameCal); //IVAN 09012024
  anodbasename = AnodBaseName;
  runAnais=rAna;
  runAnod=rNew;
  readAnais(runAnais,anaisbasename);
  readAnod(runAnod,anodbasename);
  std::cout << " ANAIS tree : tA, New DAQ tree: td " << std::endl;

  // syncronize. If files does not exists, create them
  std::string filenameN = SyncAnodBaseName + Form("_%06d_AnaInfo.root", runAnod);
  std::string filenameA = AnaisBaseName + Form(".%04d_AnodInfo.root",runAnais);
  if (access(filenameN.c_str(), F_OK)==-1 || access(filenameA.c_str(), F_OK)==-1) 
  {  
    std::cout << " .. synchronizing..." << std::endl; 
    sync();
  }
  std::cout << " reading friend trees with syncronized events " << std::endl;
  TFile * fA = new TFile(filenameA.c_str(), "read");
  TTree * tAAux = (TTree*)fA->Get("tAAux");
  if (!tAAux) std::cout << " CANNOT OPEN " << filenameA.c_str() << std::endl;
  else std::cout << " ----------- added friend " << filenameA.c_str() << std::endl;
  TFile * fN = new TFile(filenameN.c_str(), "read");
  TTree * tdAux = (TTree*)fN->Get("tdAux");
  if (!tdAux) std::cout << " CANNOT OPEN " << filenameN.c_str() << std::endl;
  else std::cout << " ----------- added friend " << filenameN.c_str() << std::endl;
  tA->AddFriend(tAAux);
  td->AddFriend(tdAux);
}



void checkSync()
{

  TH1F * hN = new TH1F ("hN","",200,0,20e6);
  TH1F * hA = new TH1F ("hA","",200,0,20e6);
  int nA=tA->GetEntries();
  tA->Draw("RT0*50.","","goff");
  for (int i=1; i<nA; i++) hA->Fill(tA->GetV1()[i]-tA->GetV1()[i-1]);
  int nsel = td->Draw("timeNs","AEvent>-1","goff");
  for (int i=1; i<nsel; i++) hN->Fill(td->GetV1()[i]-td->GetV1()[i-1]);
  hN->SetLineColor(kRed);
  hA->GetXaxis()->SetTitle("time (ns)");
  hA->GetYaxis()->SetTitle("counts/bin");
  
  TCanvas * c = new TCanvas("c","",1200,800);
  hA->Draw();
  hN->Draw("same");
  c->SaveAs(Form("logSync/timeSpc_rAna%04d_rAnod%04d.png",runAnais,runAnod));
  
  TCanvas * c2 = new TCanvas("c2","",1200,800); 
  c2->Divide(2);
  c2->cd(1);
  c2->cd(1)->SetLogy();
  TH1F * hA0 = new TH1F ("hA0","",200,0,50e3);
  tA->Draw("area00>>hA0","","goff");
  hA0->GetXaxis()->SetTitle("area00 (mV ns)");
  hA0->GetYaxis()->SetTitle("counts/bin");
  hA0->Draw();
  c2->cd(2);
  c2->cd(2)->SetLogy();
  TH1F * hn0 = new TH1F ("hn0","",200,0,300e3);
  TH1F * hn0A = new TH1F ("hn0A","",200,0,300e3);
  td->Draw("area[0]>>hn0");
  td->Draw("area[0]>>hn0A","AEvent>-1");
  hn0A->SetLineColor(kRed);
  hn0->GetXaxis()->SetTitle("area[0] (mV ns)");
  hn0->GetYaxis()->SetTitle("counts/bin");
  hn0->Draw();
  hn0A->Draw("same");
  c2->SaveAs(Form("logSync/areaSpc_rAna%04d_rAnod%04d.png",runAnais,runAnod));
  
  TCanvas * c3 = new TCanvas("c3","",1200,800); 
  int nselA = tA->Draw("area00","AnodEvent>-1","goff");
  int nselN = td->Draw("area[0]","AEvent>-1","goff");
  TGraph * g = new TGraph(nselA, tA->GetV1(), td->GetV1());
  g->SetTitle("");
  g->GetXaxis()->SetTitle("area00 (mV ns)");
  g->GetYaxis()->SetTitle("area[0] (mV ns)");
  g->Draw("ap");
  c3->SaveAs(Form("logSync/areaScatter_rAna%04d_rAnod%04d.png",runAnais,runAnod));  
}

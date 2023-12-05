#include <TFile.h>
#include <TTree.h>
#include <TGraph.h>

//////////////////// ANAIS
TTree * tA;
int runAnais;

/////////////////// Anod
TChain * td;
int NDetAnod = 4; // CHANGE HERE
int runAnod;
std::vector<TH1S> * pul;
//std::string AnodBaseName = "./Anod112DM";
std::string AnodBaseName = "/media/storage/data/Anod112DM_analyzed/Anod112DM";
//std::string AnaisBaseName = "/media/storage/data/A112DM/A112DM";
std::string AnaisBaseName = "/media/storage/data/A112DM/A112DM";
TCanvas * cPulsesN=0;


double CORRECTION_FACTOR = 0.99999788; // set to 1 for same clock

TTree * readAnod(int run, std::string anodbasename=AnodBaseName)
{
  td = new TChain("td");
  int maxPartial = 9999;
  for (int parcial=0; parcial<maxPartial; parcial++)
  {
    std::string filename = AnodBaseName + Form("_%06d_%04d.root", run, parcial);
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



TTree * readAnais(int run, std::string anaisbasename=AnaisBaseName)
{
  std::string fn = Form("%s.%04d.*.a.root",AnaisBaseName.c_str(), run);
  std::cout << " reading " << fn.c_str() << std::endl;
  tA = load(fn.c_str(),1,0);
  tA->SetEstimate(tA->GetEntries());
  std::cout << " to visualize events use drawPulses(i++)"<<std::endl;
  return tA;
}

void sync()
{
  // open info file
  std::string outfile = AnodBaseName + Form("_%06d_sync.dat", runAnod);
  fstream fq(outfile.c_str(), ios::out);

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
  double epsilon = 1000; //  acquisition window of Anod = 8000, there should not be differences smaller than this

  int nA = tA->GetEntries();
  int nN = td->GetEntries();

  bool stop = false;

  for (iA=1; iA<nA; iA++)
  {
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
      fq << "no event found in anod for " << iA << " ANAIS event . time_A = " << timeA[iA] << " delta time with anod: " << timeA[iA]- deltaIni-timeN[iN]/CORRECTION_FACTOR << " delta  - deltaIni: " << deltaIni - delta << std::endl;
      fq << std::flush;
      tAAux->Fill(-1,-1);
    }
    else if (!stop)
    {
      tAAux->Fill(iN,timeN[iN]/CORRECTION_FACTOR+deltaIni);
      tdAux->Fill(iA,timeA[iA]);
      // readjust delta for the search of coincidences, to allow for progressive tick loss
      delta = timeA[iA]-timeN[iN]/CORRECTION_FACTOR;
      fq << " Anais event " << iA << " -->  anod event " << iN << " delta - deltaIni: " << delta-deltaIni << std::endl;

      iN++;
      if (iN==nN) stop = 1;
    }
    else break;
  }

  // fill the tree that is not finish yet
  while (iA<nA) {iA++; tAAux->Fill(-1,-1);}
  while (iN<nN) {iN++; tdAux->Fill(-1,-1);}

  // write to disk
  std::string filenameN = AnodBaseName + Form("_%06d_AnaInfo.root", runAnod);
  TFile * fN = new TFile(filenameN.c_str(), "recreate");
  tdAux->Write();
  fN->Close();
  std::string filenameA = AnaisBaseName + Form(".%04d_AnodInfo.root",runAnais);
  TFile * fA = new TFile(filenameA.c_str(), "recreate");
  tAAux->Write();
  fA->Close();

  fq.close();
  
}
///////////////////////// SYNC

void readSync(int rAna, int rNew,std::string anaisbasename=AnaisBaseName, std::string anodbasename=AnodBaseName)
{
  AnaisBaseName = anaisbasename;
  AnodBaseName = anodbasename;
  runAnais = rAna;
  runAnod=rNew;
  readAnais(runAnais);
  readAnod(runAnod);
  std::cout << " ANAIS tree : tA, New DAQ tree: td " << std::endl;

  // syncronize. If files does not exists, create them
  std::string filenameN = AnodBaseName + Form("_%06d_AnaInfo.root", runAnod);
  std::string filenameA = AnaisBaseName + Form(".%04d_AnodInfo.root",runAnais);
  if (access(filenameN.c_str(), F_OK)==-1 || access(filenameA.c_str(), F_OK)==-1) 
  {  
    std::cout << " .. synchronizing..." << std::endl; 
    sync();
  }
  std::cout << " reading friend trees with syncronized events " << std::endl;
  TFile * fA = new TFile(filenameA.c_str(), "read");
  TTree * tAAux = (TTree*)fA->Get("tAAux");
  TFile * fN = new TFile(filenameN.c_str(), "read");
  TTree * tdAux = (TTree*)fN->Get("tdAux");
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

TCanvas * c = new TCanvas();
hA->Draw();
hN->Draw("same");

c = new TCanvas(); c->Divide(2);
c->cd(1);
tA->Draw("area00>>hA0(200,0,50e3");
c->cd(2);
TH1F * hn0 = new TH1F ("hn0","",200,0,300e3);
TH1F * hn0A = new TH1F ("hn0A","",200,0,300e3);
td->Draw("area[0]>>hn0");
td->Draw("area[0]>>hn0A","AEvent>-1");
hn0A->SetLineColor(kRed);
hn0->Draw();
hn0A->Draw("same");
c = new TCanvas(); 
int nselA = tA->Draw("area00","AnodEvent>-1","goff");
int nselN = td->Draw("area[0]","AEvent>-1","goff");
TGraph * g = new TGraph(nselA, tA->GetV1(), td->GetV1());
g->Draw("ap");
}

#include <TFile.h>
#include <TTree.h>
#include <TGraph.h>

//////////////////// ANAIS
TH1F * pulse1, *pulse2, *pulse3, *pulse4;
double t1,t2,t3,t4,t5,t6;
double liveTime;
double bsl1, rms1, bsl2, rms2, bsl3, rms3;
TTree * tA;


/////////////////// NEW DAQ
std::vector<TH1S> * pul;
int i=0;
int nSamples=0;
std::string rootBaseName = "../analyzed/output";


int runAnais;
int runND;

double delta;

TCanvas * cPulsesN=0;
int NDetNew = 2;

TChain * td;
TTree * readND(int run)
{
  td = new TChain("td");
  int maxPartial = 9999;
  for (int parcial=0; parcial<maxPartial; parcial++)
  {
    std::string filename = rootBaseName + Form("_%06d_%04d.root", run, parcial);
    std::cout << " adding " << filename.c_str() ;
    if (access(filename.c_str(), F_OK)==-1) { std::cout << " .. not found. Stop" << std::endl; break;}
    td->Add(filename.c_str(), -1);
    std::cout << std::endl;
  }
  td->SetEstimate(td->GetEntries());

  td->SetBranchAddress("pulse",&pul);
  td->GetEntry(0);
  nSamples=(*pul)[0].GetNbinsX();
  std::cout << " to visualize pulses use drawPulsesND(i++) " << std::endl;
  return td;
}

void drawPulsesNDch(int i, int ch=0)
{
  td->GetEntry(i);
  //TGraph * gg = new TGraph(nSamples);
  //for (int j=0; j<nSamples; j++) gg->SetPoint(j,j,(*pul)[ch][j]);
  //gg->Draw("al");
  (*pul)[ch].Draw();

}
void drawPulsesND(int i)
{
  if (!cPulsesN) {cPulsesN=new TCanvas(); cPulsesN->Divide(2,NDetNew);}
  for (int jj=0; jj<NDetNew*2; jj++)
  {
    cPulsesN->cd(jj+1);
    drawPulsesNDch(i, jj);
  }
}
// look for pulse syncronized with i and draw both
void drawPulsesAnaSyn(int i)
{
  int nn = tA->Draw("NDEvent",Form("Entry$==%d",i),"goff");
  if (nn==0 || tA->GetV1()[0]==-1) {std::cout << " not found. " << std::endl; return;}
  int evN = tA->GetV1()[0];
  drawPulses(i);
  drawPulsesND(evN);
  std::cout << " anais event: " << i << std::endl;
  std::cout << " new DAQ event: " << evN << std::endl;

}



TChain* loadTFIII(int run)
{
  std::string fn;
  //fn = Form("flux.%04d.??.a.root",run);
  fn = Form("flux.%04d.*.a.root",run);
  return load(fn.c_str(),1,0);
}



TTree * readAnais(int run)
{
TFile *f;
  tA = loadTFIII(run);
  std::cout << " to visualize events use drawPulses(i++)"<<std::endl;
  return tA;
}

void read_entry_long(int i, const char * namelist="list")
{
  TEventList * list = (TEventList*)(gDirectory->Get(namelist));
  TEventList * old = (TEventList*)(tA->GetEventList());
 
  if(list)
  {
      tA->SetEventList(list);
      tA->GetEntry(tA->GetEntryNumber(i));
  }
  else
  {
     tA->GetEntry(i);
  }

  //tA->GetEntry(i); 
  pulse1->Draw(); 
  pulse2->Draw("same"); 
  pulse3->Draw("same");
  if (pulse4) pulse4->Draw("same");
  std::cout << " t1: " << t1 << " us " << std::endl;
  std::cout << " t2: " << t2 << " us " << std::endl;
  std::cout << " t3: " << t3 << " us " << std::endl;
  std::cout << " t4: " << t4 << " us " << std::endl;
  if (pulse4)
  {
    std::cout << " t5: " << t5 << " us " << std::endl;
    std::cout << " t6: " << t6 << " us " << std::endl;
  }
  std::cout << " bsl1: " << bsl1 << " rms1: " << rms1 << " adc " << std::endl;
  std::cout << " bsl2: " << bsl2 << " rms2: " << rms2 << " adc " << std::endl;
  std::cout << " bsl3: " << bsl3 << " rms3: " << rms3 << " adc " << std::endl;
  std::cout << " liveTime: " << liveTime << " s " << std::endl;

  TLine * l1 = new TLine(0,bsl1-5*rms1, pulse1->GetBinLowEdge(pulse1->GetNbinsX()), bsl1-5*rms1);
  l1->SetLineColor(kYellow);
  l1->Draw();

  TLine * l2 = new TLine(0,bsl2-5*rms2, pulse2->GetBinLowEdge(pulse2->GetNbinsX()), bsl2-5*rms2);
  l2->SetLineColor(kCyan);
  l2->Draw();

  TLine * l3 = new TLine(0,bsl3-5*rms3, pulse3->GetBinLowEdge(pulse3->GetNbinsX()), bsl3-5*rms3);
  l3->SetLineColor(kRed);
  l3->Draw();
  
  if (t1>0) {TMarker *m1 = new TMarker(t1, bsl1,23);m1->SetMarkerColor(kYellow); m1->Draw();}
  if (t2>0) {TMarker *m2 = new TMarker(t2, bsl2,23);m2->SetMarkerColor(kCyan); m2->Draw();}
  if (t3>0) {TMarker *m3 = new TMarker(t3, bsl3,23);m3->Draw();}
  if (t4>0) {TMarker *m4 = new TMarker(t4, bsl3,23);m4->Draw();}

  tA->SetEventList(old);
}


void syncronize()
{
  tA->Draw("RT0*50.","","goff");
  td->Draw("timeNs","","goff");
  Double_t * timeA = tA->GetV1();
  Double_t * timeN = td->GetV1();

  TNtuple * tdAux = new TNtuple("tdAux","ident anais events","AEvent"); // for every new daq event, AEvent is the corresponding ANAIS event
  TNtuple * tAAux = new TNtuple("tAAux","ident new daq events","NDEvent"); // for every ANAIS event, AEvent is the corresponding new daq event

  int iA = 0;
  int iN = 0;
  delta = timeA[iA]-timeN[iN];
  tdAux->Fill(iA); 
  tAAux->Fill(iN); 
  iN++;

  double epsilon = 100; //  ns max different among coincident events (2 ticks)

  int nA = tA->GetEntries();
  int nN = td->GetEntries();

  bool stop = false;

  for (iA=1; iA<nA; iA++)
  {
    while (!stop && timeA[iA]-epsilon>delta+timeN[iN]) 
    {
      tdAux->Fill(-1);
      iN++; 
      if (iN==nN) stop = 1;
    }

    // check that it is a coincident event, or there is none
    if (!stop && delta+timeN[iN]>timeA[iA]+epsilon)
    {
      std::cout << "no event found in new daq for " << iA << " ANAIS event . time_A = " << timeA[iA] << " delta time with new daq: " << timeA[iA]- delta-timeN[iN] << std::endl;
      tAAux->Fill(-1);
    }
    else if (!stop)
    {
      tAAux->Fill(iN);
      tdAux->Fill(iA);
      iN++;
      if (iN==nN) stop = 1;
    }
    else break;
  }

  // fill the tree that is not finish yet
  while (iA<nA) {iA++; tAAux->Fill(-1);}
  while (iN<nN) {iN++; tdAux->Fill(-1);}

  // write to disk
  std::string filenameN = rootBaseName + Form("_%06d_Ana.root", runND);
  TFile * fN = new TFile(filenameN.c_str(), "recreate");
  tdAux->Write();
  fN->Close();
  std::string filenameA = Form("flux.%04d_Anod.root",runAnais);
  TFile * fA = new TFile(filenameA.c_str(), "recreate");
  tAAux->Write();
  fA->Close();
  
}
///////////////////////// SYNC

void readSync(int rAna, int rNew)
{
  runAnais = rAna;
  runND=rNew;
  readAnais(runAnais);
  readND(runND);
  std::cout << " ANAIS tree : tA, New DAQ tree: td " << std::endl;

  // syncronize. If files does not exists, create them
  std::string filenameN = rootBaseName + Form("_%06d_Ana.root", runND);
  std::string filenameA = Form("flux.%04d_Anod.root",runAnais);
  if (access(filenameN.c_str(), F_OK)==-1 || access(filenameA.c_str(), F_OK)==-1) 
  {  
    std::cout << " .. synchronizing..." << std::endl; 
    syncronize();
  }
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
int nselA = tA->Draw("area00","NDEvent>-1","goff");
int nselN = td->Draw("area[0]","AEvent>-1","goff");
TGraph * g = new TGraph(nselA, tA->GetV1(), td->GetV1());
g->Draw("ap");
}

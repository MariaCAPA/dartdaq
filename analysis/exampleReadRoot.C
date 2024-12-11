// copaindo el texto en root funciona, desde la macro no
// no entiendo bien porque
void exampleReadRoot(std::string file, int event=0, int ch=0)
{
  gROOT->ProcessLine(".L /home/dart/anod32/analysis/libAnod.so");
  TFile * f = new TFile(file.c_str());
  TTree * td = (TTree*)f->Get("td");
  std::vector<TH1S> * pul;
  td->SetBranchAddress("pulse",&pul);
  td->GetEntry(event);
  int nCh = pul->size();
  if (ch<nCh)
  {
    (*pul)[ch].Draw();
  }
  else std::cout << " requested channel not in file" << std::endl;
}

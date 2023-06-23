#include "TAUtils.hxx"
#include <iostream>

#include <TChain.h>


TChain * readARun(int run, std::string baseName)
{
  TChain * t = new TChain("td");
  int maxPartial = 99;
  for (int parcial=0; parcial<maxPartial; parcial++)
  {
    std::string filename = baseName + Form("_%06d_%04d.root", run, parcial);
    std::cout << " adding " << filename.c_str() ;
    if (access(filename.c_str(), F_OK)==-1) { std::cout << " .. not found. Stop" << std::endl; break;}
    t->Add(filename.c_str(), -1);
    std::cout << std::endl;
  }

  t->SetEstimate(t->GetEntries());
  std::cout << " to visualize and event do TAEvent * ev; t->SetBranchAddress(\"AEvent\",&ev); t->GetEntry(i); ev->Dump(); " << std::endl;
  return t;
}


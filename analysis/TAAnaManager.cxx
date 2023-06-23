#include "TAAnaManager.hxx"
#include "TChannelHistograms.hxx"

TAAnaManager::TAAnaManager()
{
 //AddHistogram(new TEvaluationHistograms());

  fVXhisto = new TV1730Waveform();

  AddHistogram(fVXhisto);
  AddHistogram(new THistoArea);
  AddHistogram(new THistoHigh);
  AddHistogram(new THistoAreaSummary);
};


void TAAnaManager::AddHistogram(THistogramArrayBase* histo) {
  histo->DisableAutoUpdate();
  fHistos.push_back(histo);
}

bool dir_exists(MVOdb* odb, const char* dir_name) {
  // Sigh:
  // * ReadKey only implemented for MidasOdb, not JsonOdb or XmlOdb.
  // * ReadDir (for iterating over subkeys) not implemented for any of them.
  // * So just try to Chdir and see if it returns NULL or not.
  return odb->Chdir(dir_name, false) != NULL;
}

void TAAnaManager::BeginRun(int transition,int run,int time, MVOdb* odb) {
  bool using_groups = false;

  std::vector<int> group_idxs;
  std::vector<int> single_idxs;
  std::map<int, int> num_boards_per_fe;

  MVOdb* equip = odb->Chdir("Equipment");

  // Ideally we would look at all the clients that are running, and only
  // choose to show boards from those clients.
  // However MVOdb is incomplete and does not let us list the content of
  // ODB directories for live experiments.....
  /*
  MVOdb* cl = odb->Chdir("System")->Chdir("Clients");

  std::vector<std::string> clients;
  std::vector<int> tids;
  std::vector<int> num_values;
  std::vector<int> total_sizes;
  std::vector<int> item_sizes;

  cl->ReadDir(&clients, &tids, &num_values, &total_sizes, &item_sizes);

  printf("Found %u clients\n", clients.size());

  for (auto client : clients) {
    MVOdb* subdir = cl->Chdir(client.c_str());
    std::string client_name;
    subdir->RS("Name", &client_name);

    printf("Found client %s\n", client_name.c_str());

    if (client_name.find("VX2740_Group_") == 0) {
      using_groups = true;
      int idx;
      sscanf(client_name.c_str(), "VX2740_Group_%d", &idx);
      group_idxs.push_back(idx);
    } else if (client_name.find("VX2740_") == 0) {
      int idx;
      sscanf(client_name.c_str(), "VX2740_%d", &idx);
      single_idxs.push_back(idx);
    }
  }*/

  // .... so instead we just have to see which Equipment directories exist, and whether the
  // "VX2740 Defaults" directory exists.

/*
  using_groups = dir_exists(odb, "V1730 Defaults");

  for (int i = 0; i < 100; i++) {
    char test_name[32];

    if (using_groups) {
      snprintf(test_name, 32, "V1730_Config_Group_%03d", i);

      if (dir_exists(equip, test_name)) {
        group_idxs.push_back(i);
      } else {
      }
    } else {
      snprintf(test_name, 32, "V1730_Config_%03d", i);

      if (dir_exists(equip, test_name)) {
        single_idxs.push_back(i);
      }
    }
  }

  // ... End of annoying hack

  if (using_groups) {
    for (auto idx : group_idxs) {
      char equip_name[255];
      snprintf(equip_name, 255, "V1730_Config_Group_%03d", idx);
      equip->Chdir(equip_name)->Chdir("Settings")->RI("Num boards (restart on change)", &(num_boards_per_fe[idx]));
    }
  } else {
    for (auto idx : single_idxs) {
      num_boards_per_fe[idx] = 1;
    }
  }

  */
  //fVXhisto->SetNumBoardsPerFrontend(num_boards_per_fe);
}

int TAAnaManager::ProcessMidasEvent(TDataContainer& dataContainer)
{

  TEventProcessor::instance()->ProcessMidasEvent(dataContainer);  // AEvent accesible for all histos
  // Fill all the  histograms
  for (unsigned int i = 0; i < fHistos.size(); i++) {
    // Some histograms are very time-consuming to draw, so we can
    // delay their update until the canvas is actually drawn.
    if (!fHistos[i]->IsUpdateWhenPlotted()) {
      fHistos[i]->UpdateHistograms(dataContainer);
    }
  }

  return 1;
}


// Little trick; we only fill the transient histograms here (not cumulative), since we don't want
// to fill histograms for events that we are not displaying.
// It is pretty slow to fill histograms for each event.
void TAAnaManager::UpdateTransientPlots(TDataContainer& dataContainer){
  std::vector<THistogramArrayBase*> histos = GetHistograms();

  for (unsigned int i = 0; i < histos.size(); i++) {
    if (histos[i]->IsUpdateWhenPlotted()) {
      histos[i]->UpdateHistograms(dataContainer);
    }
  }
}



#ifndef TEventMusic_h
#define TEventMusic_h

#include <vector>
#include <TH1F.h>
#include "TDataContainer.hxx"
#include "TDartEvent.hxx"
#include "TV1730RawData.hxx"

class TEventMusic
{
public:

  void ProcessMusicEvent(TV1730RawData * V1730, int *config, double *aqCh, double *calSl, double *bsl, double *rms, double *area, double *areaFix, double *n, double *nFix, double *high, double *t0, double *tmax, double &areaTot, double &nTot, double &areaTotFix, double &nTotFix, double &areaTotWindow, double &nTotWindow, double *max, double *mu, double *mu_tot, double *n_peak, double *P1, double *P2, TH1F *pulse, double *tFix_mod, double *tWin_mod, double *n_peak_tot);

};

#endif


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

  void ProcessMusicEvent(TV1730RawData * V1730, int *config, double *aqCh, double *calSl, double *bsl, double *rms, double *area, double *areaFix, double *n, double *nFix, double *high, double *t0, double *tmax, double &areaTot, double &nTot, double &areaTotFix, double &nTotFix, double &areaTotWindow, double &nTotWindow, double *max, double *mu, double *mu_tot, double *n_peak, double *P1, double *P2, TH1F *pulse0, TH1F *pulse1, TH1F *pulse2, TH1F *pulse3, TH1F *pulse4, TH1F *pulse5, TH1F *pulse6, TH1F *pulse7,  double *tFix_mod, double *tWin_mod, double *n_peak_tot, double *FWHM);

};

#endif


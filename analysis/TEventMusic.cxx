#include "TEventMusic.hxx"
#include "TV1730RawData.hxx"
#include <TH1F.h>
#include "math.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>

using namespace std;
const int VMEBUS_BOARDNO = 0;

// MARIA 070622
#include "midas.h"
#include "msystem.h"

void TEventMusic::ProcessMusicEvent(TV1730RawData * V1730, int *config, double *aqCh, double *calSl, double *bsl, double *rms, double *area, double *areaFix, double *n, double *nFix, double *high, double *t0, double *tmax, double &areaTot, double &nTot, double &areaTotFix, double &nTotFix, double &areaTotWindow, double &nTotWindow, double *max, double *mu, double *mu_tot, double *n_peak, double *P1, double *P2, TH1F *pulse, double *tFix_mod, double *tWin_mod, double *n_peak_tot)
{

	int cal = config[0];
	int pTrig = config[1];
	int amp = config[2];
	int tFix = config[3];
	int tWin = config[4];
	int nSamples = config[5];
	int nCh = config[6];
	int nPtsBsl = config[7];

        TH1F * hAux = new TH1F("hAux","hAux",nSamples,0,nSamples);
   
  //     
  //Ajuste de tFix y Twin para la integracion
  //
  int maxbin, bin;
  double maxheigh, heighlimit, heigh; //heighlimit se pondra como el 1% de maxheigh
  int tmin, tmaxaux, tMaxI;
/*  
  tMaxI=tFix+tWin; //El limite superior de integracion inicial
  
  maxbin=hAux->GetMaximumBin();
  maxheigh=hAux->GetBinContent(maxbin);
  heighlimit=0.01*maxheigh;
  
  bin=maxbin;
  
  heigh=hAux->GetBinContent(bin);
  
  while(heigh>heighlimit)
  {
    heigh=hAux->GetBinContent(bin);
    bin--;
  }
  
  tmin=bin+1;
  
  bin=maxbin;
  
  heigh=hAux->GetBinContent(bin);
  
  while(heigh>heighlimit)
  {
    heigh=hAux->GetBinContent(bin);
    bin++;
  }
  
  tmaxaux=bin-1;
  
  if(tmin>tFix) tFix=tmin;
  
  if(tmaxaux<tMaxI) tMaxI=tmaxaux;
  
  tWin=tMaxI-tFix;
*/  
  //
  //
  //

        double rmsMin, bslCambiante, rmsCambiante, bslRmsMin, trig;
	double bslTot, bslTotF, rmsTot, rmsTotF;

	int i, j, k, l, m, p;
 
  for(j=0; j<8; j++) //Seteo n_peak=0
    {n_peak[j]=0; n_peak_tot[j]=0;}
  
  //Varaibles para calcular n_peak y tiempo promedio
  double bin_time_mu=0;
  
  double media_ant=0, media=0, control=0, control_ant=0, rms_peak=0;

	nTot = 0;
	areaTot = 0;
	nTotFix = 0;
	areaTotFix = 0;
	areaTotWindow = 0;
	nTotWindow = 0;

        for(j = 0; j < 8; j++)
        {

		hAux->Reset();
    if(j==0) pulse->Reset();

                if(aqCh[j] == 1)
                {
                        TV1730RawChannel channelData = V1730->GetChannelData(j);
                        nSamples = channelData.GetNSamples();
                        for(i = 0; i < nSamples; i++) hAux->SetBinContent(i+1,-1.0*channelData.GetADCSample(i));
                }

                if(aqCh[j] == 1) for(i = 2; i <= nSamples; i++) hAux->SetBinContent(i,hAux->GetBinContent(i-1)+0.5*(hAux->GetBinContent(i)-hAux->GetBinContent(i-1)));

                bsl[j] = 0;
                rms[j] = 0;
                l = nPtsBsl;
                area[j] = 0;
                high[j] = 0;
                areaFix[j] = 0;
                rmsMin = 1e6;
                bslRmsMin = 0;
                bslCambiante = 0;
                rmsCambiante = 0;

                for(l = 0; l < nSamples - nPtsBsl; l++)
                {
                        if(l < tFix - nPtsBsl || l > tFix + tWin)
                        {
		                bslCambiante = 0;
		    	        rmsCambiante = 0;

		                for(k = l; k < l + nPtsBsl; k++) bslCambiante += hAux->GetBinContent(k+1)/nPtsBsl;
		                for(k = l; k < l + nPtsBsl; k++) rmsCambiante += (hAux->GetBinContent(k+1)-bslCambiante)*(hAux->GetBinContent(k+1)-bslCambiante)/nPtsBsl;
		                rmsCambiante = sqrt(rmsCambiante);
		                if(rmsCambiante < rmsMin)
		                {
		                        rmsMin = rmsCambiante;
		                        bslRmsMin = bslCambiante;
		                }
                        }
                }

                bsl[j] = bslRmsMin;
                rms[j] = rmsMin;
/*

                for(k = 0; k < nPtsBsl; k++) bslTot += hAux->GetBinContent(k+1)/nPtsBsl;
                for(k = 0; k < nPtsBsl; k++) bslTotF += hAux->GetBinContent(nSamples-k)/nPtsBsl;
                for(k = 0; k < nPtsBsl; k++) rmsTot += (hAux->GetBinContent(k+1)-bslTot)*(hAux->GetBinContent(k+1)-bslTot)/nPtsBsl;
                for(k = 0; k < nPtsBsl; k++) rmsTotF += (hAux->GetBinContent(nSamples-k)-bslTotF)*(hAux->GetBinContent(nSamples-k)-bslTotF)/nPtsBsl;
                rmsTot = sqrt(rmsTot);
                rmsTotF = sqrt(rmsTotF);

		if(bslTot > bslTotF) {bsl[j] = bslTot; rms[j] = rmsTot;}
		else {bsl[j] = bslTotF; rms[j] = rmsTotF;}
*/
                for(k = 0; k < nSamples; k++) hAux->SetBinContent(k+1,hAux->GetBinContent(k+1)-bsl[j]);

                trig = 5*rms[j];

                l = 0;

                while(l < nSamples && hAux->GetBinContent(l) > trig) l++;
                if(l == nSamples) t0[j] = -1;
                else t0[j] = l;

//                tmax[j] = hAux->GetBinCenter(hAux->GetMaximumBin());
//                high[j] = hAux->GetMaximum();
                max[j] = hAux->GetMinimum();

                for(k = t0[j]; k < t0[j] + tWin; k++) area[j] += hAux->GetBinContent(k)/amp;

                n[j] = area[j]/calSl[j];
		nTot += n[j];

  tMaxI=tFix+tWin; //El limite superior de integracion inicial
  
  hAux->GetXaxis()->SetRangeUser(tFix,tFix+tWin);
  maxbin=hAux->GetMaximumBin();
  hAux->GetXaxis()->SetRangeUser(0,nSamples);
  
  maxheigh=hAux->GetBinContent(maxbin);
  heighlimit=0.05*maxheigh;
  
  bin=maxbin;
  
  heigh=hAux->GetBinContent(bin);
  
  while(heigh>heighlimit && bin>tFix)
  {
    heigh=hAux->GetBinContent(bin);
    bin--;
  }
  
  tmin=bin+1;
  
  bin=maxbin;
  
  heigh=hAux->GetBinContent(bin);
  
  while(heigh>heighlimit && bin<(tFix+tWin))
  {
    heigh=hAux->GetBinContent(bin);
    bin++;
  }
  
  tmaxaux=bin-1;
  
  //if(tmin>tFix) tFix=tmin;
  
  //if(tmaxaux<tMaxI) tMaxI=tmaxaux;
  
  tWin=tMaxI-tFix;
  
  tFix_mod[j]=tFix;
  tWin_mod[j]=tWin;


                for(k = tFix; k < tFix + tWin; k++) areaFix[j] += hAux->GetBinContent(k)/amp;
                
                //Calculo del tiempo promedio en ventana de integracion
                for(k = tFix; k < tFix + tWin; k++) bin_time_mu += hAux->GetBinContent(k)/amp*k;
                mu[j]=bin_time_mu/areaFix[j]-tFix; //Se da en bines
                
                bin_time_mu=0;
                
                //Tiempo promedio del pulso total
                mu_tot[j]=hAux->GetMean();
                
                //Busqueda del numero de picos
                for(m=0; m<=(tWin)/3.; m++)
                {
                	media_ant=media;
                	media=0;
                	control_ant=control;
                  rms_peak=0;
            
                	for(p=1; p<=3; p++)
                		 media+=hAux->GetBinContent(tFix+m*3+p);                
            
                	media=media/3.;
            
                	if(media>media_ant)
                		control=1;
            
                	if(media<media_ant)
                		control=-1;
            
     	           //if(control-control_ant==-2 && hAux->GetBinContent(tFix+m*3)>0.25*(hAux->GetMaximum()))
                 if(control-control_ant==-2)
                		{
                     hAux->GetXaxis()->SetRangeUser(tFix+m*3-3,tFix+m*3+3);
                		 if(hAux->GetMaximum()>(5*rms[j])) n_peak[j]++;
                     hAux->GetXaxis()->SetRangeUser(0,nSamples);
                		}
                }
                
                media=0;
                control=0;
                
                //En todo el pulso
                for(m=0; m<=(nSamples)/3.; m++)
                {
                	media_ant=media;
                	media=0;
                	control_ant=control;
            
                	for(p=1; p<=3; p++)
                		 media+=hAux->GetBinContent(m*3+p);
            
                	media=media/3.;
   
            
                	if(media>media_ant)
                		control=1;
            
                	if(media<media_ant)
                		control=-1;
            
                	//if(control-control_ant==-2 && hAux->GetBinContent(tFix+m*3)>0.25*(hAux->GetMaximum()))
                  //if(control-control_ant==-2 && hAux->GetBinContent(m*3)>(5*rms[j]))
                  if(control-control_ant==-2)
                		{
                     hAux->GetXaxis()->SetRangeUser(m*3-3,m*3+3);
                		 if(hAux->GetMaximum()>(5*rms[j])) n_peak_tot[j]++;
                     hAux->GetXaxis()->SetRangeUser(0,nSamples);
                		}
                }
                
                //Fin busqueda del numero de picos
                
                //Calculo P1 y P2
                P1[j]=hAux->Integral(tFix+10,tFix+60)/hAux->Integral(tFix,tFix+60);
                P2[j]=hAux->Integral(tFix,tFix+5)/hAux->Integral(tFix,tFix+60);
                
                //Guardo el pulso
                if(j==0) pulse->Add(hAux);

                for(k = tFix; k < tFix + tWin; k++) if(hAux->GetBinContent(k)/amp > high[j]) {high[j] = hAux->GetBinContent(k)/amp; tmax[j] = 2*k;}

                nFix[j] = areaFix[j]/calSl[j];
                nTotFix += nFix[j];

		for(k = 0; k < nSamples; k++) areaTotWindow += hAux->GetBinContent(k)/amp;
		nTotWindow += areaTotWindow/calSl[j];
        }
  
	hAux->Delete();
}

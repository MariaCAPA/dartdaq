// Example Program for converting MIDAS format to ROOT format.
#include <TH1D.h>
#include <TH2D.h>
#include <TF1.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TCanvas.h>
#include <TPaveText.h>
#include <TMath.h>
#include <TStyle.h> //añadido

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include <vector>

#include "TRootanaEventLoop.hxx"
#include "TFile.h"
#include "TTree.h"
#include "TLine.h" //añadido

#include "TDartAnaManager.hxx"
#include "TEventProcessor.hxx"
#include "TEventMusic.hxx"

#include <TMidasEvent.h>
#include <TDataContainer.hxx>
#include <THistogramArrayBase.h>

#include "midasio.h"

const int VMEBUS_BOARDNO = 0;

int preAnaMusic(char *charNew, char *charConfig, int &tIni, int &tWin);

double calAnaMusic(TTree *pulses, const char *File_name); //Añadido para la calibracion

int main(int argc, char *argv[])
{
	int tFix, tWin;

	int pre = 0;

	pre = preAnaMusic(argv[1],argv[2],tFix,tWin);

	std::cout << tFix << "	" << tWin << std::endl;

	if(pre == -1) return -1;
	tFix=1215;
	tWin = 2500-tFix;
        //tFix = 700;

	int cal = 1;

        const char * file = argv[1];

	std::string nameNew = std::string(argv[1]);
        std::string nameConfig = std::string(argv[2]);
        std::string nameCal = std::string(argv[3]);

        std::string filenew = Form("../analyzed/%s.root",nameNew.c_str());
        std::string fileConfig = Form("%s.txt",nameConfig.c_str());
        std::string fileCal = Form("%s.txt",nameCal.c_str());

	char name[100];
	sprintf(name, "WF%02d", VMEBUS_BOARDNO);
	TMReaderInterface * reader = TMNewReader(file);
	if (reader->fError)
	{
		printf("Cannot open input file \"%s\"\n",file);
		delete reader;
		 return -1;
	}
	TMidasEvent event;

	int nCh, nSamples;
	int ev = 0;
	int evMax = 1000000;

        int nPtsBsl = 50;
//        int tWin = 500;
        int tTrigger;
        int amp = 1;
        int vSat = 1300;
//	int tFix;

        double pTrig;

        double * aqCh = new double [8];

	double aux;

        std::fstream fConfig (fileConfig.c_str(), std::ios::in);
        fConfig >> cal;
//        fConfig >> pTrig;
        fConfig >> amp;
	fConfig >> nSamples;
	fConfig >> aux;
	fConfig >> nCh;
        for(int i = 0; i < 8; i++) fConfig >> aqCh[i];

        int * channel = new int [8];

        int contCh = 0;

        for(int i = 0; i < 8; i++)
        {
                if(aqCh[i] == 1) {channel[contCh] = i; contCh++;}
        }

        if(contCh != nCh) {std::cout << "***************************** WARNING!!! ***************************** Incorrect number of channels in config file" << std::endl; return 0;}
        else
        {
                std::cout << "Number of channels acquired = " << nCh << std::endl;
                std::cout << "Channels #";
                for(int i = 0; i < nCh; i++) std::cout << channel[i] << ", ";
                std::cout << std::endl;
        }

        tTrigger = (100.0-pTrig)*nSamples/100.0;
        int tBslPrev;

        tBslPrev = tFix - 500;

	if(tFix < 500) tBslPrev = 0;

        double * calSl = new double[8];
        double * calIn = new double[8];
        double * acqCal = new double[8];

        if(cal == 0)
        {
                std::fstream fCal (fileCal.c_str(), std::ios::in);
                for(int i = 0; i < 8; i++) {fCal >> calSl[i]; fCal >> calIn[i]; fCal >> acqCal[i];}
                std::cout << "Run file, calibration:" << std::endl;
                for(int i = 0; i < 8; i++) if(acqCal[i] == 0) aqCh[i] = 0;
        }

        else
        {
                std::cout << "Calibration file" << std::endl;
                for(int i = 0; i < 8; i++) {calSl[i] = 1; calIn[i] = 0;}
//                tWin = 500;
//                tFix = tTrigger - 50;
        }

        for(int i = 0; i < 8; i++) {std::cout << i << " " << calSl[i] << "        " << calIn[i] << "    " << aqCh[i] << std::endl;}

        std::cout << "tFix = " << tFix << "     tWin = " << tWin << std::endl;
        std::cout << "nCh = " << nCh << std::endl;

        int sRate = 500*1e6;

        double tBin = 1.0*1e9/sRate;

        std::cout << "Input file: " << file << std::endl;
        std::cout << "Output file: " << filenew << std::endl;
        std::cout << "Config file: " << fileConfig << std::endl;
        std::cout << "Amplification: " << amp<< std::endl;
        std::cout << "PreTrigger: " << 100-pTrig << " %" << std::endl;
        std::cout << "tTrigger: " << tTrigger << std::endl;

        TH1F * hTot = new TH1F("hTot","hTot",nSamples,0,nSamples);

        TH1F * hAux = new TH1F("hAux","hAux",nSamples,0,nSamples);

        TH1F ** hP = new TH1F * [8];

        for(int i = 0; i < 8; i++) hP[i] = new TH1F(Form("hP_%d",i),Form("hP_%d",i),nSamples,0,nSamples);

        TFile * fnew = new TFile(filenew.c_str(), "recreate");
        TTree * t2 = new TTree("ta","");

        int acq0, acq1, acq2, acq3, acq4, acq5, acq6, acq7;

        double bsl0;
        double bsl1;
        double bsl2;
        double bsl3;
        double bsl4;
        double bsl5;
        double bsl6;
        double bsl7;

        double rms0;
        double rms1;
        double rms2;
        double rms3;
        double rms4;
        double rms5;
        double rms6;
        double rms7;

        double bslPrev0;
        double bslPrev1;
        double bslPrev2;
        double bslPrev3;
        double bslPrev4;
        double bslPrev5;
        double bslPrev6;
        double bslPrev7;

        double rmsPrev0;
        double rmsPrev1;
        double rmsPrev2;
        double rmsPrev3;
        double rmsPrev4;
        double rmsPrev5;
        double rmsPrev6;
        double rmsPrev7;

        double area0;
        double area1;
        double area2;
        double area3;
        double area4;
        double area5;
        double area6;
        double area7;

        double high0;
        double high1;
        double high2;
        double high3;
        double high4;
        double high5;
        double high6;
        double high7;

        double t00;
        double t01;
        double t02;
        double t03;
        double t04;
        double t05;
        double t06;
        double t07;

        double tmax0;
        double tmax1;
        double tmax2;
        double tmax3;
        double tmax4;
        double tmax5;
        double tmax6;
        double tmax7;

        double dt0;
        double dt1;
        double dt2;
        double dt3;
        double dt4;
        double dt5;
        double dt6;
        double dt7;

        double n0;
        double n1;
        double n2;
        double n3;
        double n4;
        double n5;
        double n6;
        double n7;

        double nFix0;
        double nFix1;
        double nFix2;
        double nFix3;
        double nFix4;
        double nFix5;
        double nFix6;
        double nFix7;

        double areaFix0;
        double areaFix1;
        double areaFix2;
        double areaFix3;
        double areaFix4;
        double areaFix5;
        double areaFix6;
        double areaFix7;

        double max0;
        double max1;
        double max2;
        double max3;
        double max4;
        double max5;
        double max6;
        double max7;

        double bslDif0;
        double bslDif1;
        double bslDif2;
        double bslDif3;
        double bslDif4;
        double bslDif5;
        double bslDif6;
        double bslDif7;

        double rmsDif0;
        double rmsDif1;
        double rmsDif2;
        double rmsDif3;
        double rmsDif4;
        double rmsDif5;
        double rmsDif6;
        double rmsDif7;
        
        //Tiempo promedio en ventana de integracion
        double mu0;
        double mu1;
        double mu2;
        double mu3;
        double mu4;
        double mu5;
        double mu6;
        double mu7;
        
        double mu_tot0;
        double mu_tot1;
        double mu_tot2;
        double mu_tot3;
        double mu_tot4;
        double mu_tot5;
        double mu_tot6;
        double mu_tot7;
        
        //Número de picos en ventana de integracion y en todo el pulso
        double n_peak0;
        double n_peak1;
        double n_peak2;
        double n_peak3;
        double n_peak4;
        double n_peak5;
        double n_peak6;
        double n_peak7;
        
        double n_peak_tot0;
        double n_peak_tot1;
        double n_peak_tot2;
        double n_peak_tot3;
        double n_peak_tot4;
        double n_peak_tot5;
        double n_peak_tot6;
        double n_peak_tot7;
        
        //P1 (10-60) y P2 (0-5)
        double P10;
        double P11;
        double P12;
        double P13;
        double P14;
        double P15;
        double P16;
        double P17;
        
        double P20;
        double P21;
        double P22;
        double P23;
        double P24;
        double P25;
        double P26;
        double P27;
        
        //Tiempos de integracion
        double tFix_mod0;
        double tFix_mod1;
        double tFix_mod2;
        double tFix_mod3;
        double tFix_mod4;
        double tFix_mod5;
        double tFix_mod6;
        double tFix_mod7;
        
        double tWin_mod0;
        double tWin_mod1;
        double tWin_mod2;
        double tWin_mod3;
        double tWin_mod4;
        double tWin_mod5;
        double tWin_mod6;
        double tWin_mod7;
        
        //FWHM
        double FWHM0;
        double FWHM1;
        double FWHM2;
        double FWHM3;
        double FWHM4;
        double FWHM5;
        double FWHM6;
        double FWHM7;
        
        //Pulsos
        TH1F *pulse0 = new TH1F("pulse0","pulse0",nSamples,0,nSamples);
        TH1F *pulse1 = new TH1F("pulse1","pulse1",nSamples,0,nSamples);
        TH1F *pulse2 = new TH1F("pulse2","pulse2",nSamples,0,nSamples);
        TH1F *pulse3 = new TH1F("pulse3","pulse3",nSamples,0,nSamples);
        TH1F *pulse4 = new TH1F("pulse4","pulse4",nSamples,0,nSamples);
        TH1F *pulse5 = new TH1F("pulse5","pulse5",nSamples,0,nSamples);
        TH1F *pulse6 = new TH1F("pulse6","pulse6",nSamples,0,nSamples);
        TH1F *pulse7 = new TH1F("pulse7","pulse7",nSamples,0,nSamples);


        double areaPreTrigger;

        double nT, nB, nFixT, nFixB, areaT, areaB, areaFixT, areaFixB;
        double nT2, nB2, nFixT2, nFixB2, areaT2, areaB2, areaFixT2, areaFixB2;

        int bad, sat;

        int * chBad = new int [8];

        double * bsl = new double [8];
        double * bslF = new double [8];
        double * bslX = new double [8];
        double * bslDif = new double [8];

        double maxTot;

        double * max = new double [8];

        double * bslPrev = new double [8];

        double * rms = new double [8];
        double * rmsF = new double [8];
        double * rmsX = new double [8];
        double * rmsDif = new double [8];

        double * rmsPrev = new double [8];

        double * t0 = new double [8];
        double * tmax = new double [8];        t2->Branch("pulse0","TH1F",&pulse0,32000,0);
        double * dt = new double [8];
        double * area = new double [8];
        double * areaFix = new double [8];
        double * n = new double [8];
        double * nFix = new double [8];
        double * high = new double [8];
        
        double *mu = new double [8]; //Tiempo promedio en ventana de integracion
        double *mu_tot = new double [8]; //Tiempo promedio del pulso completo
        double *n_peak = new double [8]; //Numero de picos
        double *n_peak_tot = new double [8]; //Numero de picos
        double *P1 = new double [8];
        double *P2 = new double [8];
        double *tFix_mod = new double [8];
        double *tWin_mod = new double [8];
        double *FWHM= new double [8];

	
        double * p = new double [8];

        double highTot;
        double areaP;
        double areaTot;
        double areaTotMax;
        double areaTotFix;
        double nTot;
        double nTotFix;
        double t0Tot;
        double tmaxTot;
        //double mu; //Para que no haya error con el nuevo Tiempo promedio definido
        //double mu2; //Para que no haya error con el nuevo Tiempo promedio definido
        double integ;

        double t0SofTot;

        double bslTot;
        double bslTotF;
        double bslTotPrev;

        double rmsTot;
        double rmsTotF;
        double rmsTotPrev;

        double timeFile;

        int acqT, acqB;
        double asyN, asyNFix, asyArea, asyAreaFix;

        t2->Branch("timeFile",&timeFile,"&timeFile/D");
        t2->Branch("amp",&amp,"&amp/I");
        t2->Branch("nSamples",&nSamples,"&nSamples/I");
        t2->Branch("nCh",&nCh,"&nCh/I");

        t2->Branch("asyN",&asyN,"&asyN/D");
        t2->Branch("asyNFix",&asyNFix,"&asyNFix/D");
        t2->Branch("asyArea",&asyArea,"&asyArea/D");
        t2->Branch("asyAreaFix",&asyAreaFix,"&asyAreaFix/D");

        t2->Branch("nT",&nT,"&nT/D");
        t2->Branch("nB",&nB,"&nB/D");
        t2->Branch("nFixT",&nFixT,"&nFixT/D");
        t2->Branch("nFixB",&nFixB,"&nFixB/D");
        t2->Branch("areaT",&areaT,"&areaT/D");
        t2->Branch("areaB",&areaB,"&areaB/D");
        t2->Branch("areaFixT",&areaFixT,"&areaFixT/D");
        t2->Branch("areaFixB",&areaFixB,"&areaFixB/D");

        t2->Branch("acq0",&acq0,"&acq0/I");
        t2->Branch("acq1",&acq1,"&acq1/I");
        t2->Branch("acq2",&acq2,"&acq2/I");
        t2->Branch("acq3",&acq3,"&acq3/I");
        t2->Branch("acq4",&acq4,"&acq4/I");
        t2->Branch("acq5",&acq5,"&acq5/I");
        t2->Branch("acq6",&acq6,"&acq6/I");
        t2->Branch("acq7",&acq7,"&acq7/I");

        t2->Branch("bsl0",&bsl0,"&bsl0/D");
        t2->Branch("bsl1",&bsl1,"&bsl1/D");
        t2->Branch("bsl2",&bsl2,"&bsl2/D");
        t2->Branch("bsl3",&bsl3,"&bsl3/D");
        t2->Branch("bsl4",&bsl4,"&bsl4/D");
        t2->Branch("bsl5",&bsl5,"&bsl5/D");
        t2->Branch("bsl6",&bsl6,"&bsl6/D");
        t2->Branch("bsl7",&bsl7,"&bsl7/D");

        t2->Branch("max0",&max0,"&max0/D");
        t2->Branch("max1",&max1,"&max1/D");
        t2->Branch("max2",&max2,"&max2/D");
        t2->Branch("max3",&max3,"&max3/D");
        t2->Branch("max4",&max4,"&max4/D");
        t2->Branch("max5",&max5,"&max5/D");
        t2->Branch("max6",&max6,"&max6/D");
        t2->Branch("max7",&max7,"&max7/D");

        t2->Branch("bslDif0",&bslDif0,"&bslDif0/D");
        t2->Branch("bslDif1",&bslDif1,"&bslDif1/D");
        t2->Branch("bslDif2",&bslDif2,"&bslDif2/D");
        t2->Branch("bslDif3",&bslDif3,"&bslDif3/D");
        t2->Branch("bslDif4",&bslDif4,"&bslDif4/D");
        t2->Branch("bslDif5",&bslDif5,"&bslDif5/D");
        t2->Branch("bslDif6",&bslDif6,"&bslDif6/D");
        t2->Branch("bslDif7",&bslDif7,"&bslDif7/D");

        t2->Branch("rmsDif0",&rmsDif0,"&rmsDif0/D");
        t2->Branch("rmsDif1",&rmsDif1,"&rmsDif1/D");
        t2->Branch("rmsDif2",&rmsDif2,"&rmsDif2/D");
        t2->Branch("rmsDif3",&rmsDif3,"&rmsDif3/D");
        t2->Branch("rmsDif4",&rmsDif4,"&rmsDif4/D");
        t2->Branch("rmsDif5",&rmsDif5,"&rmsDif5/D");
        t2->Branch("rmsDif6",&rmsDif6,"&rmsDif6/D");
        t2->Branch("rmsDif7",&rmsDif7,"&rmsDif7/D");

        t2->Branch("rms0",&rms0,"&rms0/D");
        t2->Branch("rms1",&rms1,"&rms1/D");
        t2->Branch("rms2",&rms2,"&rms2/D");
        t2->Branch("rms3",&rms3,"&rms3/D");
        t2->Branch("rms4",&rms4,"&rms4/D");
        t2->Branch("rms5",&rms5,"&rms5/D");
        t2->Branch("rms6",&rms6,"&rms6/D");
        t2->Branch("rms7",&rms7,"&rms7/D");

        t2->Branch("bslPrev0",&bslPrev0,"&bslPrev0/D");
        t2->Branch("bslPrev1",&bslPrev1,"&bslPrev1/D");
        t2->Branch("bslPrev2",&bslPrev2,"&bslPrev2/D");
        t2->Branch("bslPrev3",&bslPrev3,"&bslPrev3/D");
        t2->Branch("bslPrev4",&bslPrev4,"&bslPrev4/D");
        t2->Branch("bslPrev5",&bslPrev5,"&bslPrev5/D");
        t2->Branch("bslPrev6",&bslPrev6,"&bslPrev6/D");
        t2->Branch("bslPrev7",&bslPrev7,"&bslPrev7/D");

        t2->Branch("rmsPrev0",&rmsPrev0,"&rmsPrev0/D");
        t2->Branch("rmsPrev1",&rmsPrev1,"&rmsPrev1/D");
        t2->Branch("rmsPrev2",&rmsPrev2,"&rmsPrev2/D");
        t2->Branch("rmsPrev3",&rmsPrev3,"&rmsPrev3/D");
        t2->Branch("rmsPrev4",&rmsPrev4,"&rmsPrev4/D");
        t2->Branch("rmsPrev5",&rmsPrev5,"&rmsPrev5/D");
        t2->Branch("rmsPrev6",&rmsPrev6,"&rmsPrev6/D");
        t2->Branch("rmsPrev7",&rmsPrev7,"&rmsPrev7/D");

        t2->Branch("area0",&area0,"&area0/D");
        t2->Branch("area1",&area1,"&area1/D");
        t2->Branch("area2",&area2,"&area2/D");
        t2->Branch("area3",&area3,"&area3/D");
        t2->Branch("area4",&area4,"&area4/D");
        t2->Branch("area5",&area5,"&area5/D");
        t2->Branch("area6",&area6,"&area6/D");
        t2->Branch("area7",&area7,"&area7/D");

        t2->Branch("n0",&n0,"&n0/D");
        t2->Branch("n1",&n1,"&n1/D");
        t2->Branch("n2",&n2,"&n2/D");
        t2->Branch("n3",&n3,"&n3/D");
        t2->Branch("n4",&n4,"&n4/D");
        t2->Branch("n5",&n5,"&n5/D");
        t2->Branch("n6",&n6,"&n6/D");
        t2->Branch("n7",&n7,"&n7/D");

        t2->Branch("nFix0",&nFix0,"&nFix0/D");
        t2->Branch("nFix1",&nFix1,"&nFix1/D");
        t2->Branch("nFix2",&nFix2,"&nFix2/D");
        t2->Branch("nFix3",&nFix3,"&nFix3/D");
        t2->Branch("nFix4",&nFix4,"&nFix4/D");
        t2->Branch("nFix5",&nFix5,"&nFix5/D");
        t2->Branch("nFix6",&nFix6,"&nFix6/D");
        t2->Branch("nFix7",&nFix7,"&nFix7/D");

        t2->Branch("high0",&high0,"&high0/D");
        t2->Branch("high1",&high1,"&high1/D");
        t2->Branch("high2",&high2,"&high2/D");
        t2->Branch("high3",&high3,"&high3/D");
        t2->Branch("high4",&high4,"&high4/D");
        t2->Branch("high5",&high5,"&high5/D");
        t2->Branch("high6",&high6,"&high6/D");
        t2->Branch("high7",&high7,"&high7/D");

        t2->Branch("areaFix0",&areaFix0,"&areaFix0/D");
        t2->Branch("areaFix1",&areaFix1,"&areaFix1/D");
        t2->Branch("areaFix2",&areaFix2,"&areaFix2/D");
        t2->Branch("areaFix3",&areaFix3,"&areaFix3/D");
        t2->Branch("areaFix4",&areaFix4,"&areaFix4/D");
        t2->Branch("areaFix5",&areaFix5,"&areaFix5/D");
        t2->Branch("areaFix6",&areaFix6,"&areaFix6/D");
        t2->Branch("areaFix7",&areaFix7,"&areaFix7/D");

        t2->Branch("t00",&t00,"&t00/D");
        t2->Branch("t01",&t01,"&t01/D");
        t2->Branch("t02",&t02,"&t02/D");
        t2->Branch("t03",&t03,"&t03/D");
        t2->Branch("t04",&t04,"&t04/D");
        t2->Branch("t05",&t05,"&t05/D");
        t2->Branch("t06",&t06,"&t06/D");
        t2->Branch("t07",&t07,"&t07/D");

        t2->Branch("dt0",&dt0,"&dt0/D");
        t2->Branch("dt1",&dt1,"&dt1/D");
        t2->Branch("dt2",&dt2,"&dt2/D");
        t2->Branch("dt3",&dt3,"&dt3/D");
        t2->Branch("dt4",&dt4,"&dt4/D");
        t2->Branch("dt5",&dt5,"&dt5/D");
        t2->Branch("dt6",&dt6,"&dt6/D");
        t2->Branch("dt7",&dt7,"&dt7/D");

        t2->Branch("tmax0",&tmax0,"&tmax0/D");
        t2->Branch("tmax1",&tmax1,"&tmax1/D");
        t2->Branch("tmax2",&tmax2,"&tmax2/D");
        t2->Branch("tmax3",&tmax3,"&tmax3/D");
        t2->Branch("tmax4",&tmax4,"&tmax4/D");
        t2->Branch("tmax5",&tmax5,"&tmax5/D");
        t2->Branch("tmax6",&tmax6,"&tmax6/D");
        t2->Branch("tmax7",&tmax7,"&tmax7/D");
        
        //Tiempo promedio
        t2->Branch("mu0",&mu0,"&mu0/D");
        t2->Branch("mu1",&mu1,"&mu1/D");
        t2->Branch("mu2",&mu2,"&mu2/D");
        t2->Branch("mu3",&mu3,"&mu3/D");
        t2->Branch("mu4",&mu4,"&mu4/D");
        t2->Branch("mu5",&mu5,"&mu5/D");
        t2->Branch("mu6",&mu6,"&mu6/D");
        t2->Branch("mu7",&mu7,"&mu7/D");
        
        t2->Branch("mu_tot0",&mu_tot0,"&mu_tot0/D");
        t2->Branch("mu_tot1",&mu_tot1,"&mu_tot1/D");
        t2->Branch("mu_tot2",&mu_tot2,"&mu_tot2/D");
        t2->Branch("mu_tot3",&mu_tot3,"&mu_tot3/D");
        t2->Branch("mu_tot4",&mu_tot4,"&mu_tot4/D");
        t2->Branch("mu_tot5",&mu_tot5,"&mu_tot5/D");
        t2->Branch("mu_tot6",&mu_tot6,"&mu_tot6/D");
        t2->Branch("mu_tot7",&mu_tot7,"&mu_tot7/D");
        
        //Numero de picos
        t2->Branch("n_peak0",&n_peak0,"&n_peak0/D");
        t2->Branch("n_peak1",&n_peak1,"&n_peak1/D");
        t2->Branch("n_peak2",&n_peak2,"&n_peak2/D");
        t2->Branch("n_peak3",&n_peak3,"&n_peak3/D");
        t2->Branch("n_peak4",&n_peak4,"&n_peak4/D");
        t2->Branch("n_peak5",&n_peak5,"&n_peak5/D");
        t2->Branch("n_peak6",&n_peak6,"&n_peak6/D");
        t2->Branch("n_peak7",&n_peak7,"&n_peak7/D");
        
        t2->Branch("n_peak_tot0",&n_peak_tot0,"&n_peak_tot0/D");
        t2->Branch("n_peak_tot1",&n_peak_tot1,"&n_peak_tot1/D");
        t2->Branch("n_peak_tot2",&n_peak_tot2,"&n_peak_tot2/D");
        t2->Branch("n_peak_tot3",&n_peak_tot3,"&n_peak_tot3/D");
        t2->Branch("n_peak_tot4",&n_peak_tot4,"&n_peak_tot4/D");
        t2->Branch("n_peak_tot5",&n_peak_tot5,"&n_peak_tot5/D");
        t2->Branch("n_peak_tot6",&n_peak_tot6,"&n_peak_tot6/D");
        t2->Branch("n_peak_tot7",&n_peak_tot7,"&n_peak_tot7/D");
        
        //P1 y P2
        t2->Branch("P10",&P10,"&P10/D");
        t2->Branch("P11",&P11,"&P11/D");
        t2->Branch("P12",&P12,"&P12/D");
        t2->Branch("P13",&P13,"&P13/D");
        t2->Branch("P14",&P14,"&P14/D");
        t2->Branch("P15",&P15,"&P15/D");
        t2->Branch("P16",&P16,"&P16/D");
        t2->Branch("P17",&P17,"&P17/D");
        
        t2->Branch("P20",&P20,"&P20/D");
        t2->Branch("P21",&P21,"&P21/D");
        t2->Branch("P22",&P22,"&P22/D");
        t2->Branch("P23",&P23,"&P23/D");
        t2->Branch("P24",&P24,"&P24/D");
        t2->Branch("P25",&P25,"&P25/D");
        t2->Branch("P26",&P26,"&P26/D");
        t2->Branch("P27",&P27,"&P27/D");
        
        //Tiempos de integracion
        t2->Branch("tFix_mod0",&tFix_mod0,"&tFix_mod0/D");
        t2->Branch("tFix_mod1",&tFix_mod1,"&tFix_mod1/D");
        t2->Branch("tFix_mod2",&tFix_mod2,"&tFix_mod2/D");
        t2->Branch("tFix_mod3",&tFix_mod3,"&tFix_mod3/D");
        t2->Branch("tFix_mod4",&tFix_mod4,"&tFix_mod4/D");
        t2->Branch("tFix_mod5",&tFix_mod5,"&tFix_mod5/D");
        t2->Branch("tFix_mod6",&tFix_mod6,"&tFix_mod6/D");
        t2->Branch("tFix_mod7",&tFix_mod7,"&tFix_mod7/D");
        
        t2->Branch("tWin_mod0",&tWin_mod0,"&tWin_mod0/D");
        t2->Branch("tWin_mod1",&tWin_mod1,"&tWin_mod1/D");
        t2->Branch("tWin_mod2",&tWin_mod2,"&tWin_mod2/D");
        t2->Branch("tWin_mod3",&tWin_mod3,"&tWin_mod3/D");
        t2->Branch("tWin_mod4",&tWin_mod4,"&tWin_mod4/D");
        t2->Branch("tWin_mod5",&tWin_mod5,"&tWin_mod5/D");
        t2->Branch("tWin_mod6",&tWin_mod6,"&tWin_mod6/D");
        t2->Branch("tWin_mod7",&tWin_mod7,"&tWin_mod7/D");
        
        //FWHM
        t2->Branch("FWHM0",&FWHM0,"&FWHM0/D");
        t2->Branch("FWHM1",&FWHM1,"&FWHM1/D");
        t2->Branch("FWHM2",&FWHM2,"&FWHM2/D");
        t2->Branch("FWHM3",&FWHM3,"&FWHM3/D");
        t2->Branch("FWHM4",&FWHM4,"&FWHM4/D");
        t2->Branch("FWHM5",&FWHM5,"&FWHM5/D");
        t2->Branch("FWHM6",&FWHM6,"&FWHM6/D");
        t2->Branch("FWHM7",&FWHM7,"&FWHM7/D");
        
        //Pulsos
        t2->Branch("pulse0","TH1F",&pulse0,32000,0);
	t2->Branch("pulse1","TH1F",&pulse1,32000,0);
	t2->Branch("pulse2","TH1F",&pulse2,32000,0);
	t2->Branch("pulse3","TH1F",&pulse3,32000,0);
        t2->Branch("pulse4","TH1F",&pulse4,32000,0);
        t2->Branch("pulse5","TH1F",&pulse5,32000,0);
        t2->Branch("pulse6","TH1F",&pulse6,32000,0);
        t2->Branch("pulse7","TH1F",&pulse7,32000,0);	

        t2->Branch("bslTot",&bslTot,"bslTot/D");
        t2->Branch("bslTotF",&bslTotF,"bslTotF/D");
        t2->Branch("bslTotPrev",&bslTotPrev,"bslTotPrev/D");

        t2->Branch("rmsTot",&rmsTot,"rmsTot/D");
        t2->Branch("rmsTotF",&rmsTotF,"rmsTotF/D");
        t2->Branch("rmsTotPrev",&rmsTotPrev,"rmsTotPrev/D");

        t2->Branch("highTot",&highTot,"highTot/D");
        t2->Branch("areaTot",&areaTot,"areaTot/D");
        t2->Branch("areaP",&areaP,"areaP/D");
        t2->Branch("areaTotFix",&areaTotFix,"areaTotFix/D");
        t2->Branch("nTot",&nTot,"nTot/D");

        t2->Branch("nTotFix",&nTotFix,"nTotFix/D");
        t2->Branch("areaPreTrigger",&areaPreTrigger,"areaPreTrigger/D");
        t2->Branch("areaTotMax",&areaTotMax,"areaTotMax/D");

        t2->Branch("mu",&mu,"mu/D");
        t2->Branch("mu2",&mu2,"mu2/D");
        t2->Branch("t0Tot",&t0Tot,"t0Tot/D");
        t2->Branch("tmaxTot",&tmaxTot,"tmaxTot/D");

        t2->Branch("t0SofTot",&t0SofTot,"t0SofTot/D");
        t2->Branch("maxTot",&maxTot,"maxTot/D");

        t2->Branch("bad",&bad,"bad/I");
        t2->Branch("sat",&sat,"sat/I");

	double areaTotWindow, nTotWindow;

	t2->Branch("areaTotWindow",&areaTotWindow,"areaTotWindow/D");
        t2->Branch("nTotWindow",&nTotWindow,"nTotWindow/D");

        double trig;

        int i, j, k, l;

        for(i = 0; i < nCh; i++) p[i] = 1;

        contCh = 0;

	int tRmsMin;
	double rmsMin, bslCambiante, rmsCambiante;
	double bslRmsMin;

	int * config = new int[8];

	config[0] = cal;
        config[1] = pTrig;
        config[2] = amp;
        config[3] = tFix;
        config[4] = tWin;
        config[5] = nSamples;
        config[6] = nCh;
        config[7] = nPtsBsl;

	while (TMReadEvent(reader, &event) && ev < evMax)
	{
                if ((event.GetEventId() & 0xFFFF) == 0x8000) continue; // begin of run
	        TDataContainer dataContainer;
		// Set the midas event pointer in the physics event.
		dataContainer.SetMidasEventPointer(event);
		TV1730RawData * V1730 = dataContainer.GetEventData<TV1730RawData>(name);
		if (!V1730 || V1730->GetNChannels()==0)
		{
			printf("Didn't see bank %s or there are no channels \n", name);
      
      //
      //Hace la calibracion
      //
      
      double calibration = calAnaMusic(t2, file); //Añadido para la calibracion
      
      std::cout << "Calibracion phe: " << calibration << std::endl; //Añadido para la calibracion
      
      //
      //
      //
      
			return -1;
		}
	        timeFile = V1730->GetHeader().timeStampNs;
	        if(ev%100 == 0) {std::cout << ev << std::endl;}

                t0Tot = nSamples + 1;

		TEventMusic evMusic;
		evMusic.ProcessMusicEvent(V1730, config, aqCh, calSl, bslX, rmsX, area, areaFix, n, nFix, high, t0, tmax, areaTot, nTot, areaTotFix, nTotFix, areaTotWindow, nTotWindow, max, mu, mu_tot, n_peak, P1, P2, pulse0, pulse1, pulse2, pulse3, pulse4, pulse5, pulse6, pulse7, tFix_mod, tWin_mod, n_peak_tot, FWHM);

//		std::cout << areaTotWindow << std::endl;

                nT = 0;
                nB = 0;
                nFixT = 0;
                nFixB = 0;
                areaT = 0;
                areaB = 0;
                areaFixT = 0;
                areaFixB = 0;
                acqT = 0;
                acqB = 0;

/*
                if(acqT > 0)
                {
                        nT2 = 4.0/acqT*nT;
                        nFixT2 = 4.0/acqT*nFixT;
                        areaT2 = 4.0/acqT*areaT;
                        areaFixT2 = 4.0/acqT*areaFixT;
                }
                else
                {
                        nT2 = 0;
                        nFixT2 = 0;
                        areaT2 = 0;
                        areaFixT2 = 0;
                }

                if(acqB > 0)
                {
                        nB2 = 4.0/acqB*nB;
                        nFixB2 = 4.0/acqB*nFixB;
                        areaB2 = 4.0/acqB*areaB;
                        areaFixB2 = 4.0/acqB*areaFixB;
                }
                else
                {
                        nB2 = 0;
                        nFixB2 = 0;
                        areaB2 = 0;
                        areaFixB2 = 0;
                }

                asyN = (nT2-nB2)/(nT2+nB2);
                asyNFix = (nFixT2-nFixB2)/(nFixT2+nFixB2);
                asyArea = (areaT2-areaB2)/(areaT2+areaB2);
                asyAreaFix = (areaFixT2-areaFixB2)/(areaFixT2+areaFixB2);

                for(k = 0; k < nPtsBsl; k++) bslTot += hTot->GetBinContent(k+1)/nPtsBsl;
                for(k = 0; k < nPtsBsl; k++) bslTotF += hTot->GetBinContent(nSamples-k)/nPtsBsl;
                for(k = 0; k < nPtsBsl; k++) rmsTot += (hTot->GetBinContent(k+1)-bslTot)*(hTot->GetBinContent(k+1)-bslTot)/nPtsBsl;
                for(k = 0; k < nPtsBsl; k++) rmsTotF += (hTot->GetBinContent(nSamples-k)-bslTotF)*(hTot->GetBinContent(nSamples-k)-bslTotF)/nPtsBsl;
                rmsTot = sqrt(rmsTot);
                rmsTotF = sqrt(rmsTotF);

                for(k = 0; k < nPtsBsl; k++) bslTotPrev += hTot->GetBinContent(tBslPrev-k)/nPtsBsl;
                for(k = 0; k < nPtsBsl; k++) rmsTotPrev += (hTot->GetBinContent(tBslPrev-k)-bslTotPrev)*(hTot->GetBinContent(tBslPrev-k)-bslTotPrev)/nPtsBsl;
                rmsTotPrev = sqrt(rmsTotPrev);

                for(k = 0; k < nSamples; k++) hTot->SetBinContent(k+1,hTot->GetBinContent(k+1)-bslTotPrev);

                tmaxTot = hTot->GetBinCenter(hTot->GetMinimumBin());
                highTot = -1.0*hTot->GetMinimum();

                areaTot = 0;
                for(k = t0Tot; k < t0Tot + tWin; k++) areaTot += hTot->GetBinContent(k)/amp;

                areaTotFix = 0;
                for(k = tFix; k < tFix + tWin; k++) areaTotFix += hTot->GetBinContent(k)/amp;

                areaTotMax = 0;
                for(k = 0; k < nSamples; k++) areaTotMax += hTot->GetBinContent(k+1)/amp;

                integ = 0;
                for(k = tFix; k < nSamples; k++) {mu += hTot->GetBinContent(k)/amp*(hTot->GetBinCenter(k)-t0Tot); integ += hTot->GetBinContent(k);}
                mu = tBin*mu/integ/1000.0;

                trig = -5*rmsTotPrev;
                l = 1;
                while(l < nSamples && hTot->GetBinContent(l) > trig) l++;
                if(l == nSamples) t0SofTot = -1;
                else t0SofTot = l;

                maxTot = hTot->GetMaximum();

                areaPreTrigger = 0;
                for(k = 0; k < tFix; k++) areaPreTrigger += hTot->GetBinContent(k)/amp;
*/
                bsl0 = bslX[0];
                bsl1 = bslX[1];
                bsl2 = bslX[2];
                bsl3 = bslX[3];
                bsl4 = bslX[4];
                bsl5 = bslX[5];
                bsl6 = bslX[6];
                bsl7 = bslX[7];

                max0 = max[0];
                max1 = max[1];
                max2 = max[2];
                max3 = max[3];
                max4 = max[4];
                max5 = max[5];
                max6 = max[6];
                max7 = max[7];

                rms0 = rmsX[0];
                rms1 = rmsX[1];
                rms2 = rmsX[2];
                rms3 = rmsX[3];
                rms4 = rmsX[4];
                rms5 = rmsX[5];
                rms6 = rmsX[6];
                rms7 = rmsX[7];

                bslPrev0 = bslPrev[0];
                bslPrev1 = bslPrev[1];
                bslPrev2 = bslPrev[2];
                bslPrev3 = bslPrev[3];
                bslPrev4 = bslPrev[4];
                bslPrev5 = bslPrev[5];
                bslPrev6 = bslPrev[6];
                bslPrev7 = bslPrev[7];

                rmsPrev0 = rmsPrev[0];
                rmsPrev1 = rmsPrev[1];
                rmsPrev2 = rmsPrev[2];
                rmsPrev3 = rmsPrev[3];
                rmsPrev4 = rmsPrev[4];
                rmsPrev5 = rmsPrev[5];
                rmsPrev6 = rmsPrev[6];
                rmsPrev7 = rmsPrev[7];

                bslDif0 = bslDif[0];
                bslDif1 = bslDif[1];
                bslDif2 = bslDif[2];
                bslDif3 = bslDif[3];
                bslDif4 = bslDif[4];
                bslDif5 = bslDif[5];
                bslDif6 = bslDif[6];
                bslDif7 = bslDif[7];

                rmsDif0 = rmsDif[0];
                rmsDif1 = rmsDif[1];
                rmsDif2 = rmsDif[2];
                rmsDif3 = rmsDif[3];
                rmsDif4 = rmsDif[4];
                rmsDif5 = rmsDif[5];
                rmsDif6 = rmsDif[6];
                rmsDif7 = rmsDif[7];

                area0 = area[0];
                area1 = area[1];
                area2 = area[2];
                area3 = area[3];
                area4 = area[4];
                area5 = area[5];
                area6 = area[6];
                area7 = area[7];

                n0 = n[0];
                n1 = n[1];
                n2 = n[2];
                n3 = n[3];
                n4 = n[4];
                n5 = n[5];
                n6 = n[6];
                n7 = n[7];

                nFix0 = nFix[0];
                nFix1 = nFix[1];
                nFix2 = nFix[2];
                nFix3 = nFix[3];
                nFix4 = nFix[4];
                nFix5 = nFix[5];
                nFix6 = nFix[6];
                nFix7 = nFix[7];

                high0 = high[0];
                high1 = high[1];
                high2 = high[2];
                high3 = high[3];
                high4 = high[4];
                high5 = high[5];
                high6 = high[6];
                high7 = high[7];

                areaFix0 = areaFix[0];
                areaFix1 = areaFix[1];
                areaFix2 = areaFix[2];
                areaFix3 = areaFix[3];
                areaFix4 = areaFix[4];
                areaFix5 = areaFix[5];
                areaFix6 = areaFix[6];
                areaFix7 = areaFix[7];
                
                //Tiempo promedio
                mu0 = mu[0];
                mu1 = mu[1];
                mu2 = mu[2];
                mu3 = mu[3];
                mu4 = mu[4];
                mu5 = mu[5];
                mu6 = mu[6];
                mu7 = mu[7];
                
                mu_tot0 = mu_tot[0];
                mu_tot1 = mu_tot[1];
                mu_tot2 = mu_tot[2];
                mu_tot3 = mu_tot[3];
                mu_tot4 = mu_tot[4];
                mu_tot5 = mu_tot[5];
                mu_tot6 = mu_tot[6];
                mu_tot7 = mu_tot[7];
                
                //Numero de picos
                n_peak0 = n_peak[0];
                n_peak1 = n_peak[1];
                n_peak2 = n_peak[2];
                n_peak3 = n_peak[3];
                n_peak4 = n_peak[4];
                n_peak5 = n_peak[5];
                n_peak6 = n_peak[6];
                n_peak7 = n_peak[7];
                
                n_peak_tot0 = n_peak_tot[0];
                n_peak_tot1 = n_peak_tot[1];
                n_peak_tot2 = n_peak_tot[2];
                n_peak_tot3 = n_peak_tot[3];
                n_peak_tot4 = n_peak_tot[4];
                n_peak_tot5 = n_peak_tot[5];
                n_peak_tot6 = n_peak_tot[6];
                n_peak_tot7 = n_peak_tot[7];
                
                //P1 y P2
                P10 = P1[0];
                P11 = P1[1];
                P12 = P1[2];
                P13 = P1[3];
                P14 = P1[4];
                P15 = P1[5];
                P16 = P1[6];
                P17 = P1[7];
                
                P20 = P2[0];
                P21 = P2[1];
                P22 = P2[2];
                P23 = P2[3];
                P24 = P2[4];
                P25 = P2[5];
                P26 = P2[6];
                P27 = P2[7];
                
                //FWHM
                FWHM0 = FWHM[0];
                FWHM1 = FWHM[1];
                FWHM2 = FWHM[2];
                FWHM3 = FWHM[3];
                FWHM4 = FWHM[4];
                FWHM5 = FWHM[5];
                FWHM6 = FWHM[6];
                FWHM7 = FWHM[7];
                
                //Tiempos de integracion
                tFix_mod0 = tFix_mod[0];
                tFix_mod1 = tFix_mod[1];
                tFix_mod2 = tFix_mod[2];
                tFix_mod3 = tFix_mod[3];
                tFix_mod4 = tFix_mod[4];
                tFix_mod5 = tFix_mod[5];
                tFix_mod6 = tFix_mod[6];
                tFix_mod7 = tFix_mod[7];
                
                tWin_mod0 = tWin_mod[0];
                tWin_mod1 = tWin_mod[1];
                tWin_mod2 = tWin_mod[2];
                tWin_mod3 = tWin_mod[3];
                tWin_mod4 = tWin_mod[4];
                tWin_mod5 = tWin_mod[5];
                tWin_mod6 = tWin_mod[6];
                tWin_mod7 = tWin_mod[7];

/*
		for(k = 0; k < 8; k++) std::cout << areaFix[k] << "	";
		std::cout << std::endl;
*/
                t00 = t0[0];
                t01 = t0[1];
                t02 = t0[2];
                t03 = t0[3];
                t04 = t0[4];
                t05 = t0[5];
                t06 = t0[6];
                t07 = t0[7];

                dt0 = dt[0];
                dt1 = dt[1];
                dt2 = dt[2];
                dt3 = dt[3];
                dt4 = dt[4];
                dt5 = dt[5];
                dt6 = dt[6];
                dt7 = dt[7];

                tmax0 = tmax[0];
                tmax1 = tmax[1];
                tmax2 = tmax[2];
                tmax3 = tmax[3];
                tmax4 = tmax[4];
                tmax5 = tmax[5];
                tmax6 = tmax[6];
                tmax7 = tmax[7];

                t2->Fill();
		if(ev%100 == 0) t2->AutoSave();

		ev++;
	}

	t2->Print();
	t2->Write();

        fnew->Close();

	return 0;
}

int preAnaMusic(char *charNew, char *charConfig, int &tIni, int &tWin)
{

        int evMax = 20000;
        double areaMin, areaMax;

        int cal = 1;

        std::string nameNew = std::string(charNew);
        std::string nameConfig = std::string(charConfig);

        std::string filenew = Form("../analyzed/%s_pulses.root",nameNew.c_str());
        std::string fileConfig = Form("%s.txt",nameConfig.c_str());

        char name[100];
        sprintf(name, "WF%02d", VMEBUS_BOARDNO);
        TMReaderInterface * reader = TMNewReader(charNew);
        if (reader->fError)
        {
                printf("Cannot open input file \"%s\"\n",charNew);
                delete reader;
                 return -1;
        }
        TMidasEvent event;

        int nCh, nSamples;
        int ev = 0;

        int nPtsBsl = 10;
//        int tWin = 500;
        int tTrigger;
        int amp = 1;
        int vSat = 1300;
        int tFix;

        double pTrig;

        double * aqCh = new double [8];

	pTrig = 0;

	double thPulse = 5;

        std::fstream fConfig (fileConfig.c_str(), std::ios::in);
        fConfig >> cal;
//        fConfig >> pTrig;
        fConfig >> amp;
        fConfig >> nSamples;
	fConfig >> thPulse;
        fConfig >> nCh;
        for(int i = 0; i < 8; i++) fConfig >> aqCh[i];

        int * channel = new int [8];

        int contCh = 0;

        for(int i = 0; i < 8; i++)
        {
                if(aqCh[i] == 1) {channel[contCh] = i; contCh++;}
        }

        if(contCh != nCh) {std::cout << "***************************** WARNING!!! ***************************** Incorrect number of channels in config file" << std::endl; return -1;}
        else
        {
                std::cout << "Number of channels acquired = " << nCh << std::endl;
                std::cout << "Channels #";
                for(int i = 0; i < nCh; i++) std::cout << channel[i] << ", ";
                std::cout << std::endl;
        }

        tTrigger = (100.0-pTrig)*nSamples/100.0;
        int tBslPrev;

        tBslPrev = tFix - 500;

        if(tFix < 500) tBslPrev = 0;

        std::cout << "tFix = " << tFix << "     tWin = " << tWin << std::endl;
        std::cout << "nCh = " << nCh << std::endl;

        int sRate = 250*1e6;

        double tBin = 1.0*1e9/sRate;
/*
        std::cout << "Input file: " << file << std::endl;
        std::cout << "Output file: " << filenew << std::endl;
        std::cout << "Config file: " << fileConfig << std::endl;
        std::cout << "Amplification: " << amp<< std::endl;
        std::cout << "PreTrigger: " << 100-pTrig << " %" << std::endl;
        std::cout << "tTrigger: " << tTrigger << std::endl;
*/
        TH1F * hTot = new TH1F("hTot","hTot",nSamples,0,nSamples);

        TH1F * hAux = new TH1F("hAux","hAux",nSamples,0,nSamples);

        TH1F ** hP = new TH1F * [nCh];
        TH1F ** hP2 = new TH1F * [nCh];

        for(int i = 0; i < nCh; i++) hP[i] = new TH1F(Form("hP_%d",i),Form("hP_%d",i),nSamples,0,nSamples);
        for(int i = 0; i < nCh; i++) hP2[i] = new TH1F(Form("hP2_%d",i),Form("hP2_%d",i),nSamples,0,nSamples);

        TH1F * hPTot = new TH1F("hPTot","hPTot",nSamples,0,nSamples);
        TH1F * hPTot2 = new TH1F("hPTot2","hPTot2",nSamples,0,2*nSamples);

        TH1F * hArea = new TH1F("hArea","hArea",10000,-1e5,1e5);
        TH1F * hAreaTot = new TH1F("hAreaTot","hAreaTot",10000,-1e5,1e5);

        int i,j;

        double area;
        double bsl;

        double trig;

        contCh = 0;

        while (TMReadEvent(reader, &event) && ev < evMax)
        {
                if ((event.GetEventId() & 0xFFFF) == 0x8000) continue; // begin of run
                TDataContainer dataContainer;
                // Set the midas event pointer in the physics event.
                dataContainer.SetMidasEventPointer(event);
                TV1730RawData * V1730 = dataContainer.GetEventData<TV1730RawData>(name);
                if (!V1730 || V1730->GetNChannels()==0)
                {
                        printf("Didn't see bank %s or there are no channels \n", name);
                        return -1;
                }

                if(ev%100 == 0) {std::cout << ev << std::endl;}

                area = 0;

                for(j = 0; j < 8; j++)
                {
                        if(aqCh[j] == 1)
                        {
                                TV1730RawChannel channelData = V1730->GetChannelData(j);
                                nSamples = channelData.GetNSamples();
                                //for(i = 0; i < nSamples; i++) hP[j]->SetBinContent(i+1,-1.0*channelData.GetADCSample(i));
                                for(i = 0; i < nSamples; i++) hP[j]->SetBinContent(i+1,1.0*channelData.GetADCSample(i));
                                contCh++;
                        }
                }

                for(j = 0; j < 8; j++) if(aqCh[j] == 1) for(i = 2; i <= nSamples; i++) hP[j]->SetBinContent(i,hP[j]->GetBinContent(i-1)+0.5*(hP[j]->GetBinContent(i)-hP[j]->GetBinContent(i-1)));

                for(j = 0; j < 8; j++)
                {
                        if(aqCh[j] == 1)
                        {
                                bsl = 0;
                                for(i = 0; i < nPtsBsl; i++) bsl += hP[j]->GetBinContent(i+1)/nPtsBsl;
//                                for(i = 0; i < nSamples; i++) hP[j]->SetBinContent(i+1,hP[j]->GetBinContent(i+1)-bsl);
                                for(i = nPtsBsl; i < nSamples; i++) {area += hP[j]->GetBinContent(i+1)-bsl;}
                        }
                }

//              area *= -1;

                hAreaTot->Fill(area);

                ev++;
        }

        areaMin = 0.5*hAreaTot->GetBinCenter(hAreaTot->GetMaximumBin());
        areaMax = 1.5*hAreaTot->GetBinCenter(hAreaTot->GetMaximumBin());

        areaMin = 0;
        areaMax = 1e9;//0*areaMax;

        hAreaTot->Reset();

        ev = 0;

        while (TMReadEvent(reader, &event) && ev < evMax)
        {
                if ((event.GetEventId() & 0xFFFF) == 0x8000) continue; // begin of run
                TDataContainer dataContainer;
                // Set the midas event pointer in the physics event.
                dataContainer.SetMidasEventPointer(event);
                TV1730RawData * V1730 = dataContainer.GetEventData<TV1730RawData>(name);
                if (!V1730 || V1730->GetNChannels()==0)
                {
                        printf("Didn't see bank %s or there are no channels \n", name);
                        return -1;
                }

                if(ev%100 == 0) {std::cout << ev << std::endl;}

                area = 0;

                for(j = 0; j < 8; j++)
                {
                        if(aqCh[j] == 1)
                        {
                                TV1730RawChannel channelData = V1730->GetChannelData(j);
                                nSamples = channelData.GetNSamples();
                                //for(i = 0; i < nSamples; i++) hP[j]->SetBinContent(i+1,-1.0*channelData.GetADCSample(i));
                                for(i = 0; i < nSamples; i++) hP[j]->SetBinContent(i+1,1.0*channelData.GetADCSample(i));
                                contCh++;
                        }
                }

                for(j = 0; j < 8; j++) if(aqCh[j] == 1) for(i = 2; i <= nSamples; i++) hP[j]->SetBinContent(i,hP[j]->GetBinContent(i-1)+0.5*(hP[j]->GetBinContent(i)-hP[j]->GetBinContent(i-1)));

                for(j = 0; j < 8; j++)
                {
                        if(aqCh[j] == 1)
                        {
                                bsl = 0;
                                for(i = 0; i < nPtsBsl; i++) bsl += hP[j]->GetBinContent(i+1)/nPtsBsl;
//                                for(i = 0; i < nSamples; i++) hP[j]->SetBinContent(i+1,hP[j]->GetBinContent(i+1)-bsl);
                                for(i = nPtsBsl; i < nSamples; i++) area += hP[j]->GetBinContent(i+1)-bsl;
                        }
                }

//              area *= -1;

                if(area > areaMin && area < areaMax) {hArea->Fill(area); for(j = 0; j < nCh; j++) if(aqCh[j] == 1) hP2[j]->Add(hP[j]);}

                hAreaTot->Fill(area);

                ev++;
        }

        double bslTot = 0;

        for(int i = 0; i < nCh; i++) hPTot->Add(hP2[i]);

        for(int i = nSamples-150; i < nSamples-50; i++) bslTot += hPTot->GetBinContent(i+1)/100;

//        bslTot = hPTot->GetBinContent(1);

        for(i = 0; i < nSamples; i++) hPTot2->SetBinContent(i+1,(hPTot->GetBinContent(i+1)-bslTot));

        int max = hPTot2->GetMaximum();

        for(i = 0; i < nSamples; i++) hPTot2->SetBinContent(i+1,hPTot2->GetBinContent(i+1)/max);
//        for(i = 0; i < nSamples; i++) if(hPTot2->GetBinContent(i+1) < 0) hPTot2->SetBinContent(i+1,0);

	tIni = hPTot2->GetMaximumBin();
	int tFin = hPTot2->GetMaximumBin();
	tWin = 0;

	while(hPTot2->GetBinContent(tIni) > thPulse/100 && tIni > 0) tIni--;
        while(hPTot2->GetBinContent(tFin) > thPulse/100 && tFin < nSamples) tFin++;
	tWin = tFin-tIni;

	std::cout << thPulse/100 << std::endl;
	std::cout << tIni << "	" << tFin << "	" << tWin << std::endl;

        TCanvas *cPTot = new TCanvas("cPTot","multipads");
//        cPTot->SetLogy();
        hPTot2->SetTitle("Averaged Pulse and integration window");
        hPTot2->GetXaxis()->SetTitle("time (ns)");
        hPTot2->GetYaxis()->SetTitle("amplitude (arb. units)");
        hPTot2->Draw();
	TLine * lIni = new TLine(hPTot2->GetBinCenter(tIni),0,hPTot2->GetBinCenter(tIni),1);
	lIni->SetLineColor(kRed);
	lIni->Draw();
        TLine * lFin = new TLine(hPTot2->GetBinCenter(tFin),0,hPTot2->GetBinCenter(tFin),1);
        lFin->SetLineColor(kRed);
        lFin->Draw();

        TFile * foutroot = new TFile(Form("%s",filenew.c_str()),"recreate");
                hPTot2->Write();
                hPTot->Write();
                cPTot->Write();
                hArea->Write();
                hAreaTot->Write();
        foutroot->Close();

	return 1;

}


double calAnaMusic (TTree *pulses, const char *File_name) //Añadido para la calibracion
{
    //Definimos propiedades histograma de area

    TH1F *area_aux = new TH1F("area_aux","area_aux",10000,100,50);

    pulses->Draw("areaFix0 >> area_aux","high0>0","");
    
    gStyle->SetOptStat(0);

    int area_max = area_aux->GetXaxis()->GetXmax();
    int area_min = area_aux->GetXaxis()->GetXmin();
    int area_nbins = 500;

    int area_bin_size = (area_max-area_min)/10000.;

    int area_final_bin = area_aux->FindLastBinAbove(1);
    int area_initial_bin = area_aux->FindFirstBinAbove(1);

    area_max = area_final_bin*area_bin_size+area_min;
    area_min = area_initial_bin*area_bin_size+area_min;

    //Definimos propiedades histograma de altura
    TH1F *heigh_aux = new TH1F("heigh_aux","heigh_aux",10000,100,50);

    pulses->Draw("high0 >> heigh_aux","high0>0","");

    int heigh_max = heigh_aux->GetXaxis()->GetXmax();
    int heigh_min = heigh_aux->GetXaxis()->GetXmin();
    int heigh_nbins = 300;

    double heigh_bin_size = (heigh_max-heigh_min)/10000.;

    int heigh_final_bin = heigh_aux->FindLastBinAbove(1);
    int heigh_initial_bin = heigh_aux->FindFirstBinAbove(1);

    heigh_max = heigh_final_bin*heigh_bin_size+heigh_min;
    heigh_min = heigh_initial_bin*heigh_bin_size+heigh_min;


    TH1F *area = new TH1F("area", "Area",area_nbins,area_min,area_max);
    TH1F *area_no_cuts = new TH1F("area_no_cuts", "Area cuts",area_nbins,area_min,area_max);
    TH1F *heigh = new TH1F("heigh", "Heigh",heigh_nbins,0,heigh_max);
    TH1F *rms = new TH1F("rms", "Rms",90,0,100);

    pulses->Draw("areaFix0 >> area","high0>0 && n_peak0==1","");
    pulses->Draw("areaFix0 >> area_no_cuts","high0>0","");
    pulses->Draw("high0 >> heigh","high0>0","");
    pulses->Draw("rms0 >> rms","","");

    std::cout << "Mean area: " << area->GetMean() << std::endl;
    std::cout << "Mean heigh: " << heigh->GetMean() << std::endl;
    std::cout << "Mean rms: " << rms->GetMean() << std::endl;

    TCanvas *c1 = new TCanvas("c1","The Graph example",10,10,700,500);

    int i, j, k=0;
    int heigh_cut[3];

    double media=0, control=1, media_ant=0, control_ant=1;

    //heigh->GetXaxis()->SetRangeUser(0,500);

    for(i=0; i<3; i++)
    	heigh_cut[i]=0;
     
    //Suavizamos el histograma de altura
    //for(i = 2; i <= heigh_nbins; i++) heigh->SetBinContent(i,heigh->GetBinContent(i-1)+0.5*(heigh->GetBinContent(i)-heigh->GetBinContent(i-1)));

    for(i=0; i<heigh_nbins/3.; i++)
    {
    	media_ant=media;
    	media=0;
    	control_ant=control;

    	for(j=1; j<=3; j++)
    		 media+=heigh->GetBinContent(i*3+j);

    	media=media/3.;

    	if(media>media_ant)
    		control=1;

    	if(media<media_ant)
    		control=-1;

    	if(control-control_ant==2 && k<3)
     //if(control!=control_ant && control==1 && k<3)
    		{
    		 heigh_cut[k]=((i-1)*3+1.5)*(double(heigh_max)/double(heigh_nbins)); //Se pasa de bin a high0
    		 k++;
    		}
    }
    
    //Posicionamiento mas fino de los limites de 1 phe (buscando el minimo en el intervalo entre el limite inferior/superior y el siguiente al 30%)
    /*heigh->GetXaxis()->SetRangeUser(heigh_cut[0],heigh_cut[0]+(heigh_cut[1]-heigh_cut[0])*0.4);
    
    heigh_cut[0]=heigh->GetMinimumBin()*(heigh_max/heigh_nbins);
    
    heigh->GetXaxis()->SetRangeUser(0,heigh_max);
    
    
    heigh->GetXaxis()->SetRangeUser(heigh_cut[1],heigh_cut[1]+(heigh_cut[2]-heigh_cut[1])*0.4);
    
    heigh_cut[1]=(heigh->GetMinimumBin())*(double(heigh_max)/double(heigh_nbins));
    
    cout << double(double(heigh_max)/double(heigh_nbins)) << endl;
    
    heigh->GetXaxis()->SetRangeUser(0,heigh_max);*/
    

    if(heigh_cut[2]-heigh_cut[1]>1.2*(heigh_cut[1]-heigh_cut[0]) || heigh_cut[2]-heigh_cut[1]<0.6*(heigh_cut[1]-heigh_cut[0])) //Ajuste por si se va mucho el limite superior de 2 phe
	      heigh_cut[2]=heigh_cut[1]+heigh_cut[1]-heigh_cut[0];

    if(heigh->GetBinContent(1)==0)
    {
      heigh_cut[2]=heigh_cut[1];
      heigh_cut[1]=heigh_cut[0];
      heigh_cut[0]=0;
    }

    heigh->Draw();

    TLine * line;

    for(k=0; k<3; k++)
    {
    	line = new TLine(heigh_cut[k],0,heigh_cut[k],heigh->GetBinContent(heigh->GetMaximumBin())); //Coordenadas de la línea
    	line->SetLineWidth(2.); //Tamaño de la línea
    	line->SetLineColor(kRed); //Color de la línea
    	line->Draw("same"); //Para dibujar la línea
    }
    
    //c1->SaveAs(Form("heigh_cuts_%s.png",File_name));
    
    //Calibración en altura (usamos corte n_peak0==1)
    TCanvas *c4 = new TCanvas("c4","The Graph example",10,10,700,500);
    
    pulses->Draw("high0 >> heigh","high0>0 && n_peak0==1","");
    
    TGraphErrors *heigh_cal = new TGraphErrors();
    TF1 *heigh_cal_fit = new TF1("heigh_cal_fit","[0]+[1]*x");
    
    heigh->GetXaxis()->SetRangeUser(heigh_cut[0],heigh_cut[1]);
    heigh_cal->SetPoint(0,1.,heigh->GetMean());
    heigh_cal->SetPointError(0,0.,heigh->GetMeanError());
    
    heigh->GetXaxis()->SetRangeUser(heigh_cut[1],heigh_cut[2]);
    heigh_cal->SetPoint(1,2.,heigh->GetMean());
    heigh_cal->SetPointError(1,0.,heigh->GetMeanError());
    
    heigh_cal->SetTitle("Heigh calibration");
    heigh_cal->GetXaxis()->SetTitle("nphe");
    heigh_cal->GetYaxis()->SetTitle("Heigh (a.u)");
    
    heigh_cal->GetXaxis()->SetTitleSize(0.04);
    heigh_cal->GetYaxis()->SetTitleSize(0.04);
    heigh_cal->SetMarkerStyle(8);
    
    heigh_cal->Draw("AP");
    
    TPaveText *pv1 = new TPaveText(0.2,0.65,0.47,0.85,"NDC"); //Coordenadas del texto (x0,y0,x1,y1)
    pv1->SetFillColor(0); //Color del fondo
    pv1->SetTextSize(.04); //Tamaño del texto
    pv1->SetTextColor(kBlack); //Color del texto
    pv1->SetMargin(0.01); //Margen del texto
    
    heigh_cal->Fit(heigh_cal_fit);
    
    pv1->AddText("Linear fit");
    pv1->AddText(Form("n = %f +- %f",heigh_cal_fit->GetParameter(0),heigh_cal_fit->GetParError(0)));
    pv1->AddText(Form("m = %f +- %f",heigh_cal_fit->GetParameter(1),heigh_cal_fit->GetParError(1)));
    
    pv1->Draw("same");
    
    //c4->SaveAs(Form("heigh_calibration_%s.png",File_name));

    //Distribuciones phe en area
    TH1F * one_phe = new TH1F("one_phe", "1 phe",area_nbins,area_min,area_max);
    TH1F * two_phe = new TH1F("two_phe", "2 phe",area_nbins,area_min,area_max);
    TH1F * noise = new TH1F("noise", "noise",area_nbins,area_min,area_max);
    
    one_phe->SetLineColor(kRed);
    two_phe->SetLineColor(kGreen);
    noise->SetLineColor(kBlack);

    pulses->Draw("areaFix0>>one_phe",Form("high0>%d && high0<%d && n_peak0==1",heigh_cut[0],heigh_cut[1]));
    pulses->Draw("areaFix0>>two_phe",Form("high0>%d && high0<%d && n_peak0==1",heigh_cut[1],heigh_cut[2]));
    pulses->Draw("areaFix0>>noise",Form("high0<%d && n_peak0==1",heigh_cut[0]));
    
    TCanvas *c2 = new TCanvas("c2","The Graph example",10,10,700,500);

    area->Draw();
    one_phe->Draw("same");
    two_phe->Draw("same");
    noise->Draw("same");

    TLegend *legend = new TLegend(0.6,0.65,0.87,0.85); //Tamaño de la leyenda (x0,y0,x1,y1)
    legend->SetTextSize(0.04); //Tamaño del texto
    legend->SetFillColor(kWhite); //Fondo de la leyenda

    legend->AddEntry(noise,"Noise","l");
    legend->AddEntry(one_phe,"1 photoelectron","l");
    legend->AddEntry(two_phe,"2 photoelectrons","l");

    legend->Draw("same");
    
    //c2->SaveAs(Form("area_phe_%s.png",File_name));
    
    //Area spectrum comparison
    TCanvas *c5 = new TCanvas("c5","The Graph example",10,10,700,500);
    
    area_no_cuts->SetLineColor(kBlack);
    area->SetLineColor(kRed);
    
    area_no_cuts->Draw("HIST");
    area->Draw("HIST && same");
    
    TLegend *legend2 = new TLegend(0.47,0.72,0.87,0.85); //Tamaño de la leyenda (x0,y0,x1,y1)
    legend2->SetTextSize(0.04); //Tamaño del texto
    legend2->SetFillColor(kWhite); //Fondo de la leyenda

    legend2->AddEntry(area_no_cuts,"Area spectrum","l");
    legend2->AddEntry(area,"Area spectrum (n_peak=1)","l");
    
    legend2->Draw("same");
    
    //c5->SaveAs(Form("area_cuts_comparison_%s.png",File_name));
    
    TCanvas *c3 = new TCanvas("c3","The Graph example",10,10,700,500);
    
    //Calibracion en area
    TGraphErrors *area_cal = new TGraphErrors();
    TF1 *area_cal_fit = new TF1("area_cal_fit","[0]+[1]*x");
    
    area_cal->SetTitle("Area calibration");
    area_cal->GetXaxis()->SetTitle("nphe");
    area_cal->GetYaxis()->SetTitle("Area (a.u)");
    
    area_cal->GetXaxis()->SetTitleSize(0.04);
    area_cal->GetYaxis()->SetTitleSize(0.04);
    area_cal->SetMarkerStyle(8);
    
    area_cal->SetPoint(0,1.,one_phe->GetMean());
    area_cal->SetPointError(0,0.,one_phe->GetMeanError());
    
    area_cal->SetPoint(1,2.,two_phe->GetMean());
    area_cal->SetPointError(1,0.,two_phe->GetMeanError());
    
    area_cal->Draw("AP");
    
    TPaveText *pv = new TPaveText(0.2,0.65,0.47,0.85,"NDC"); //Coordenadas del texto (x0,y0,x1,y1)
    pv->SetFillColor(0); //Color del fondo
    pv->SetTextSize(.04); //Tamaño del texto
    pv->SetTextColor(kBlack); //Color del texto
    pv->SetMargin(0.01); //Margen del texto
    
    area_cal->Fit(area_cal_fit);
    
    pv->AddText("Linear fit");
    pv->AddText(Form("n = %f +- %f",area_cal_fit->GetParameter(0),area_cal_fit->GetParError(0)));
    pv->AddText(Form("m = %f +- %f",area_cal_fit->GetParameter(1),area_cal_fit->GetParError(1)));
    
    pv->Draw("same");
    
    //c3->SaveAs(Form("area_calibration_%s.png",File_name));

    double phe_calibration, phe_calibration_error;

    phe_calibration=area_cal_fit->GetParameter(1);
    phe_calibration_error=area_cal_fit->GetParError(1);
    
    FILE *f;
    
    f=fopen("calibration.txt","a");
    
    fprintf(f,"%s %f %f %f %f \n", File_name, phe_calibration, phe_calibration_error, area_cal_fit->GetParameter(0), area_cal_fit->GetParError(0));
    
    fclose(f);

    return phe_calibration;
}



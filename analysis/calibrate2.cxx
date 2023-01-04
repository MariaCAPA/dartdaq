// Example Program for converting MIDAS format to ROOT format.
#include <TH1D.h>
#include <TH2D.h>
#include <TGraph.h>
#include <TCanvas.h>
#include <TMath.h>

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include <vector>

#include "TRootanaEventLoop.hxx"
#include "TFile.h"
#include "TLine.h"
#include "TTree.h"

#include "TDartAnaManager.hxx"
#include "TEventProcessor.hxx"

#include <TMidasEvent.h>
#include <TDataContainer.hxx>
#include <THistogramArrayBase.h>

#include "midasio.h"

const int VMEBUS_BOARDNO = 0;

int main(int argc, char *argv[])
{
	double yCut = 0.999;

	int evMax = 2000;
        double areaMin, areaMax;

	int cal = 1;

        const char * file = argv[1];

	std::string nameNew = std::string(argv[1]);
        std::string nameConfig = std::string(argv[2]);

        std::string filenew = Form("%s_cal.root",nameNew.c_str());
        std::string fileConfig = Form("%s.txt",nameConfig.c_str());

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

        int nPtsBsl = 50;
        int tWin = 500;
        int tTrigger;
        int amp = 1;
        int vSat = 1300;
	int tFix;

        double pTrig;

        double * aqCh = new double [8];

        std::fstream fConfig (fileConfig.c_str(), std::ios::in);
        fConfig >> cal;
        fConfig >> pTrig;
        fConfig >> amp;
	fConfig >> tFix;
	fConfig >> tWin;
	fConfig >> nSamples;
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

        TH1F ** hP = new TH1F * [nCh];
        TH1F ** hP2 = new TH1F * [nCh];

        for(int i = 0; i < nCh; i++) hP[i] = new TH1F(Form("hP_%d",i),Form("hP_%d",i),nSamples,0,nSamples);
        for(int i = 0; i < nCh; i++) hP2[i] = new TH1F(Form("hP2_%d",i),Form("hP2_%d",i),nSamples,0,nSamples);

        TH1F * hPTot = new TH1F("hPTot","hPTot",nSamples,0,nSamples);
        TH1F * hPTot2 = new TH1F("hPTot2","hPTot2",nSamples,0,2*nSamples);

        TH1F * hArea = new TH1F("hArea","hArea",210,-1000,20000);
        TH1F * hAreaTot = new TH1F("hAreaTot","hAreaTot",210,-1000,20000);

        TH2F * hAreaHigh = new TH2F("hAreaHigh","hAreaHigh",210,-1000,20000,100,0,1000);

        int i,j;

        double area;
        double bsl;

        double trig;

        contCh = 0;

	double high;
	int tmax;

        int a = 0;

        int tRmsMin;
        double rmsMin, bslCambiante, rmsCambiante;
        double bslRmsMin;

        int l, k;

        while (TMReadEvent(reader, &event) && a == 0)
        {
                if ((event.GetEventId() & 0xFFFF) == 0x8000) continue; // begin of run
                TDataContainer dataContainer;
                // Set the midas event pointer in the physics event.
                dataContainer.SetMidasEventPointer(event);
                TV1730RawData * V1730 = dataContainer.GetEventData<TV1730RawData>(name);
                if (!V1730 || V1730->GetNChannels()==0)
                {
                        printf("Didn't see bank %s or there are no channels \n", name);
                        a = 1;
                }

		if(a == 0)
		{

	                if(ev%100 == 0) {std::cout << ev << std::endl;}

	                area = 0;

	                for(j = 0; j < 8; j++)
	                {
	                        if(aqCh[j] == 1)
	                        {
	                                TV1730RawChannel channelData = V1730->GetChannelData(j);
	                                nSamples = channelData.GetNSamples();
	                                for(i = 0; i < nSamples; i++) hP[j]->SetBinContent(i+1,-1.0*channelData.GetADCSample(i)/amp);
	                                contCh++;
	                        }
	                }

	                for(j = 0; j < 8; j++) if(aqCh[j] == 1) for(i = 2; i <= nSamples; i++) hP[j]->SetBinContent(i,hP[j]->GetBinContent(i-1)+0.5*(hP[j]->GetBinContent(i)-hP[j]->GetBinContent(i-1)));

	                for(j = 0; j < 8; j++)
	                {
	                        if(aqCh[j] == 1)
	                        {

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

	                                                for(k = l; k < l + nPtsBsl; k++) bslCambiante += hP[j]->GetBinContent(k+1)/nPtsBsl;
	                                                for(k = l; k < l + nPtsBsl; k++) rmsCambiante += (hP[j]->GetBinContent(k+1)-bslCambiante)*(hP[j]->GetBinContent(k+1)-bslCambiante)/nPtsBsl;
	                                                rmsCambiante = sqrt(rmsCambiante);
	                                                if(rmsCambiante < rmsMin)
	                                                {
	                                                        rmsMin = rmsCambiante;
	                                                        tRmsMin = l;
	                                                        bslRmsMin = bslCambiante;
	                                                }
	                                        }
	                                }

	                                bsl = bslRmsMin;

	                                for(i = 0; i < nSamples; i++) hP[j]->SetBinContent(i+1,hP[j]->GetBinContent(i+1)-bsl);
//	                                for(i = tMin/2; i < tMax/2; i++) area += hP[j]->GetBinContent(i+1);
	                        }
	                }


			high = 0;
			tmax = 0;
			for(i = tFix; i < tFix+tWin; i++) if(hP[0]->GetBinContent(i) > high) {high = hP[0]->GetBinContent(i); tmax = i;}
			area = 0;
			i = tmax;
			while(hP[0]->GetBinContent(i) > 0 && i < tFix+tWin) {area += hP[0]->GetBinContent(i); i++;}// std::cout << hP[0]->GetBinContent(i) << "	" << bsl << std::endl;}
			i = tmax;
                        while(hP[0]->GetBinContent(i) > 0 && i > tFix) {area += hP[0]->GetBinContent(i); i--;}
/*
                        high = 0;
                        tmax = 0;
                        for(i = 0; i < nSamples; i++) if(hP[0]->GetBinContent(i) > high) {high = hP[0]->GetBinContent(i); tmax = i;}
                        area = 0;
                        i = tmax;
                        while(hP[0]->GetBinContent(i) > 0) {area += hP[0]->GetBinContent(i); i++;}// std::cout << hP[0]->GetBinContent(i) << " " << bsl << std::endl;}
                        i = tmax;
                        while(hP[0]->GetBinContent(i) > 0) {area += hP[0]->GetBinContent(i); i--;}
*/
	                if(area > 0) hAreaTot->Fill(area);

			if(area > 0) hAreaHigh->Fill(area,high);

	                ev++;
		}
        }

        TFile * foutroot = new TFile(Form("%s",filenew.c_str()),"recreate");
		hArea->Write();
		hAreaTot->Write();
		hAreaHigh->Write();
        foutroot->Close();

	return 0;
}

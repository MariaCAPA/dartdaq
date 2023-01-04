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
#include "TF1.h"

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

	double hMax = 1000;

        if(*argv[3] == '1') hMax = 200;
        else if(*argv[3] == '2') hMax = 400;
        else if(*argv[3] == '3') hMax = 600;
        else if(*argv[3] == '4') hMax = 800;
        else if(*argv[3] == '5') hMax = 1000;
        else if(*argv[3] == '6') hMax = 1200;
        else if(*argv[3] == '7') hMax = 1400;
        else if(*argv[3] == '8') hMax = 1600;
        else if(*argv[3] == '9') hMax = 1800;
        else if(*argv[3] == '0') hMax = 2000;

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

        TH1F * hHigh = new TH1F("hHigh","hHigh",100,0,hMax);

        TH2F * hAreaHigh = new TH2F("hAreaHigh","hAreaHigh",2*hMax+10,-1000,20*hMax,hMax/10,0,hMax);

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

        TTree * t2 = new TTree("ta","");
        t2->Branch("area",&area,"&area/D");
        t2->Branch("high",&high,"&high/D");

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
	                        }
	                }

			high = -1e6;
			tmax = 0;
			for(i = tFix; i < tFix+tWin; i++) if(hP[0]->GetBinContent(i) > high) {high = hP[0]->GetBinContent(i); tmax = i;}
			area = 0;
			for(i = tFix; i < tFix+tWin; i++) area += hP[0]->GetBinContent(i);

			t2->Fill();

	                ev++;

			hAreaHigh->Fill(area,high);
			hHigh->Fill(high);
			hArea->Fill(area);

		}
        }

	double min1, min2, min3, min4;
	double max0, max1, max2, max3;

	i = 0;

	while(hHigh->GetBinContent(i+2)-hHigh->GetBinContent(i+1) > 0) i++;
	max0 = (i+0.5)*(hMax/100);
	i += 2;
        while(hHigh->GetBinContent(i+2)-hHigh->GetBinContent(i+1) < 0) i++;
        min1 = (i+0.5)*(hMax/100);
        i += 2;
        while(hHigh->GetBinContent(i+2)-hHigh->GetBinContent(i+1) > 0) i++;
        max1 = (i+0.5)*(hMax/100);
        i += 2;
        while(hHigh->GetBinContent(i+2)-hHigh->GetBinContent(i+1) < 0) i++;
        min2 = (i+0.5)*(hMax/100);
	i += 2;
        while(hHigh->GetBinContent(i+2)-hHigh->GetBinContent(i+1) > 0) i++;
        max2 = (i+0.5)*(hMax/100);
//        i += 2;
//        while(hHigh->GetBinContent(i+2)-hHigh->GetBinContent(i+1) < 0) i++;
        min3 = 2*min2-min1;
        i = (int)100*min3/hMax;
        while(hHigh->GetBinContent(i+2)-hHigh->GetBinContent(i+1) > 0) i++;
        max3 = (i+0.5)*(hMax/100);
//        i += 2;
//        while(hHigh->GetBinContent(i+2)-hHigh->GetBinContent(i+1) < 0) i++;
        min4 = 3*min2-2*min1;

        TCanvas * cAreaHigh = new TCanvas("cAreaHigh","cAreaHigh");
	hAreaHigh->GetXaxis()->SetTitle("Area");
        hAreaHigh->GetYaxis()->SetTitle("Altura");
        hAreaHigh->Draw("colz");
        TLine * lineMinArea1 = new TLine(0,min1,20000,min1);
        lineMinArea1->SetLineColor(kRed);
        lineMinArea1->SetLineWidth(2);
        lineMinArea1->Draw("same");
        TLine * lineMinArea2 = new TLine(0,min2,20000,min2);
        lineMinArea2->SetLineColor(kRed);
        lineMinArea2->SetLineWidth(2);
        lineMinArea2->Draw("same");
        TLine * lineMinArea3 = new TLine(0,min3,20000,min3);
        lineMinArea3->SetLineColor(kRed);
        lineMinArea3->SetLineWidth(2);
        lineMinArea3->Draw("same");
        TLine * lineMinArea4 = new TLine(0,min4,20000,min4);
        lineMinArea4->SetLineColor(kRed);
        lineMinArea4->SetLineWidth(2);
        lineMinArea4->Draw("same");

	TCanvas * cHigh = new TCanvas("cHigh","cHigh");
        hHigh->GetXaxis()->SetTitle("Altura");
	hHigh->Draw();
        TLine * lineMax0 = new TLine(max0,0,max0,hHigh->GetMaximum());
        lineMax0->SetLineColor(kOrange);
        lineMax0->SetLineWidth(2);
        lineMax0->Draw("same");
        TLine * lineMax1 = new TLine(max1,0,max1,hHigh->GetMaximum());
        lineMax1->SetLineColor(kOrange);
        lineMax1->SetLineWidth(2);
        lineMax1->Draw("same");
        TLine * lineMax2 = new TLine(max2,0,max2,hHigh->GetMaximum());
        lineMax2->SetLineColor(kOrange);
        lineMax2->SetLineWidth(2);
        lineMax2->Draw("same");
        TLine * lineMax3 = new TLine(max3,0,max3,hHigh->GetMaximum());
        lineMax3->SetLineColor(kOrange);
        lineMax3->SetLineWidth(3);
        lineMax3->Draw("same");
        TLine * lineMin1 = new TLine(min1,0,min1,hHigh->GetMaximum());
        lineMin1->SetLineColor(kGreen);
        lineMin1->SetLineWidth(2);
        lineMin1->Draw("same");
        TLine * lineMin2 = new TLine(min2,0,min2,hHigh->GetMaximum());
        lineMin2->SetLineColor(kGreen);
        lineMin2->SetLineWidth(2);
        lineMin2->Draw("same");
        TLine * lineMin3 = new TLine(min3,0,min3,hHigh->GetMaximum());
        lineMin3->SetLineColor(kGreen);
        lineMin3->SetLineWidth(2);
        lineMin3->Draw("same");
        TLine * lineMin4 = new TLine(min4,0,min4,hHigh->GetMaximum());
        lineMin4->SetLineColor(kGreen);
        lineMin4->SetLineWidth(2);
        lineMin4->Draw("same");

	double area1, area2, area3;
	double highEv, areaEv;
	int nEv1, nEv2, nEv3;
	nEv1 = 0;
	nEv2 = 0;
	nEv3 = 0;

        t2->SetBranchAddress("high",&highEv);
        t2->SetBranchAddress("area",&areaEv);

	for(int i = 0; i < ev; i++)
	{
		t2->GetEntry(i);
		if(highEv > min1 && highEv < min2) {area1 += areaEv; nEv1++;}
                if(highEv > min2 && highEv < min3) {area2 += areaEv; nEv2++;}
                if(highEv > min3 && highEv < min4) {area3 += areaEv; nEv3++;}
	}

	area1 = area1/nEv1;
        area2 = area2/nEv2;
	area3 = area3/nEv3;

	double * meanArea = new double [3];
        double * nphe = new double [3];

	meanArea[0] = area1;
        meanArea[1] = area2;
        meanArea[2] = area3;
        nphe[0] = 1;
        nphe[1] = 2;
        nphe[2] = 3;

	TF1 * fCal = new TF1("fCal","[0]+[1]*x",0,3);

        TCanvas * cFit = new TCanvas("cFit","cFit");
	TGraph * gArea = new TGraph(3,nphe,meanArea);
	gArea->GetXaxis()->SetTitle("# de phes");
        gArea->GetYaxis()->SetTitle("Area");
	gArea->SetMarkerStyle(21);
	gArea->Fit("fCal");
	gArea->Draw("Ap");

        TFile * foutroot = new TFile(Form("%s",filenew.c_str()),"recreate");
		hArea->Write();
		cHigh->Write();
		cAreaHigh->Write();
		cFit->Write();
        foutroot->Close();

	return 0;
}

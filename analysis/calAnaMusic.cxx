//C,C++ Libraries
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <iomanip>

// ROOT libraries
#include <TROOT.h>
#include <TF1.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TTree.h>
#include <TChain.h>
#include <TObject.h>
#include <TStyle.h>
#include <TBrowser.h>
#include <TApplication.h>
#include <TFile.h>
#include <TRandom3.h>
#include <TCutG.h>
#include <TString.h>
#include <TObjArray.h>
#include <TPaveLabel.h>

double calAnaMusic (TTree *pulses)
{
    //Definimos propiedades histograma de area

    TH1F *area_aux = new TH1F("area_aux","area_aux",10000,100,50);

    pulses->Draw("areaFix0 >> area_aux","high0>0","");

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
    TH1F *heigh = new TH1F("heigh", "Heigh",heigh_nbins,0,heigh_max);
    TH1F *rms = new TH1F("rms", "Rms",90,0,100);

    pulses->Draw("areaFix0 >> area","high0>0","");
    pulses->Draw("high0 >> heigh","high0>0","");
    pulses->Draw("rms0 >> rms","","");

    cout << "Mean area: " << area->GetMean() << endl;
    cout << "Mean heigh: " << heigh->GetMean() << endl;
    cout << "Mean rms: " << rms->GetMean() << endl;

   TCanvas *c1 = new TCanvas("c1","The Graph example",10,10,700,500);


    int i, j, k=0;
    int heigh_cut[3];

    double media=0, control=1, media_ant=0, control_ant=1;

    //heigh->GetXaxis()->SetRangeUser(0,500);

    for(i=0; i<3; i++)
    	heigh_cut[i]=0;

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
    

    if(heigh_cut[2]-heigh_cut[1]>1.2*(heigh_cut[1]-heigh_cut[0])) //Ajuste por si se va mucho el limite superior de 2 phe
	      heigh_cut[2]=heigh_cut[1]+heigh_cut[1]-heigh_cut[0];

    heigh->SetTitle(Form("Heigh: run %d",run));

    heigh->Draw();


    TLine * line;

    for(k=0; k<3; k++)
    {
    	line = new TLine(heigh_cut[k],0,heigh_cut[k],heigh->GetBinContent(heigh->GetMaximumBin())); //Coordenadas de la línea
    	line->SetLineWidth(2.); //Tamaño de la línea
    	line->SetLineColor(kRed); //Color de la línea
    	line->Draw("same"); //Para dibujar la línea
    }

    TH1F * one_phe = new TH1F("one_phe", "1 phe",area_nbins,area_min,area_max);
    TH1F * two_phe = new TH1F("two_phe", "2 phe",area_nbins,area_min,area_max);
    TH1F * noise = new TH1F("noise", "noise",area_nbins,area_min,area_max);

    one_phe->SetLineColor(kRed);
    two_phe->SetLineColor(kGreen);
    noise->SetLineColor(kBlack);

    pulses->Draw("areaFix0>>one_phe",Form("high0>%d && high0<%d",heigh_cut[0],heigh_cut[1]));
    pulses->Draw("areaFix0>>two_phe",Form("high0>%d && high0<%d",heigh_cut[1],heigh_cut[2]));
    pulses->Draw("areaFix0>>noise",Form("high0<%d",heigh_cut[0]));

    area->SetTitle(Form("Area integration window: run %d",run));
    
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

    TF1 *one_phe_fit = new TF1("one_phe","gaus",0,pulses->GetMaximum("areaFix0"));
    TF1 *two_phe_fit = new TF1("two_phe","gaus",0,pulses->GetMaximum("areaFix0"));

    one_phe->Fit("one_phe");
    two_phe->Fit("two_phe");

    double phe_calibration, phe_calibration_error;

    phe_calibration=two_phe_fit->GetParameter(1)-one_phe_fit->GetParameter(1);
    phe_calibration_error=sqrt(two_phe_fit->GetParError(1)*two_phe_fit->GetParError(1)+one_phe_fit->GetParError(1)*one_phe_fit->GetParError(1));


    return phe_calibration;
}

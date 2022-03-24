#ifndef __DART_EVENT__H__
#define __DART_EVENT__H__

#include "TObject.h"
#include <vector>
#include <iostream>


class TDartEvent: public TObject
{
  public:
    class TDartCh
    {
      public:
      int ch;
      double charge;
      double charge90;
      double charge640;
      double bsl;
      double bmax;
      double bmin;
      double bmaxp;
      double bminp;
      double bimax;
      double bslEnd;
      double bmaxEnd;
      double rms;
      double rmsEnd;
      double max;
      double min;
      double t0;
      double tMax;
      double tMin;
      void Reset() {ch=-1; charge=-1; charge90=-1; charge640=-1; bsl=-1; bmax=-1; bmaxp=-1; bmin=-1; bminp=-1; bimax=-1; bslEnd=-1; rms=-1; rmsEnd=-1; max=-1; min=-1; t0=-1; tMax=-1; tMin=-1;}
      TDartCh(){Reset();}
      virtual ~TDartCh(){Reset();}
      TDartCh( const TDartCh &obj){ch=obj.ch; charge=obj.charge; charge90=obj.charge90; charge640=obj.charge640; bsl=obj.bsl; bmax=obj.bmax; bmaxp=obj.bmaxp; bmin=obj.bmin; bminp=obj.bminp; bimax=obj.bimax; bslEnd=obj.bslEnd; rms=obj.rms; rmsEnd=obj.rmsEnd; max=obj.max; min=obj.min; t0=obj.t0; tMax=obj.tMax; tMin=obj.tMin;} // copy constructor

      const TDartCh & operator = ( const TDartCh &obj ){ch=obj.ch; charge=obj.charge; charge90=obj.charge90; charge640=obj.charge640; bsl=obj.bsl; bmax=obj.bmax; bmaxp=obj.bmaxp; bmin=obj.bmin; bminp=obj.bminp; bimax=obj.bimax; bslEnd=obj.bslEnd; rms=obj.rms; rmsEnd=obj.rmsEnd; max=obj.max; min=obj.min; t0=obj.t0; tMax=obj.tMax; tMin=obj.tMin; return *this;} 
      void Dump() const
      {
        std::cout << "- ch: " << ch << " charge: " << charge << " charge90: " << charge90 << " charge640: " << charge640 << std::endl;
        std::cout << " bsl: " << bsl << " bslEnd: " << bslEnd << " bmax: " << bmax << " bmaxp: " << bmaxp << "bimax: " << bimax << "bmin" << bmin << "bminp" << bminp << " rms: " << rms << " rmsEnd: " << rmsEnd << std::endl;
        std::cout << " max: " << max << " min: " << min << " t0: " << t0 << " tMax: " << tMax << " tMin: " << tMin << std::endl;
      }
      ClassDef (TDartEvent::TDartCh,2);
    };
   
    class TVetoCh
    {
      public:
      int Vch;
      double Vcharge;
      double Vbsl;
      double Vbmax;
      double Vbmin;
      double Vbmaxp;
      double Vbminp;
      double Vbimax;
      double Vrms;
      double Vmax;
      double Vt0; 
      double VtMax;
      double Vmin;
      double VtMin;
      void Reset() {Vch=-1; Vcharge=-1; Vbsl=-1; Vrms=-1; Vmax=-1; Vt0=-1; VtMax=-1;Vmin=-1; VtMin=-1;}
      TVetoCh(){Reset();}
      virtual ~TVetoCh(){Reset();}
      TVetoCh( const TVetoCh &obj){Vch=obj.Vch; Vcharge=obj.Vcharge; Vbsl=obj.Vbsl; Vrms=obj.Vrms; Vmax=obj.Vmax; Vt0=obj.Vt0; VtMax=obj.VtMax; Vmin=obj.Vmin; VtMin = obj.VtMin;}  // copy constructor
      const TVetoCh & operator = ( const TVetoCh& obj ) {Vch=obj.Vch; Vcharge=obj.Vcharge; Vbsl=obj.Vbsl; Vrms=obj.Vrms; Vmax=obj.Vmax; Vt0=obj.Vt0; VtMax=obj.VtMax; Vmin=obj.Vmin; VtMin = obj.VtMin; return *this;}
      void Dump() const{std::cout << "- Vch: " << Vch << " Vcharge: " << Vcharge << " Vbsl: " << Vbsl << " Vrms: " << Vrms << " Vmax: " << Vmax << " Vt0: " << Vt0 << " VtMax: " << VtMax << " Vmin: " << Vmin << " VtMin: " << VtMin <<  std::endl;}
     ClassDef (TDartEvent::TVetoCh,2);
    };

    TDartEvent(){Reset();}
    virtual ~TDartEvent(){};
    void Reset()
    {
      run=-1;
      eventNumber=-1;
      midasEventNumber=-1;
      bankNumber=-1;
      time=-1;
      timeNs=-1;
      vetoMult=-1;
      totCharge=-1;
      vetoCharge=-1;
      original_file="";
      dt=-1; 
      dat=-1;
      type=-1;
      if (dartChannel.size()>0) dartChannel.clear();
      if (vetoChannel.size()>0) vetoChannel.clear(); // MARIA 220620
    }

    // MEMEBERS
    int run;
    int eventNumber;
    int midasEventNumber;
    int bankNumber;
    double time;
    ULong64_t timeNs;
    int vetoMult;
    double totCharge;
    double vetoCharge;
    std::string original_file; // original raw file name, to look for the pulse 
    unsigned long int  dt; // Time since prev event, in ns 
    unsigned long int  dat; // Time since prev alfa event, in ns 
    int type; // 0-> gamma, 1-> alpha, 2-> muon
    std::vector<TDartEvent::TDartCh> dartChannel;
    std::vector<TDartEvent::TVetoCh> vetoChannel; // VetoCh PMTs
    void Dump() const
    {
      std::cout << " run: " << run << std::endl;
      std::cout << " eventNumber: " << eventNumber << std::endl;
      std::cout << " midasEventNumber: " << midasEventNumber << std::endl;
      std::cout << " bankNumber: " << bankNumber << std::endl;
      std::cout << " time: " << time << std::endl;
      std::cout << " timeNs: " << timeNs << std::endl;
      std::cout << " vetoMult: " << vetoMult << std::endl;
      std::cout << " totCharge: " << totCharge << std::endl;
      std::cout << " vetoCharge: " << vetoCharge << std::endl;
      std::cout << " original file: " << original_file << std::endl;
      std::cout << " dt: " << dt << std::endl;
      std::cout << " dat: " << dat << std::endl;
      std::cout << " type: " << type << std::endl;
      if (dartChannel.size()>0) std::cout<< "* dartChannels: " << std::endl;
      for (unsigned int i=0; i<dartChannel.size();i++) dartChannel[i].Dump();
      if (vetoChannel.size()>0) std::cout<< "* vetoChannels: " << std::endl;
      for (unsigned int i=0; i<vetoChannel.size();i++) vetoChannel[i].Dump();

    }

  ClassDef (TDartEvent,3);
};



#endif // __DART_EVENT__H__

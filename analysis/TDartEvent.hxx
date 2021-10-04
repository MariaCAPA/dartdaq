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
      double bsl;
      double rms;
      double high;
      double t0;
      double tMax;
      void Reset() {ch=-1; charge=-1; bsl=-1; rms=-1;high=-1; t0=-1; tMax=-1;}
      TDartCh(){Reset();}
      virtual ~TDartCh(){Reset();}
      TDartCh( const TDartCh &obj){ch=obj.ch; charge=obj.charge; bsl=obj.bsl; rms=obj.rms; high=obj.high; t0=obj.t0; tMax=obj.tMax;} // copy constructor

      const TDartCh & operator = ( const TDartCh& obj ){ch=obj.ch; charge=obj.charge; bsl=obj.bsl; rms=obj.rms; high=obj.high; t0=obj.t0; tMax=obj.tMax; return *this;} 
      void Dump() const{std::cout << "- ch: " << ch << " charge: " << charge << " bsl: " << bsl << " rms: " << rms << " high: " << high << " t0: " << t0 << " tMax: " << tMax << std::endl;}
      ClassDef (TDartEvent::TDartCh,1);
    };
   
    class TVetoCh
    {
      public:
      int Vch;
      double Vcharge;
      double Vbsl;
      double Vrms;
      double Vhigh;
      double Vt0; 
      double VtMax;
      void Reset() {Vch=-1; Vcharge=-1; Vbsl=-1; Vrms=-1; Vhigh=-1; Vt0=-1; VtMax=-1;}
      TVetoCh(){Reset();}
      virtual ~TVetoCh(){Reset();}
      TVetoCh( const TVetoCh &obj){Vch=obj.Vch; Vcharge=obj.Vcharge; Vbsl=obj.Vbsl; Vrms=obj.Vrms; Vhigh=obj.Vhigh; Vt0=obj.Vt0; VtMax=obj.VtMax; }  // copy constructor
      const TVetoCh & operator = ( const TVetoCh& obj ) {Vch=obj.Vch; Vcharge=obj.Vcharge; Vbsl=obj.Vbsl; Vrms=obj.Vrms; Vhigh=obj.Vhigh; Vt0=obj.Vt0; VtMax=obj.VtMax; return *this;}
      void Dump() const{std::cout << "- Vch: " << Vch << " Vcharge: " << Vcharge << " Vbsl: " << Vbsl << " Vrms: " << Vrms << " Vhigh: " << Vhigh << " Vt0: " << Vt0 << " VtMax: " << VtMax << std::endl;}
     ClassDef (TDartEvent::TVetoCh,1);
    };

    TDartEvent(){Reset();}
    virtual ~TDartEvent(){};
    void Reset()
    {
      run=-1;
      eventNumber=-1;
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

  ClassDef (TDartEvent,1);
};



#endif // __DART_EVENT__H__

#ifndef __A_EVENT__H__
#define __A_EVENT__H__

#include "TObject.h"
#include <vector>
#include <iostream>


class TAEvent: public TObject
{
  public:
    class TACh
    {
      public:
      int ch;
      int det;
      double area;
      double bsl;
      double rms;
      double max;
      double min;
      double t0;
      double tMax;
      double tMin;
      void Reset() {ch=-1; det=-1; area=-1; bsl=-1; rms=-1; max=-1; min=-1; t0=-1; tMax=-1; tMin=-1;}
      TACh(){Reset();}
      virtual ~TACh(){Reset();}
      TACh( const TACh &obj){ch=obj.ch; det=obj.det; area=obj.area; bsl=obj.bsl; rms=obj.rms; max=obj.max; min=obj.min; t0=obj.t0; tMax=obj.tMax; tMin=obj.tMin;} // copy constructor

      const TACh & operator = ( const TACh &obj ){ch=obj.ch; det=obj.det; area=obj.area; bsl=obj.bsl; rms=obj.rms; max=obj.max; min=obj.min; t0=obj.t0; tMax=obj.tMax; tMin=obj.tMin; return *this;} 
      void Dump() const
      {
        std::cout << "- ch: " << ch << " det: " << det << " area: " << area << std::endl;
        std::cout << " bsl: " << bsl << " rms: " << rms << std::endl;
        std::cout << " max: " << max << " min: " << min << " t0: " << t0 << " tMax: " << tMax << " tMin: " << tMin << std::endl;
      }
      ClassDef (TAEvent::TACh,1);
    };
   
    TAEvent(){Reset();}
    virtual ~TAEvent(){};
    void Reset()
    {
      run=-1;
      eventNumber=-1;
      eventCounter=-1;
      midasEventNumber=-1;
      bankNumber=-1;
      time=-1;
      timeNs=-1;
      mult=-1;
      areaS=-1;
      original_file="";
      dt=-1; 
      if (channel.size()>0) channel.clear();
    }

    // MEMEBERS
    int run;
    int eventNumber;
    int eventCounter;
    int midasEventNumber;
    int bankNumber;
    double time;
    ULong64_t timeNs;
    int mult;
    double areaS;
    std::string original_file; // original raw file name, to look for the pulse 
    unsigned long int  dt; // Time since prev event, in ns 
    std::vector<TAEvent::TACh> channel;
    void Dump() const
    {
      std::cout << " run: " << run << std::endl;
      std::cout << " eventNumber: " << eventNumber << std::endl;
      std::cout << " eventCounter: " << eventNumber << std::endl;
      std::cout << " midasEventNumber: " << midasEventNumber << std::endl;
      std::cout << " bankNumber: " << bankNumber << std::endl;
      std::cout << " time: " << time << std::endl;
      std::cout << " timeNs: " << timeNs << std::endl;
      std::cout << " mult: " << mult << std::endl;
      std::cout << " areaS: " << areaS << std::endl;
      std::cout << " original file: " << original_file << std::endl;
      std::cout << " dt: " << dt << std::endl;
      if (channel.size()>0) std::cout<< "* channel: " << std::endl;
      for (unsigned int i=0; i<channel.size();i++) channel[i].Dump();

    }

  ClassDef (TAEvent,2);
};



#endif // __A_EVENT__H__

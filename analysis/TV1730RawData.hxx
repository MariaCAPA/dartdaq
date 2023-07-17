#ifndef TV1730RawData_hxx_seen
#define TV1730RawData_hxx_seen

#include <vector>
#include <map>

#include "TGenericData.hxx"

struct TV1730EventHeader 
{
   uint16_t channel_mask;
   uint16_t trigger_mask;
   uint32_t samples;
   uint64_t timeStampNs;
   uint64_t eventCounter;

   TV1730EventHeader(const uint16_t* buffer=NULL);
};


/// Class to store information from a single V1730 channel.
class TV1730RawChannel 
{

public:

  /// constructor
  TV1730RawChannel(int channel=-1, int num_samples=0) 
  {
    fChannelNumber = channel;
    fWaveform.resize(num_samples);
  }

  int GetChannelNumber() const 
  {
    return fChannelNumber;
  }

  /// Get the ADC sample for a particular bin (for uncompressed data).
  int GetNSamples() const 
  {
    return fWaveform.size();
  }

  /// Get the ADC sample for a particular bin (for uncompressed data).
  int GetADCSample(int i) const 
  {
    if (i >= 0 && i < (int) fWaveform.size())
      return fWaveform[i];

    // otherwise, return error value.
    return -1;
  }

  /// Returns true for objects with no ADC samples 
  int IsEmpty() const 
  {
    return fWaveform.size() == 0;
  }

  /// Set ADC sample
  void SetADCSample(uint32_t i, uint16_t sample) 
  {
    fWaveform[i] = sample;
  }

private:

  /// Channel number
  int fChannelNumber;

  std::vector<uint16_t> fWaveform;

};

/// Class to store data from CAEN V1730, 500MHz FADC.
/// This class encapsulates the data from a single board (in a single MIDAS bank).
/// This decoder is for the default version of the firmware.
class TV1730RawData: public TGenericData 
{

public:

  /// Constructor
  TV1730RawData(int bklen, int bktype, const char *name, void *pdata);

  /// Get the number of words in bank.
  TV1730EventHeader GetHeader() const 
  {
    return fHeader;
  }

  /// Get Number of channels in this bank.
  int GetNChannels() const 
  {
    return fMeasurements.size();
  }

  /// Get Channel Data
  TV1730RawChannel& GetChannelData(int i) 
  {
    return fMeasurements[i];
  }

  void Print();

private:

  TV1730EventHeader fHeader;

  /// Map of V1730 measurements
  //std::map<int, TV1730RawChannel> fMeasurements;
  // define as a vector. Every RawChannel knows its channel number
  std::vector <TV1730RawChannel> fMeasurements; 

};

#endif

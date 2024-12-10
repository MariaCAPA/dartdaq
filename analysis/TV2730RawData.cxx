#include "TV2730RawData.hxx"

#include <iomanip>
#include <iostream>
#include <arpa/inet.h>

const int numChannels=32;
TV2730EventHeader::TV2730EventHeader(const uint16_t *data) 
{
  // header =  word + word +  dword +  ddword + ddword = 12 * word
  if (data) 
  {
     channel_mask = *((uint32_t*)(&data[0]));
     //channel_mask = data[0];
     //trigger_mask = data[1];
     samples = *((uint32_t*)(&data[2]));
     timeStampNs = *((uint64_t*)(&data[4]));
     eventCounter = *((uint64_t*)(&data[8]));
// DEB
//std::cout <<" timestamps:"  <<  timeStampNs << std::endl;
//std::cout << "samples : " << samples << std::endl;

    //format = data[0] & 0xFF;
    //event_counter = (data[0] >> 8) & (0xFFFFFF);
    //size_64bit_words = data[0] >> 32;
    //trigger_mask = data[1] >> 52;
    //overlap = (data[1] >> 48) & 0xF;
    //trigger_time = data[1] & 0xFFFFFFFFFFFF;
  }
}

TV2730RawData::TV2730RawData(int bklen, int bktype, const char *name, void *pdata) :
    TGenericData(bklen, bktype, name, pdata) 
{
  fHeader = TV2730EventHeader(GetData16());
  uint32_t num_samples = fHeader.samples;

// DEB
//std::cout <<  " channel_mask " << fHeader.channel_mask
   //<< " trigger_mask " <<  fHeader.trigger_mask
   //<< " samples " <<  fHeader.samples
   //<< " timeStampNs " <<  fHeader.timeStampNs
   //<< "eventCounter " <<  fHeader.eventCounter << std::endl;


  // fMeasurement
  for (int i = 0; i < 32; i++) 
  {
    if (fHeader.channel_mask & (1 << i)) 
    {
      //fMeasurements[i] = TV2730RawChannel(i, num_samples); // es un objeto, no un puntero
      fMeasurements.push_back(TV2730RawChannel(i, num_samples)); // es un objeto, no un puntero
    }
  }

  // Data format is: header = 12 words
  const uint16_t* p = GetData16() + 12;

  //for (auto it = fMeasurements.begin(); it != fMeasurements.end(); it++) 
  for (unsigned int i=0; i<fMeasurements.size(); i++)
  {
    for (uint32_t samp = 0; samp < num_samples; samp ++)
    {
      //uint64_t samp_dcba = *p++;

      fMeasurements[i].SetADCSample(samp, *p++);

      //it->second.SetADCSample(samp, *p++);
      //it->second.SetADCSample(samp + 1, (samp_dcba >> 16) & 0xFFFF);
      //it->second.SetADCSample(samp + 2, (samp_dcba >> 32) & 0xFFFF);
      //it->second.SetADCSample(samp + 3, (samp_dcba >> 48) & 0xFFFF);
    }
  }
}

void TV2730RawData::Print() 
{
  std::cout << "V2730 decoder for bank " << GetName().c_str() << std::endl;

  std::cout << "Channel Mask: " << fHeader.channel_mask << std::endl;

  //std::cout << "trigger_mask: " << fHeader.trigger_mask << std::endl;
  std::cout << "Time stamp ns: " << fHeader.timeStampNs << std::endl;
  std::cout << "Event Counter: " << fHeader.eventCounter << std::endl;

  std::cout << "Num channels: " << GetNChannels() << std::endl;

}

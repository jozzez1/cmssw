#include "SimTracker/SiStripDigitizer/interface/SiTrivialDigitalConverter.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

SiTrivialDigitalConverter::SiTrivialDigitalConverter(float in, bool PreMix) :
  electronperADC(in), PreMixing_(PreMix) {
  _temp.reserve(800);
  _tempRaw.reserve(800);
}

SiDigitalConverter::DigitalVecType
SiTrivialDigitalConverter::convert(const std::vector<float>& analogSignal, edm::ESHandle<SiStripGain> & gainHandle, unsigned int detid){
  
  _temp.clear();

  if(PreMixing_) {
    for ( size_t i=0; i<analogSignal.size(); i++) {
      if (analogSignal[i]<=0) continue;
      // convert analog amplitude to digital - special algorithm for PreMixing. 
      // Need to keep all hits, including those at very low pulse heights.
      int adc = truncate( sqrt(9.0*analogSignal[i]) );
      if ( adc > 0) _temp.push_back(SiStripDigi(i, adc));
    }
  }
  else if(gainHandle.isValid()) {
    SiStripApvGain::Range detGainRange = gainHandle->getRange(detid);
    for ( size_t i=0; i<analogSignal.size(); i++) {
      if (analogSignal[i]<=0) continue;
      // convert analog amplitude to digital
      int adc = convert( (gainHandle->getStripGain(i, detGainRange))*(analogSignal[i]) );
      if ( adc > 0) _temp.push_back(SiStripDigi(i, adc));
    }
  } else {
    for ( size_t i=0; i<analogSignal.size(); i++) {
      if (analogSignal[i]<=0) continue;
      // convert analog amplitude to digital
      int adc = convert( analogSignal[i] );
      if ( adc > 0) _temp.push_back(SiStripDigi(i, adc));
    }
  }
  return _temp;
}

SiDigitalConverter::DigitalRawVecType
SiTrivialDigitalConverter::convertRaw(const std::vector<float>& analogSignal, edm::ESHandle<SiStripGain> & gainHandle, unsigned int detid){
  
  _tempRaw.clear();

  if(gainHandle.isValid()) {
    SiStripApvGain::Range detGainRange = gainHandle->getRange(detid);
    for ( size_t i=0; i<analogSignal.size(); i++) {
      if (analogSignal[i]<=0) { _tempRaw.push_back(SiStripRawDigi(0)); continue; }
      // convert analog amplitude to digital
      int adc = convertRaw( (gainHandle->getStripGain(i, detGainRange))*(analogSignal[i]));
      _tempRaw.push_back(SiStripRawDigi(adc));
    }
  } else {
    for ( size_t i=0; i<analogSignal.size(); i++) {
      if (analogSignal[i]<=0) { _tempRaw.push_back(SiStripRawDigi(0)); continue; }
      // convert analog amplitude to digital
      int adc = convertRaw( analogSignal[i] );
      _tempRaw.push_back(SiStripRawDigi(adc));
    }
  }
  return _tempRaw;
}

int SiTrivialDigitalConverter::truncate(float in_adc) const {
  //Rounding the ADC number instead of truncating it
  //we shift bits to the right so as to ignore the most significant
  //and least significant bit
  int adc = int(in_adc+0.5) >> 1;
  /*
   * we lose some resolution, but can measure up to 510 in charge deposit
   * charge values {0,2,4,...,508,510} are represented as {0,1,2,...,254,255}
   *
   * 254 ADC:  510 <= raw charge < 1023   -- BSZS truncated mode
   * 255 ADC: 1023 <= raw charge < inf.   -- saturated mode (hardware limit)
  */
  if(PreMixing_) {
    if (adc > 1022 ) return 511;
    if (adc > 510 ) return 510;
  }
  else {
    if (adc > 511 ) return 255;
    if (adc > 253) return 254;
  }
  //Protection
  if (adc < 0) return 0;
  return adc;
}

int SiTrivialDigitalConverter::truncateRaw(float in_adc) const {
  //Rounding the ADC number
  int adc = int(in_adc+0.5);
  if (adc > 1023 ) return 1023;
  //Protection
  if (adc < 0) return 0;
  return adc;
}


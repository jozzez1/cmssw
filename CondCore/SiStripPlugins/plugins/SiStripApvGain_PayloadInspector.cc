/*!
  \file SiStripApvGains_PayloadInspector
  \Payload Inspector Plugin for SiStrip Gain
  \author M. Musich
  \version $Revision: 1.0 $
  \date $Date: 2017/07/02 17:59:56 $
*/

#include "CondCore/Utilities/interface/PayloadInspectorModule.h"
#include "CondCore/Utilities/interface/PayloadInspector.h"
#include "CondCore/CondDB/interface/Time.h"

// the data format of the condition to be inspected
#include "CondFormats/SiStripObjects/interface/SiStripApvGain.h"
#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/SiStripDetId/interface/StripSubdetector.h" 

// needed for the tracker map
#include "CommonTools/TrackerMap/interface/TrackerMap.h"

// auxilliary functions
#include "CondCore/SiStripPlugins/interface/SiStripPayloadInspectorHelper.h"
#include "CalibTracker/SiStripCommon/interface/StandaloneTrackerTopology.h" 

#include <memory>
#include <sstream>

namespace {

  /************************************************
    1d histogram of SiStripApvGains of 1 IOV 
  *************************************************/

  // inherit from one of the predefined plot class: Histogram1D
  class SiStripApvGainsValue : public cond::payloadInspector::Histogram1D<SiStripApvGain> {
    
  public:
    SiStripApvGainsValue() : cond::payloadInspector::Histogram1D<SiStripApvGain>("SiStripApv Gains values",
										 "SiStripApv Gains values", 200,0.0,2.0){
      Base::setSingleIov( true );
    }
    
    bool fill( const std::vector<std::tuple<cond::Time_t,cond::Hash> >& iovs ){
      for ( auto const & iov: iovs) {
	std::shared_ptr<SiStripApvGain> payload = Base::fetchPayload( std::get<1>(iov) );
	if( payload.get() ){
	 
	  std::vector<uint32_t> detid;
	  payload->getDetIds(detid);
	  
	  for (const auto & d : detid) {
	    SiStripApvGain::Range range=payload->getRange(d);
	    for(int it=0;it<range.second-range.first;it++){

	      // to be used to fill the histogram
	      fillWithValue(payload->getApvGain(it,range));
	      
	    }// loop over APVs
	  } // loop over detIds
	}// payload
      }// iovs
      return true;
    }// fill
  };

  /************************************************
    1d histogram of means of SiStripApvGains
    for Tracker Barrel of 1 IOV 
  *************************************************/

  // inherit from one of the predefined plot class: Histogram1D
  class SiStripApvBarrelGainsByLayer : public cond::payloadInspector::Histogram1D<SiStripApvGain> {
    
  public:
    SiStripApvBarrelGainsByLayer() : cond::payloadInspector::Histogram1D<SiStripApvGain>("SiStripApv Gains averages by Barrel layer",
											 "Barrel layer (0-3: TIB), (4-9: TOB)",10,0,10,"average SiStripApv Gain"){
      Base::setSingleIov( true );
    }
    
    bool fill( const std::vector<std::tuple<cond::Time_t,cond::Hash> >& iovs ){
      for ( auto const & iov: iovs) {
	std::shared_ptr<SiStripApvGain> payload = Base::fetchPayload( std::get<1>(iov) );
	if( payload.get() ){
	 
	  TrackerTopology tTopo = StandaloneTrackerTopology::fromTrackerParametersXML(edm::FileInPath("Geometry/TrackerCommonData/data/trackerParameters.xml").fullPath());

	  std::vector<uint32_t> detid;
	  payload->getDetIds(detid);
	  
	  std::map<int,std::pair<float,float> > sumOfGainsByLayer;

	  for (const auto & d : detid) {
	    
	    int subid = DetId(d).subdetId();
	    int layer(-1); 
	    if(subid!=3 && subid!=5) continue;
	    if(subid==3){
	      layer = tTopo.tibLayer(d);
	    } else if(subid==5){
	      // layers of TOB start at 5th bin
	      layer = tTopo.tobLayer(d);
	      layer+=4;
	    }

	    SiStripApvGain::Range range=payload->getRange(d);
	    for(int it=0;it<range.second-range.first;it++){
	      sumOfGainsByLayer[layer].first+=payload->getApvGain(it,range);
	      sumOfGainsByLayer[layer].second+=1.;
	    }// loop over APVs
	  } // loop over detIds

	  // loop on the map to fill the plot
	  for (auto& data : sumOfGainsByLayer){
	    
	    fillWithBinAndValue(data.first-1,(data.second.first/data.second.second));
	  }
	  
	}// payload
      }// iovs
      return true;
    }// fill
  };

  /************************************************
    2d histogram of absolute (i.e. not average)
    SiStripApvGains for Tracker Barrel of 1 IOV
  *************************************************/

  class SiStripApvAbsoluteBarrelGainsByLayer : public cond::payloadInspector::Histogram2D<SiStripApvGain> {
    public:
      SiStripApvAbsoluteBarrelGainsByLayer() : cond::payloadInspector::Histogram2D<SiStripApvGain>("SiStripApv Gains by Barrel layer", "Barrel layer (0-3: TIB), (4-9: TOB)", 10, 0, 10, "SiStripApv Gain", 200, 0.0, 2.0){
          Base::setSingleIov(true);
      }
      

      bool fill (const std::vector< std::tuple<cond::Time_t,cond::Hash> >& iovs){
        for (auto const& iov: iovs){
          std::shared_ptr<SiStripApvGain> payload = Base::fetchPayload (std::get<1>(iov));
          if (payload.get()){

            TrackerTopology tTopo = StandaloneTrackerTopology::fromTrackerParametersXML(edm::FileInPath("Geometry/TrackerCommonData/data/trackerParameters.xml").fullPath());

            std::vector<uint32_t> detid;
            payload->getDetIds(detid);
            for (const auto & d : detid){
              int subid = DetId(d).subdetId();
              if (subid!=3 && subid!=5) continue;

              SiStripApvGain::Range range = payload->getRange(d);
              for (int it=0;it<range.second-range.first;it++){
                  float gain = payload->getApvGain(it, range);
                  fillWithValue(static_cast<float>((subid == 5) ? tTopo.tobLayer(d)+4 : tTopo.tibLayer(d)),
                                (gain > 2.0)?2.0:gain);
              }
            }//loop over detIds
          }// loop over payloads
        }// loop over iovs
        return true;
      }// fill
  };


  /************************************************
    1d histogram of means of SiStripApvGains
    for Tracker Endcaps (minus side) of 1 IOV 
  *************************************************/

  // inherit from one of the predefined plot class: Histogram1D
  class SiStripApvEndcapMinusGainsByDisk : public cond::payloadInspector::Histogram1D<SiStripApvGain> {
    
  public:
    SiStripApvEndcapMinusGainsByDisk() : cond::payloadInspector::Histogram1D<SiStripApvGain>("SiStripApv Gains averages by Endcap (minus) disk",
											     "Endcap (minus) disk (0-2: TID), (3-11: TEC)",12,0,12,"average SiStripApv Gain"){
      Base::setSingleIov( true );
    }
    
    bool fill( const std::vector<std::tuple<cond::Time_t,cond::Hash> >& iovs ){
      for ( auto const & iov: iovs) {
	std::shared_ptr<SiStripApvGain> payload = Base::fetchPayload( std::get<1>(iov) );
	if( payload.get() ){
	 
	  TrackerTopology tTopo = StandaloneTrackerTopology::fromTrackerParametersXML(edm::FileInPath("Geometry/TrackerCommonData/data/trackerParameters.xml").fullPath());

	  std::vector<uint32_t> detid;
	  payload->getDetIds(detid);
	  
	  std::map<int,std::pair<float,float> > sumOfGainsByDisk;

	  for (const auto & d : detid) {

	    int disk=-1;
	    int side=-1;
	    int subid = DetId(d).subdetId();
	    if(subid!=4 && subid!=6) continue;
	    	    
	    if(subid==4){

	      side = tTopo.tidSide(d);
	      disk = tTopo.tidWheel(d); 
	    } else {

	      side = tTopo.tecSide(d);
	      disk = tTopo.tecWheel(d);
	      
	      // disks of TEC start at 4th bin
	      disk+=3;
	    }

	    // only negative side
	    if(side!=1) continue;

	    SiStripApvGain::Range range=payload->getRange(d);
	    for(int it=0;it<range.second-range.first;it++){
	      sumOfGainsByDisk[disk].first+=payload->getApvGain(it,range);
	      sumOfGainsByDisk[disk].second+=1.;
	    }// loop over APVs
	  } // loop over detIds

	  // loop on the map to fill the plot
	  for (auto& data : sumOfGainsByDisk){
	    fillWithBinAndValue(data.first-1,(data.second.first/data.second.second));
	  }
	  
	}// payload
      }// iovs
      return true;
    }// fill
  };

  /************************************************
    1d histogram of means of SiStripApvGains
    for Tracker Endcaps (plus side) of 1 IOV 
  *************************************************/

  // inherit from one of the predefined plot class: Histogram1D
  class SiStripApvEndcapPlusGainsByDisk : public cond::payloadInspector::Histogram1D<SiStripApvGain> {
    
  public:
    SiStripApvEndcapPlusGainsByDisk() : cond::payloadInspector::Histogram1D<SiStripApvGain>("SiStripApv Gains averages by Endcap (plus) disk",
											    "Endcap (plus) disk (0-2: TID), (3-11: TEC)",12,0,12,"average SiStripApv Gain"){
      Base::setSingleIov( true );
    }
    
    bool fill( const std::vector<std::tuple<cond::Time_t,cond::Hash> >& iovs ){
      for ( auto const & iov: iovs) {
	std::shared_ptr<SiStripApvGain> payload = Base::fetchPayload( std::get<1>(iov) );
	if( payload.get() ){
	 
	  TrackerTopology tTopo = StandaloneTrackerTopology::fromTrackerParametersXML(edm::FileInPath("Geometry/TrackerCommonData/data/trackerParameters.xml").fullPath());

	  std::vector<uint32_t> detid;
	  payload->getDetIds(detid);
	  
	  std::map<int,std::pair<float,float> > sumOfGainsByDisk;
	  
	  for (const auto & d : detid) {

	    int disk=-1;
	    int side=-1;
	    int subid = DetId(d).subdetId();
	    if(subid!=4 && subid!=6) continue;

	    if(subid==4){
	      side = tTopo.tidSide(d);
	      disk = tTopo.tidWheel(d);; 
	    } else {
	      
	      side = tTopo.tecSide(d);
	      disk = tTopo.tecWheel(d); 
	      
	      // disks of TEC start at 4th bin
	      disk+=3;
	    }
	    
	    // only positive side
	    if(side!=2) continue;

	    SiStripApvGain::Range range=payload->getRange(d);
	    for(int it=0;it<range.second-range.first;it++){
	      sumOfGainsByDisk[disk].first+=payload->getApvGain(it,range);
	      sumOfGainsByDisk[disk].second+=1.;
	    }// loop over APVs
	  } // loop over detIds

	  // loop on the map to fill the plot
	  for (auto& data : sumOfGainsByDisk){
	    fillWithBinAndValue(data.first-1,(data.second.first/data.second.second));
	  }
	  
	}// payload
      }// iovs
      return true;
    }// fill
  };

  /************************************************
    2D histogram of absolute (i.e. not average)
    SiStripApv Gains on the Endcap- for 1 IOV
   ************************************************/
  class SiStripApvAbsoluteEndcapMinusGainsByDisk : public cond::payloadInspector::Histogram2D<SiStripApvGain> {
  public:
    SiStripApvAbsoluteEndcapMinusGainsByDisk() : cond::payloadInspector::Histogram2D<SiStripApvGain>(
            "SiStripApv Gains averages by Endcap (minus) disk",
            "Endcap (minus) disk (0-2: TID), (3-11: TEC)",12,0,12,
            "SiStripApv Gain", 200, 0.0, 2.0){
        Base::setSingleIov(true);
    }

    bool fill (const std::vector< std::tuple<cond::Time_t,cond::Hash> >& iovs){
      for (auto const& iov: iovs) {
	    std::shared_ptr<SiStripApvGain> payload = Base::fetchPayload( std::get<1>(iov) );
	    if( payload.get() ){

	      TrackerTopology tTopo = StandaloneTrackerTopology::fromTrackerParametersXML(edm::FileInPath("Geometry/TrackerCommonData/data/trackerParameters.xml").fullPath());

	      std::vector<uint32_t> detid;
	      payload->getDetIds(detid);

          for (const auto & d : detid){
            int subid = DetId(d).subdetId(),
                side  = -1,
                disk  = -1;

            switch (subid){
              case 4: side = tTopo.tidSide(d); disk = tTopo.tidWheel(d)     ; break;
              case 6: side = tTopo.tecSide(d); disk = tTopo.tecWheel(d) + 4 ; break;
              default: continue;
            }

            if (side!=1) continue;
            SiStripApvGain::Range range = payload->getRange(d);
            for (int it=0;it<range.second-range.first;it++){
              float gain = payload->getApvGain(it, range);
              fillWithValue((float) disk, (gain>2.0)?2.0:gain);
            }// apvs
          }// detids
        }
      }// iovs
      return true;
    }// fill
  };

  /************************************************
    2D histogram of absolute (i.e. not average)
    SiStripApv Gains on the Endcap+ for 1 IOV
   ************************************************/
  class SiStripApvAbsoluteEndcapPlusGainsByDisk : public cond::payloadInspector::Histogram2D<SiStripApvGain> {
  public:
    SiStripApvAbsoluteEndcapPlusGainsByDisk() : cond::payloadInspector::Histogram2D<SiStripApvGain>(
            "SiStripApv Gains averages by Endcap (plus) disk",
            "Endcap (plus) disk (0-2: TID), (3-11: TEC)",12,0,12,
            "SiStripApv Gain", 200, 0.0, 2.0){
        Base::setSingleIov(true);
    }

    bool fill (const std::vector< std::tuple<cond::Time_t,cond::Hash> >& iovs){
      for (auto const& iov: iovs) {
	    std::shared_ptr<SiStripApvGain> payload = Base::fetchPayload( std::get<1>(iov) );
	    if( payload.get() ){

	      TrackerTopology tTopo = StandaloneTrackerTopology::fromTrackerParametersXML(edm::FileInPath("Geometry/TrackerCommonData/data/trackerParameters.xml").fullPath());

	      std::vector<uint32_t> detid;
	      payload->getDetIds(detid);

          for (const auto & d : detid){
            int subid = DetId(d).subdetId(),
                side  = -1,
                disk  = -1;

            switch (subid){
                case 4: side = tTopo.tidSide(d); disk = tTopo.tidWheel(d)     ; break;
                case 6: side = tTopo.tecSide(d); disk = tTopo.tecWheel(d) + 4 ; break;
                default: continue;
            }

            if (side!=2) continue;
            SiStripApvGain::Range range = payload->getRange(d);
            for (int it=0;it<range.second-range.first;it++){
              float gain = payload->getApvGain(it, range);
              fillWithValue((float) disk, (gain>2.0)?2.0:gain);
            }//apvs
          }//detids
        }
      }// iovs
      return true;
    }// fill
  };

  /************************************************
    TrackerMap of SiStripApvGains (average gain per detid)
  *************************************************/
  class SiStripApvGainsAverageTrackerMap : public cond::payloadInspector::PlotImage<SiStripApvGain> {
  public:
    SiStripApvGainsAverageTrackerMap() : cond::payloadInspector::PlotImage<SiStripApvGain>( "Tracker Map of average SiStripGains" ){
      setSingleIov( true );
    }

    bool fill( const std::vector<std::tuple<cond::Time_t,cond::Hash> >& iovs ){
      auto iov = iovs.front();
      std::shared_ptr<SiStripApvGain> payload = fetchPayload( std::get<1>(iov) );

      std::string titleMap = "SiStrip APV Gain average per module (payload : "+std::get<1>(iov)+")";

      std::unique_ptr<TrackerMap> tmap = std::unique_ptr<TrackerMap>(new TrackerMap("SiStripApvGains"));
      tmap->setTitle(titleMap.c_str());
      tmap->setPalette(1);
      
      std::vector<uint32_t> detid;
      payload->getDetIds(detid);
      
      for (const auto & d : detid) {
	SiStripApvGain::Range range=payload->getRange(d);
	float sumOfGains=0;
	float nAPVsPerModule=0.;
	for(int it=0;it<range.second-range.first;it++){
	  nAPVsPerModule+=1;
	  sumOfGains+=payload->getApvGain(it,range);
	} // loop over APVs
	// fill the tracker map taking the average gain on a single DetId
	tmap->fill(d,(sumOfGains/nAPVsPerModule));
      } // loop over detIds

      //=========================
      
      std::string fileName(m_imageFileName);
      tmap->save(true,0,0,fileName.c_str());

      return true;
    }
  };
  
  /************************************************
    TrackerMap of SiStripApvGains (ratio with previous gain per detid)
  *************************************************/
  class SiStripApvGainsRatioWithPreviousIOVTrackerMap : public cond::payloadInspector::PlotImage<SiStripApvGain> {
  public:
    SiStripApvGainsRatioWithPreviousIOVTrackerMap() : cond::payloadInspector::PlotImage<SiStripApvGain>( "Tracker Map of ratio of SiStripGains with previous IOV" ){
      setSingleIov( false );
    }

    bool fill( const std::vector<std::tuple<cond::Time_t,cond::Hash> >& iovs ){
      
      std::vector<std::tuple<cond::Time_t,cond::Hash> > sorted_iovs = iovs;

      // make absolute sure the IOVs are sortd by since
      std::sort(begin(sorted_iovs), end(sorted_iovs), [](auto const &t1, auto const &t2) {
	  return std::get<0>(t1) < std::get<0>(t2);
	});
      
      auto firstiov  = sorted_iovs.front();
      auto lastiov   = sorted_iovs.back();
      
      std::shared_ptr<SiStripApvGain> last_payload  = fetchPayload( std::get<1>(lastiov) );
      std::shared_ptr<SiStripApvGain> first_payload = fetchPayload( std::get<1>(firstiov) );
      
      std::string titleMap = "SiStrip APV Gain ratio per module average (IOV: ";

      titleMap+=std::to_string(std::get<0>(firstiov));
      titleMap+="/ IOV:";
      titleMap+=std::to_string(std::get<0>(lastiov));
      titleMap+=")";

      std::unique_ptr<TrackerMap> tmap = std::unique_ptr<TrackerMap>(new TrackerMap("SiStripApvGains"));
      tmap->setTitle(titleMap.c_str());
      tmap->setPalette(1);

      std::map<uint32_t,float> lastmap,firstmap;

      std::vector<uint32_t> detid;
      last_payload->getDetIds(detid);

      // cache the last IOV
      for (const auto & d : detid) {
	SiStripApvGain::Range range=last_payload->getRange(d);
	float Gain=0;
	float nAPV=0;
	for(int it=0;it<range.second-range.first;it++){
	  nAPV+=1;
	  Gain+=last_payload->getApvGain(it,range);
	} // loop over APVs
	lastmap[d]=(Gain/nAPV);
      } // loop over detIds
      
      detid.clear();
      
      first_payload->getDetIds(detid);
      
      // cache the first IOV
      for (const auto & d : detid) {
	SiStripApvGain::Range range=first_payload->getRange(d);
	float Gain=0;
	float nAPV=0;
	for(int it=0;it<range.second-range.first;it++){
	  nAPV+=1;
	  Gain+=first_payload->getApvGain(it,range);
	} // loop over APVs
	firstmap[d]=(Gain/nAPV);
      } // loop over detIds
      

      std::map<uint32_t,float> cachedRatio; 
      for(const auto &d : detid){
	float ratio = firstmap[d]/lastmap[d];
	tmap->fill(d,ratio);
	cachedRatio[d] = ratio;
      }
    
      //=========================
      auto range = getTheRange(cachedRatio);

      std::string fileName(m_imageFileName);
      tmap->save(true,range.first,range.second,fileName.c_str());

      return true;
    }
  };

  /************************************************
    TrackerMap of SiStripApvGains (ratio for largest deviation with previous gain per detid)
  *************************************************/
  class SiStripApvGainsRatioMaxDeviationWithPreviousIOVTrackerMap : public cond::payloadInspector::PlotImage<SiStripApvGain> {
  public:
    SiStripApvGainsRatioMaxDeviationWithPreviousIOVTrackerMap() : cond::payloadInspector::PlotImage<SiStripApvGain>( "Tracker Map of ratio (for largest deviation) of SiStripGains with previous IOV" ){
      setSingleIov( false );
    }

    bool fill( const std::vector<std::tuple<cond::Time_t,cond::Hash> >& iovs ){
      
      std::vector<std::tuple<cond::Time_t,cond::Hash> > sorted_iovs = iovs;

      // make absolute sure the IOVs are sortd by since
      std::sort(begin(sorted_iovs), end(sorted_iovs), [](auto const &t1, auto const &t2) {
	  return std::get<0>(t1) < std::get<0>(t2);
	});
      
      auto firstiov  = sorted_iovs.front();
      auto lastiov   = sorted_iovs.back();
      
      std::shared_ptr<SiStripApvGain> last_payload  = fetchPayload( std::get<1>(lastiov) );
      std::shared_ptr<SiStripApvGain> first_payload = fetchPayload( std::get<1>(firstiov) );
      
      std::string titleMap = "SiStrip APV Gain ratio for largest deviation per module (IOV: ";

      titleMap+=std::to_string(std::get<0>(firstiov));
      titleMap+="/ IOV:";
      titleMap+=std::to_string(std::get<0>(lastiov));
      titleMap+=")";

      std::unique_ptr<TrackerMap> tmap = std::unique_ptr<TrackerMap>(new TrackerMap("SiStripApvGains"));
      tmap->setTitle(titleMap.c_str());
      tmap->setPalette(1);

      std::map<std::pair<uint32_t,int>,float> lastmap,firstmap;

      std::vector<uint32_t> detid;
      last_payload->getDetIds(detid);
      
      // cache the last IOV
      for (const auto & d : detid) {
	SiStripApvGain::Range range=last_payload->getRange(d);
	float Gain=0;
	float nAPV=0;
	for(int it=0;it<range.second-range.first;it++){
	  nAPV+=1;
	  Gain+=last_payload->getApvGain(it,range);
	  std::pair<uint32_t,int> index = std::make_pair(d,nAPV);
	  lastmap[index]=(Gain/nAPV);
	} // loop over APVs
      } // loop over detIds
      
      detid.clear();
      
      first_payload->getDetIds(detid);
      
      // cache the first IOV
      for (const auto & d : detid) {
	SiStripApvGain::Range range=first_payload->getRange(d);
	float Gain=0;
	float nAPV=0;
	for(int it=0;it<range.second-range.first;it++){
	  nAPV+=1;
	  Gain+=first_payload->getApvGain(it,range);
	  std::pair<uint32_t,int> index = std::make_pair(d,nAPV);
	  firstmap[index]=(Gain/nAPV);
	} // loop over APVs
      } // loop over detIds
      
      // find the largest deviation
      std::map<uint32_t,float> cachedRatio; 

      for(const auto &item : firstmap ){
	
	// packed index (detid,APV)
	auto index   = item.first;
	auto mod     = item.first.first;
	
	float ratio = firstmap[index]/lastmap[index];
	// if we have already cached something
	if(cachedRatio[mod]){
	  if(std::abs(cachedRatio[mod])>std::abs(ratio)){
	    cachedRatio[mod]=ratio;
	  }
	} else {
	  cachedRatio[mod]=ratio;
	}
      }

      for (const auto &element : cachedRatio){
	tmap->fill(element.first,element.second);
      }

      // get the range of the TrackerMap (saturate at +/-2 std deviations)
      auto range = getTheRange(cachedRatio);
      
      //=========================
      
      std::string fileName(m_imageFileName);
      tmap->save(true,range.first,range.second,fileName.c_str());

      return true;
    }
  };

  /************************************************
    TrackerMap of SiStripApvGains (maximum gain per detid)
  *************************************************/
  class SiStripApvGainsMaximumTrackerMap : public cond::payloadInspector::PlotImage<SiStripApvGain> {
  public:
    SiStripApvGainsMaximumTrackerMap() : cond::payloadInspector::PlotImage<SiStripApvGain>( "Tracker Map of SiStripAPVGains (maximum per DetId)" ){
      setSingleIov( true );
    }
    
    bool fill( const std::vector<std::tuple<cond::Time_t,cond::Hash> >& iovs ){
      auto iov = iovs.front();
      std::shared_ptr<SiStripApvGain> payload = fetchPayload( std::get<1>(iov) );

      std::string titleMap = "SiStrip APV Gain maximum per module (payload : "+std::get<1>(iov)+")";

      std::unique_ptr<TrackerMap> tmap = std::unique_ptr<TrackerMap>(new TrackerMap("SiStripApvGains"));
      tmap->setTitle(titleMap.c_str());
      tmap->setPalette(1);
      
      std::vector<uint32_t> detid;
      payload->getDetIds(detid);
      
      for (const auto & d : detid) {
	SiStripApvGain::Range range=payload->getRange(d);
	float theMaxGain=0;
	for(int it=0;it<range.second-range.first;it++){
	  
	  float currentGain = payload->getApvGain(it,range);
	  if(currentGain > theMaxGain){
	    theMaxGain=currentGain;
	  }
	} // loop over APVs
	// fill the tracker map taking the average gain on a single DetId
	tmap->fill(d,theMaxGain);
      } // loop over detIds

      //=========================
      
      std::string fileName(m_imageFileName);
      tmap->save(true,0,0,fileName.c_str());

      return true;
    }
  };

  /************************************************
    TrackerMap of SiStripApvGains (minimum gain per detid)
  *************************************************/
  class SiStripApvGainsMinimumTrackerMap : public cond::payloadInspector::PlotImage<SiStripApvGain> {
  public:
    SiStripApvGainsMinimumTrackerMap() : cond::payloadInspector::PlotImage<SiStripApvGain>( "Tracker Map of SiStripAPVGains (minimum per DetId)" ){
      setSingleIov( true );
    }

    bool fill( const std::vector<std::tuple<cond::Time_t,cond::Hash> >& iovs ){
      auto iov = iovs.front();
      std::shared_ptr<SiStripApvGain> payload = fetchPayload( std::get<1>(iov) );

      std::string titleMap = "SiStrip APV Gain minumum per module (payload : "+std::get<1>(iov)+")";

      std::unique_ptr<TrackerMap> tmap = std::unique_ptr<TrackerMap>(new TrackerMap("SiStripApvGains"));
      tmap->setTitle(titleMap.c_str());
      tmap->setPalette(1);
      
      std::vector<uint32_t> detid;
      payload->getDetIds(detid);
      
      for (const auto & d : detid) {
	SiStripApvGain::Range range=payload->getRange(d);
	float theMinGain=999.;
	for(int it=0;it<range.second-range.first;it++){
	  float currentGain = payload->getApvGain(it,range);
	  if(currentGain < theMinGain){
	    theMinGain=currentGain;
	  }
	} // loop over APVs
	// fill the tracker map taking the average gain on a single DetId
	tmap->fill(d,theMinGain);
      } // loop over detIds

      //=========================
      
      std::string fileName(m_imageFileName);
      tmap->save(true,0,0,fileName.c_str());

      return true;
    }
  };


  /************************************************
    time history histogram of SiStripApvGains 
  *************************************************/

  class SiStripApvGainByRunMeans : public cond::payloadInspector::HistoryPlot<SiStripApvGain,float> {
  public:
    SiStripApvGainByRunMeans() : cond::payloadInspector::HistoryPlot<SiStripApvGain,float>( "SiStripApv Gains average","average Strip APV gain value"){}
    virtual ~SiStripApvGainByRunMeans() = default;

    float getFromPayload( SiStripApvGain& payload ){
     
      std::vector<uint32_t> detid;
      payload.getDetIds(detid);
      
      float nAPVs=0;
      float sumOfGains=0;

      for (const auto & d : detid) {
	SiStripApvGain::Range range=payload.getRange(d);
	for(int it=0;it<range.second-range.first;it++){
	  nAPVs+=1;
	  sumOfGains+=payload.getApvGain(it,range);
	} // loop over APVs
      } // loop over detIds

      return sumOfGains/nAPVs;      
    } // payload
  };

  /************************************************
    time history histogram of TIB SiStripApvGains 
  *************************************************/

  class SiStripApvTIBGainByRunMeans : public cond::payloadInspector::HistoryPlot<SiStripApvGain,float> {
  public:
    SiStripApvTIBGainByRunMeans() : cond::payloadInspector::HistoryPlot<SiStripApvGain,float>( "SiStripApv Gains average","average Tracker Inner Barrel APV gain value"){}
    virtual ~SiStripApvTIBGainByRunMeans() = default;

    float getFromPayload( SiStripApvGain& payload ){
     
      std::vector<uint32_t> detid;
      payload.getDetIds(detid);
      
      float nAPVs=0;
      float sumOfGains=0;

      for (const auto & d : detid) {

	int subid = DetId(d).subdetId();
	if(subid!=StripSubdetector::TIB) continue;
	
	SiStripApvGain::Range range=payload.getRange(d);
	for(int it=0;it<range.second-range.first;it++){
	  nAPVs+=1;
	  sumOfGains+=payload.getApvGain(it,range);
	} // loop over APVs
      } // loop over detIds

      return sumOfGains/nAPVs;

    } // payload
  };

  /************************************************
    time history histogram of TOB SiStripApvGains 
  *************************************************/

  class SiStripApvTOBGainByRunMeans : public cond::payloadInspector::HistoryPlot<SiStripApvGain,float> {
  public:
    SiStripApvTOBGainByRunMeans() : cond::payloadInspector::HistoryPlot<SiStripApvGain,float>( "SiStripApv Gains average","average Tracker Outer Barrel gain value"){}
    virtual ~SiStripApvTOBGainByRunMeans() = default;

    float getFromPayload( SiStripApvGain& payload ){
     
      std::vector<uint32_t> detid;
      payload.getDetIds(detid);
      
      float nAPVs=0;
      float sumOfGains=0;
      
      for (const auto & d : detid) {

	int subid = DetId(d).subdetId();
	if(subid!=StripSubdetector::TOB) continue;

	SiStripApvGain::Range range=payload.getRange(d);
	for(int it=0;it<range.second-range.first;it++){
	  nAPVs+=1;
	  sumOfGains+=payload.getApvGain(it,range);
	} // loop over APVs
      } // loop over detIds

      return sumOfGains/nAPVs;

    } // payload
  };

  /************************************************
    time history histogram of TID SiStripApvGains 
  *************************************************/

  class SiStripApvTIDGainByRunMeans : public cond::payloadInspector::HistoryPlot<SiStripApvGain,float> {
  public:
    SiStripApvTIDGainByRunMeans() : cond::payloadInspector::HistoryPlot<SiStripApvGain,float>( "SiStripApv Gains average","average Tracker Inner Disks APV gain value"){}
    virtual ~SiStripApvTIDGainByRunMeans() = default;

    float getFromPayload( SiStripApvGain& payload ){
     
      std::vector<uint32_t> detid;
      payload.getDetIds(detid);
      
      float nAPVs=0;
      float sumOfGains=0;
      for (const auto & d : detid) {
	
	int subid = DetId(d).subdetId();
	if(subid!=StripSubdetector::TID) continue;
	
	SiStripApvGain::Range range=payload.getRange(d);
	for(int it=0;it<range.second-range.first;it++){
	  nAPVs+=1;
	  sumOfGains+=payload.getApvGain(it,range);
	} // loop over APVs
      } // loop over detIds

      return sumOfGains/nAPVs;

    } // payload
  };

  /************************************************
    time history histogram of TEC SiStripApvGains 
  *************************************************/

  class SiStripApvTECGainByRunMeans : public cond::payloadInspector::HistoryPlot<SiStripApvGain,float> {
  public:
    SiStripApvTECGainByRunMeans() : cond::payloadInspector::HistoryPlot<SiStripApvGain,float>( "SiStripApv Gains average in TEC","average Tracker Endcaps APV gain value"){}
    virtual ~SiStripApvTECGainByRunMeans() = default;

    float getFromPayload( SiStripApvGain& payload ){
     
      std::vector<uint32_t> detid;
      payload.getDetIds(detid);
      
      float nAPVs=0;
      float sumOfGains=0;

      for (const auto & d : detid) {

	int subid = DetId(d).subdetId();
	if(subid!=StripSubdetector::TEC) continue;
	
	SiStripApvGain::Range range=payload.getRange(d);
	for(int it=0;it<range.second-range.first;it++){
	  nAPVs+=1;
	  sumOfGains+=payload.getApvGain(it,range);
	} // loop over APVs
      } // loop over detIds

      return sumOfGains/nAPVs;

    } // payload
  };

    
} // close namespace

// Register the classes as boost python plugin
PAYLOAD_INSPECTOR_MODULE(SiStripApvGain){
  PAYLOAD_INSPECTOR_CLASS(SiStripApvGainsValue);
  PAYLOAD_INSPECTOR_CLASS(SiStripApvBarrelGainsByLayer);
  PAYLOAD_INSPECTOR_CLASS(SiStripApvAbsoluteBarrelGainsByLayer);
  PAYLOAD_INSPECTOR_CLASS(SiStripApvEndcapMinusGainsByDisk);
  PAYLOAD_INSPECTOR_CLASS(SiStripApvEndcapPlusGainsByDisk);
  PAYLOAD_INSPECTOR_CLASS(SiStripApvAbsoluteEndcapMinusGainsByDisk);
  PAYLOAD_INSPECTOR_CLASS(SiStripApvAbsoluteEndcapPlusGainsByDisk);
  PAYLOAD_INSPECTOR_CLASS(SiStripApvGainsAverageTrackerMap);
  PAYLOAD_INSPECTOR_CLASS(SiStripApvGainsMaximumTrackerMap);
  PAYLOAD_INSPECTOR_CLASS(SiStripApvGainsMinimumTrackerMap);
  PAYLOAD_INSPECTOR_CLASS(SiStripApvGainsRatioWithPreviousIOVTrackerMap);
  PAYLOAD_INSPECTOR_CLASS(SiStripApvGainsRatioMaxDeviationWithPreviousIOVTrackerMap);
  PAYLOAD_INSPECTOR_CLASS(SiStripApvGainByRunMeans);
  PAYLOAD_INSPECTOR_CLASS(SiStripApvTIBGainByRunMeans);
  PAYLOAD_INSPECTOR_CLASS(SiStripApvTIDGainByRunMeans);
  PAYLOAD_INSPECTOR_CLASS(SiStripApvTOBGainByRunMeans);
  PAYLOAD_INSPECTOR_CLASS(SiStripApvTECGainByRunMeans);
}

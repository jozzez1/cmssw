// -*- C++ -*-
//
// Package:    DumpSimToTree/dumpSimToTree
// Class:      dumpSimToTree
// 
/**\class dumpSimToTree dumpSimToTree.cc DumpSimToTree/dumpSimToTree/plugins/dumpSimToTree.cc

 Description: Program is used to extract the Sim and Reco charges on the strips of the clusters.

 Implementation: Individual clusters with up to 6 Reco strips and 2 Sim strips are taken into
                 consideration. They can be then used to test the performance of cross-talk
                 inversion vs. neural network approach in the HSCP analysis.
*/
//
// Original Author:  Joze Zobec
//         Created:  Mon, 18 Apr 2016 14:01:38 GMT
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h" 
#include "FWCore/Framework/interface/ESHandle.h"

#include "SimTracker/SiStripDigitizer/plugins/SiHitDigitizer.h"
#include "SimTracker/SiStripDigitizer/plugins/SiTrivialInduceChargeOnStrips.h"

#include "SimDataFormats/TrackingHit/interface/PSimHit.h"
#include "SimDataFormats/TrackingHit/interface/PSimHitContainer.h"

#include <MagneticField/Engine/interface/MagneticField.h>
#include <MagneticField/Records/interface/IdealMagneticFieldRecord.h>

#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/CommonDetUnit/interface/GeomDetUnit.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "DataFormats/GeometrySurface/interface/BoundSurface.h"
#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/GeometryVector/interface/GlobalVector.h"
#include "DataFormats/SiStripCluster/interface/SiStripCluster.h"
#include "DataFormats/Common/interface/DetSetVectorNew.h"
#include "DataFormats/Common/interface/DetSetNew.h"

#include "DataFormats/GeometrySurface/interface/TrapezoidalPlaneBounds.h"
#include "DataFormats/GeometrySurface/interface/RectangularPlaneBounds.h"

#include "TTree.h"

#include "CommonTools/UtilAlgos/interface/TFileService.h"



class dumpSimToTree : public edm::one::EDAnalyzer<edm::one::SharedResources>  {
   public:
      explicit dumpSimToTree(const edm::ParameterSet&);
      ~dumpSimToTree();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);


   private:
      virtual void beginJob() override;
      virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
      virtual void endJob() override;

      size_t distance (size_t F1, size_t L1, size_t F2, size_t L2);

//      SiHitDigitizer* SiStripHitDigitizer;
      TTree* t1;

      double Sim1;
      double Sim2;
      double SimC;

      double Rec1;
      double Rec2;
      double Rec3;
      double Rec4;
      double Rec5;
      double Rec6;

      double MeVPerCM;
      double CorrectedMeV;
      double CorrectedSimC;
      double traversedPath;

      unsigned short interactionId;
      int            particleId;
      unsigned int   trackId;
      size_t         clusterSize;
      double         matchDistance;
      double         nearbyClusterDistance;
      unsigned int   numberOfNeighbours;
      uint32_t       rawId;
      uint16_t       firstStrip;

      std::vector<std::string> hitLabels;
      std::vector< edm::EDGetTokenT<edm::PSimHitContainer> > SimHitTokensV;
      edm::EDGetTokenT<edmNew::DetSetVector<SiStripCluster>> RecoClusterToken;
      edm::ESHandle <TrackerGeometry> tkGeom;
      edm::ESHandle <MagneticField>   MF;
      GlobalVector bfield;
      size_t minAffectedStrip;
      size_t maxAffectedStrip;
      std::vector<float> localCharges;
};


dumpSimToTree::dumpSimToTree(const edm::ParameterSet& iConfig)
{
   usesResource("TFileService");

   hitLabels.push_back ("g4SimHits:TrackerHitsTECHighTof");
   hitLabels.push_back ("g4SimHits:TrackerHitsTECLowTof");
   hitLabels.push_back ("g4SimHits:TrackerHitsTIBHighTof");
   hitLabels.push_back ("g4SimHits:TrackerHitsTIBLowTof");
   hitLabels.push_back ("g4SimHits:TrackerHitsTIDHighTof");
   hitLabels.push_back ("g4SimHits:TrackerHitsTIDLowTof");
   hitLabels.push_back ("g4SimHits:TrackerHitsTOBHighTof");
   hitLabels.push_back ("g4SimHits:TrackerHitsTOBLowTof");


   for (size_t i=0; i<hitLabels.size(); i++)
       SimHitTokensV.push_back(consumes <edm::PSimHitContainer>(edm::InputTag(hitLabels[i])));

   RecoClusterToken = consumes <edmNew::DetSetVector<SiStripCluster>>(edm::InputTag("siStripClusters"));
//   SiStripHitDigitizer = new SiHitDigitizer (iConfig);
}


dumpSimToTree::~dumpSimToTree()
{
//    delete SiStripHitDigitizer;
}

size_t
dumpSimToTree::distance (size_t F1, size_t L1, size_t F2, size_t L2){
    if (F1 <= F2 && F2 <= L2 && L2 <= L1) return 0;
    if (F2 <= F1 && F1 <= L2 && L2 <= L1) return 0;
    if (F1 <= F2 && F2 <= L1 && L1 <= L2) return 0;
    if (F2 <= L2 && L2 <= F1 && F1 <= L2) return F1 - L2;
    if (F1 <= L1 && L1 <= F2 && F2 <= L2) return F2 - L1;
    else return 99999;
}

void
dumpSimToTree::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   using namespace edm;



#ifdef THIS_IS_AN_EVENT_EXAMPLE
   Handle<ExampleData> pIn;
   iEvent.getByLabel("example",pIn);
#endif
   
#ifdef THIS_IS_AN_EVENTSETUP_EXAMPLE
   ESHandle<SetupData> pSetup;
   iSetup.get<SetupRecord>().get(pSetup);
#endif

   iSetup.get<TrackerDigiGeometryRecord>().get(tkGeom);

   iSetup.get<IdealMagneticFieldRecord>().get(MF);
//   const MagneticField* theMagneticField = MF.product();
            

   edm::Handle <edmNew::DetSetVector<SiStripCluster>> clusterDetSetHandle;
   iEvent.getByToken (RecoClusterToken, clusterDetSetHandle);
   if (!clusterDetSetHandle.isValid()){
       std::cerr << "Missing siStripClusters collection!" << std::endl;
       return;
   }
   edmNew::DetSetVector<SiStripCluster> detSetClusters = *clusterDetSetHandle.product();

   double pitchAtEntry = 1;
   double pitchAtExit  = 1;
   double length       = 1;
   double trapezeParam = 1;
   double width        = 1;
   unsigned int nstrips= 1;
   double SimHitStripPos = -1;

   // HERE WE LOOP OVER THE CLUSTERS FIRST AND TRY TO FIND A MATCHING PSimHit
   for (auto clusterSet = detSetClusters.begin(); clusterSet != detSetClusters.end(); clusterSet++){
       rawId = clusterSet->detId();
       edmNew::DetSet<SiStripCluster> RecoClusters = *clusterSet;

       for (auto cluster = RecoClusters.begin(); cluster != RecoClusters.end(); cluster++){
           PSimHit closestSimHit;
           matchDistance = 99999;
           size_t closestSimHitToken;
           bool foundClosestSimHit=false;

           for (size_t token_i=0; token_i<SimHitTokensV.size(); token_i++){
               edm::Handle <edm::PSimHitContainer> SimHitsHandle;
               iEvent.getByToken (SimHitTokensV[token_i], SimHitsHandle);
               if (!SimHitsHandle.isValid()) continue;
               std::vector<PSimHit> SimHits = *SimHitsHandle.product();
           
               for (auto SimHit = SimHits.begin(); SimHit != SimHits.end(); SimHit++){
                   if (SimHit->detUnitId() != rawId) continue;
                   DetId detid (SimHit->detUnitId());
                   GeomDet* detUnit = (GeomDet*) tkGeom->idToDetUnit(detid);
                   const BoundPlane plane = detUnit->surface();
                   const TrapezoidalPlaneBounds* trapezoidalBounds( dynamic_cast<const TrapezoidalPlaneBounds*>(&(plane.bounds())));
                   const RectangularPlaneBounds* rectangularBounds( dynamic_cast<const RectangularPlaneBounds*>(&(plane.bounds())));
                   
                   if(trapezoidalBounds){
                       std::array<const float, 4> const & parameters = (*trapezoidalBounds).parameters();
                       width            = (double) parameters[0]*2;
                       length           = (double) parameters[3]*2;
                       trapezeParam = parameters[1]/parameters[0];
                   }else if(rectangularBounds){
                       width            = (double) detUnit->surface().bounds().width();
                       length           = (double) detUnit->surface().bounds().length();
                       trapezeParam = 1;
                   }
                   
                   double correctedWidthAtEntryPoint =
                       trapezoidalBounds ? width * ((trapezeParam - 1)*(SimHit->entryPoint().y()/length) + 1) : width;
                   double correctedWidthAtExitPoint  = 
                       trapezoidalBounds ? width * ((trapezeParam - 1)*(SimHit-> exitPoint().y()/length) + 1) : width;
                   
                   const StripGeomDetUnit* DetUnit = dynamic_cast <const StripGeomDetUnit*> (detUnit);
                   if (DetUnit){
                       const StripTopology& Topo = DetUnit->specificTopology();
                       nstrips = Topo.nstrips();
                       pitchAtEntry = trapezoidalBounds ? correctedWidthAtEntryPoint/Topo.nstrips() : width/Topo.nstrips();
                       pitchAtExit  = trapezoidalBounds ? correctedWidthAtExitPoint /Topo.nstrips() : width/Topo.nstrips();
                   }
                   
                   //
                   // x           = stripNumber * pitch - width/2
                   // stripNumber = (x + width/2)/pitch
                   // 
                   
                   size_t firstSimStrip = (size_t) ((SimHit->entryPoint().x() + correctedWidthAtEntryPoint/2)/pitchAtEntry);
                   size_t lastSimStrip  = (size_t) ((SimHit->exitPoint(). x() + correctedWidthAtExitPoint /2)/pitchAtExit);
                   SimHitStripPos = 0.5 * (double) (firstSimStrip + lastSimStrip);
           
                   if (matchDistance > std::fabs(cluster->barycenter() - SimHitStripPos)){
                       matchDistance = std::fabs(cluster->barycenter() - SimHitStripPos);
                       closestSimHit = *SimHit;
                       closestSimHitToken = token_i;
                       foundClosestSimHit = true;
                   }
               } // end of main SimHit loop
           } // end of the main token loop

           // now we have to loop again to find nearby hits
//           std::cerr << "Closest SimHit is " << foundClosestSimHit << std::endl;
           if (foundClosestSimHit){
               particleId    = closestSimHit.particleType();
               interactionId = closestSimHit.processType();
               trackId       = closestSimHit.trackId();
               Local3DVector pathVector = closestSimHit.exitPoint() - closestSimHit.entryPoint();
               traversedPath = pathVector.x()*pathVector.x()
                             + pathVector.y()*pathVector.y()
                             + pathVector.z()*pathVector.z();
               traversedPath = sqrt(traversedPath);
               if (traversedPath < 0.032) continue;

               CorrectedMeV   = closestSimHit.energyLoss() * 1000;
               CorrectedSimC  = CorrectedMeV / (3.61e-6*265);
               SimC           = CorrectedSimC;
               MeVPerCM       = CorrectedMeV / traversedPath;

               nearbyClusterDistance = 999999;
               numberOfNeighbours    = 0;

               edm::Handle <edm::PSimHitContainer> SimHitsHandle;
               iEvent.getByToken (SimHitTokensV[closestSimHitToken], SimHitsHandle);
               if (!SimHitsHandle.isValid()) continue;
               std::vector<PSimHit> nearbySimHits = *SimHitsHandle.product();

               for (auto SimHit = nearbySimHits.begin(); SimHit != nearbySimHits.end(); SimHit++){
                   if (SimHit->detUnitId() != rawId) continue;

                   double closestX = 0.5*(closestSimHit.entryPoint().x() + closestSimHit.exitPoint().x());
                   double closestY = 0.5*(closestSimHit.entryPoint().y() + closestSimHit.exitPoint().y());
                   double nearbyX  = 0.5*(SimHit->entryPoint().x() + SimHit->exitPoint().x());
                   double nearbyY  = 0.5*(SimHit->entryPoint().y() + SimHit->exitPoint().y());
                   double distanceInCM = sqrt((closestX - nearbyX)*(closestX - nearbyX) + (closestY - nearbyY)*(closestY - nearbyY));
                   double correctedWidthAtEntryPoint =
                       (trapezeParam!=1) ? width * ((trapezeParam - 1)*(SimHit->entryPoint().y()/length) + 1) : width;
                   double correctedWidthAtExitPoint  = 
                       (trapezeParam!=1) ? width * ((trapezeParam - 1)*(SimHit-> exitPoint().y()/length) + 1) : width;
                   double correctedPitchAtEntryPoint = correctedWidthAtEntryPoint/nstrips;
                   double correctedPitchAtExitPoint  = correctedWidthAtExitPoint/nstrips;

                   double firstNearbyStrip = (SimHit->entryPoint().x() + correctedWidthAtEntryPoint/2)/correctedPitchAtEntryPoint;
                   double lastNearbyStrip  = (SimHit->exitPoint().x() + correctedWidthAtExitPoint/2)/correctedPitchAtExitPoint;
                   double distanceInStrip  = std::fabs(0.5*(firstNearbyStrip + lastNearbyStrip) - SimHitStripPos);
 
                   if (distanceInCM > 1e-10 && distanceInStrip < 2){
                       CorrectedMeV  += SimHit->energyLoss()*1000;
                       CorrectedSimC += (SimHit->energyLoss()*1000) / (3.61e-6 * 265);
                       numberOfNeighbours++;
                   }

                   if (distanceInCM > 1e-10 && distanceInStrip < nearbyClusterDistance) nearbyClusterDistance = distanceInStrip;
               } // end of nearby SimHit loop

               // all passed -- fill the tree
               clusterSize = cluster->amplitudes().size();
               // push the maximum to the center as much as possible
               int maxPos = static_cast<int> (cluster->barycenter() - cluster->firstStrip());
               std::vector<int> amps (6,0);
               if (cluster->amplitudes().size() <= 3){
                   int c=3;

                   for (int i=(int)maxPos; i >= 0; i--){
                       amps[c] = cluster->amplitudes()[i];
                       c--;
                   }
                   c = 4;
                   for (size_t i=(size_t)maxPos+1; i < cluster->amplitudes().size(); i++){
                       amps[c] = cluster->amplitudes()[i];
                       c++;
                   }
               }

               else if (cluster->amplitudes().size() == 4){
                   if (cluster->amplitudes()[0] < cluster->amplitudes()[1] && cluster->amplitudes()[3] < cluster->amplitudes()[2]){
                       int c=3;
                       for (int i=(int)maxPos; i >= 0; i--){
                           amps[c] = cluster->amplitudes()[i];
                           c--;
                       }
                       c = 4;
                       for (size_t i=(size_t)maxPos+1; i < cluster->amplitudes().size(); i++){
                           amps[c] = cluster->amplitudes()[i];
                           c++;
                       }
                   }
                   else if (cluster->amplitudes()[0] > cluster->amplitudes()[1]){
                       for (size_t i=0; i < cluster->amplitudes().size(); i++)
                           amps[2+i] = cluster->amplitudes()[i];
                   }
                   else if (cluster->amplitudes()[3] > cluster->amplitudes()[2]){
                       for (size_t i=0; i < cluster->amplitudes().size(); i++)
                           amps[3-i] = cluster->amplitudes()[3-i];
                   }
               }

               else if (cluster->amplitudes().size() == 5){
                   if (cluster->barycenter() >= 2.5)
                       for (size_t i=0; i < cluster->amplitudes().size(); i++)
                           amps[i+1] = cluster->amplitudes()[i];
                   else
                       for (size_t i=0; i < cluster->amplitudes().size(); i++)
                           amps[i] = cluster->amplitudes()[i];
               }

               else if (cluster->amplitudes().size() == 6){
                   for (size_t i=0; i < cluster->amplitudes().size(); i++)
                       amps[i] = cluster->amplitudes()[i];
               }

	       firstStrip = cluster->firstStrip();
                
               Rec1 = amps[0];
               Rec2 = amps[1];
               Rec3 = amps[2];
               Rec4 = amps[3];
               Rec5 = amps[4];
               Rec6 = amps[5];
               t1->Fill();
           }
       } // end of clusters loop
   } // end of rawId loop

/* HERE WE LOOP OVER THE HITS FIRST AND FIND A MATCHING CLUSTER
   for (size_t label_i=0; label_i < hitLabels.size(); label_i++){
       edm::Handle <edm::PSimHitContainer> SimHitsHandle;
       iEvent.getByToken (SimHitTokensV[label_i], SimHitsHandle);
       if (!SimHitsHandle.isValid()){
           std::cerr << "Missing " << hitLabels[label_i] << " collection!" << std::endl;
           continue;
       }

       std::vector <PSimHit> SimHits = *SimHitsHandle.product();

       for (size_t hit_i=0; hit_i < SimHits.size(); hit_i++){
           DetId detid (SimHits[hit_i].detUnitId());
           rawId = detid.rawId();

           if (!detSetClusters.exists(rawId)) continue;
           edmNew::DetSet<SiStripCluster> RecoClusters = detSetClusters [rawId];

           particleId    = SimHits[hit_i].particleType();
           interactionId = SimHits[hit_i].processType();

//           if (std::abs(particleId) != 13) continue; // only take (anti-)muons

           Local3DVector pathVector = SimHits[hit_i].exitPoint() - SimHits[hit_i].entryPoint();
           traversedPath = pathVector.x()*pathVector.x()
                         + pathVector.y()*pathVector.y()
                         + pathVector.z()*pathVector.z();
           traversedPath = sqrt(traversedPath); // <-- in cm
           if (traversedPath < 0.032) continue;


           GeomDet* detUnit = (GeomDet*) tkGeom->idToDetUnit(detid);
           bfield = theMagneticField->inInverseGeV(detUnit->toGlobal(SimHits[hit_i].localPosition()));

           const BoundPlane plane = detUnit->surface();
           const TrapezoidalPlaneBounds* trapezoidalBounds( dynamic_cast<const TrapezoidalPlaneBounds*>(&(plane.bounds())));
           const RectangularPlaneBounds* rectangularBounds( dynamic_cast<const RectangularPlaneBounds*>(&(plane.bounds())));

           if(trapezoidalBounds){
               std::array<const float, 4> const & parameters = (*trapezoidalBounds).parameters();
               width            = (double) parameters[0]*2;
               length           = (double) parameters[3]*2;
               trapezeParam = parameters[1]/parameters[0];
           }else if(rectangularBounds){
               width            = (double) detUnit->surface().bounds().width();
               length           = (double) detUnit->surface().bounds().length();
               trapezeParam = 1;
           }

           double correctedWidthAtEntryPoint = trapezoidalBounds ? width * ((trapezeParam - 1)*(SimHits[hit_i].entryPoint().y()/length) + 1)
                                                                 : width;
           double correctedWidthAtExitPoint  = trapezoidalBounds ? width * ((trapezeParam - 1)*(SimHits[hit_i]. exitPoint().y()/length) + 1)
                                                                 : width;

           const StripGeomDetUnit* DetUnit = dynamic_cast <const StripGeomDetUnit*> (detUnit);
           if (DetUnit){
               const StripTopology& Topo = DetUnit->specificTopology();
               pitchAtEntry = trapezoidalBounds ? correctedWidthAtEntryPoint/Topo.nstrips() : width/Topo.nstrips();
               pitchAtExit  = trapezoidalBounds ? correctedWidthAtExitPoint /Topo.nstrips() : width/Topo.nstrips();
           }

           //
           // x           = stripNumber * pitch - width/2
           // stripNumber = (x + width/2)/pitch
           // 

           size_t firstSimStrip = (size_t) ((SimHits[hit_i].entryPoint().x() + correctedWidthAtEntryPoint/2)/pitchAtEntry);
           size_t lastSimStrip  = (size_t) ((SimHits[hit_i].exitPoint(). x() + correctedWidthAtExitPoint /2)/pitchAtExit);
           double SimHitStripPos = 0.5 * (double) (firstSimStrip + lastSimStrip);

           if (firstSimStrip > lastSimStrip){
               size_t tmp    = lastSimStrip;
               lastSimStrip  = firstSimStrip;
               firstSimStrip = tmp;
           }

           //                           [ GeV] >> [MeV] >> [MeV/cm]
           MeVPerCM = (SimHits[hit_i].energyLoss()*1000) / traversedPath;
           //                           [ GeV] >> [MeV] >> [ADC]
           SimC     = (SimHits[hit_i].energyLoss()*1000) / (3.61e-6*265);

           if (MeVPerCM < 1) continue;
           bool foundACandidateMatch = false;
           auto closestCluster = RecoClusters.begin();
           matchDistance = std::fabs(closestCluster->barycenter() - SimHitStripPos);
           for (auto cluster = RecoClusters.begin()+1; cluster != RecoClusters.end(); cluster++){
               if (matchDistance > std::fabs(cluster->barycenter() - SimHitStripPos)){
                   matchDistance  = std::fabs(cluster->barycenter() - SimHitStripPos);
                   closestCluster = cluster;

                   if (matchDistance <= 5 && cluster->amplitudes().size() <= 6 && cluster->amplitudes().size() > 0)
                       foundACandidateMatch = true;
//                   else if (foundACandidateMatch && cluster->amplitudes().size() <= 6)
//                       foundTwoPSimHits = true;
               }
           }

           if (foundACandidateMatch && !foundTwoPSimHits){
               clusterSize = closestCluster->amplitudes().size();
               // push the maximum to the center as much as possible
               int maxPos = static_cast<int> (closestCluster->barycenter() - closestCluster->firstStrip());
               std::vector<int> amps (6,0);
               if (closestCluster->amplitudes().size() <= 3){
                   int c=3;

                   for (int i=(int)maxPos; i >= 0; i--){
                       amps[c] = closestCluster->amplitudes()[i];
                       c--;
                   }
                   c = 4;
                   for (size_t i=(size_t)maxPos+1; i < closestCluster->amplitudes().size(); i++){
                       amps[c] = closestCluster->amplitudes()[i];
                       c++;
                   }
               }

               else if (closestCluster->amplitudes().size() == 4){
                   if (closestCluster->amplitudes()[0] < closestCluster->amplitudes()[1] && closestCluster->amplitudes()[3] < closestCluster->amplitudes()[2]){
                       int c=3;
                       for (int i=(int)maxPos; i >= 0; i--){
                           amps[c] = closestCluster->amplitudes()[i];
                           c--;
                       }
                       c = 4;
                       for (size_t i=(size_t)maxPos+1; i < closestCluster->amplitudes().size(); i++){
                           amps[c] = closestCluster->amplitudes()[i];
                           c++;
                       }
                   }
                   else if (closestCluster->amplitudes()[0] > closestCluster->amplitudes()[1]){
                       for (size_t i=0; i < closestCluster->amplitudes().size(); i++)
                           amps[2+i] = closestCluster->amplitudes()[i];
                   }
                   else if (closestCluster->amplitudes()[3] > closestCluster->amplitudes()[2]){
                       for (size_t i=0; i < closestCluster->amplitudes().size(); i++)
                           amps[3-i] = closestCluster->amplitudes()[3-i];
                   }
               }

               else if (closestCluster->amplitudes().size() == 5){
                   if (closestCluster->barycenter() >= 2.5)
                       for (size_t i=0; i < closestCluster->amplitudes().size(); i++)
                           amps[i+1] = closestCluster->amplitudes()[i];
                   else
                       for (size_t i=0; i < closestCluster->amplitudes().size(); i++)
                           amps[i] = closestCluster->amplitudes()[i];
               }

               else if (closestCluster->amplitudes().size() == 6){
                   for (size_t i=0; i < closestCluster->amplitudes().size(); i++)
                       amps[i] = closestCluster->amplitudes()[i];
               }
                
               Rec1 = amps[0];
               Rec2 = amps[1];
               Rec3 = amps[2];
               Rec4 = amps[3];
               Rec5 = amps[4];
               Rec6 = amps[5];
               t1->Fill();
           }

//           SiStripHitDigitizer->processHit (SimHits[hit_i], dynamic_cast<const StripGeomDetUnit*> detUnit, bfield, langle,
//                   localCharges, firstChannel, lastChannel, tTopo, engine);
       } // end hit loop

   } // end hit label loop
*/
}


void 
dumpSimToTree::beginJob()
{
    edm::Service<TFileService> tfs; // do not forget to call the file service in the python file
    t1 = tfs->make<TTree> ("T", "Neural Network Training Tree");

    t1->Branch ("Sim1", &Sim1, "Sim1/D");
    t1->Branch ("Sim2", &Sim1, "Sim2/D");
    t1->Branch ("SimC", &SimC, "SimC/D");
    t1->Branch ("Rec1", &Rec1, "Rec1/D");
    t1->Branch ("Rec2", &Rec2, "Rec2/D");
    t1->Branch ("Rec3", &Rec3, "Rec3/D");
    t1->Branch ("Rec4", &Rec4, "Rec4/D");
    t1->Branch ("Rec5", &Rec5, "Rec5/D");
    t1->Branch ("Rec6", &Rec6, "Rec6/D");
    t1->Branch ("MeVPerCM",      &MeVPerCM,      "MeVPerCM/D");
    t1->Branch ("path",          &traversedPath, "path/D");
    t1->Branch ("rawId",         &rawId,         "rawId/i");
    t1->Branch ("firstStrip",    &firstStrip,    "firstStrip/s");
    t1->Branch ("interactionID", &interactionId, "interactionID/s");
    t1->Branch ("particleID",    &particleId,    "particleID/I");
    t1->Branch ("matchDistance", &matchDistance, "matchDistance/D");
    t1->Branch ("clusterSize",   &clusterSize,   "clusterSize/i");
    t1->Branch ("correctedMeV",  &CorrectedMeV,  "correctedMeV/D");
    t1->Branch ("correctedSimC", &CorrectedSimC, "correctedSimC/D");
    t1->Branch ("distanceToClosestNeighbour", &nearbyClusterDistance, "distanceToClosestNeighbour/D");
    t1->Branch ("NumOfNeighbours", &numberOfNeighbours, "NumOfNeighbours/i");
    t1->Branch ("trackId", &trackId, "trackId/i")
}


void 
dumpSimToTree::endJob() 
{
}


void
dumpSimToTree::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}


DEFINE_FWK_MODULE(dumpSimToTree);

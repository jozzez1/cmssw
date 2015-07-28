// Original Author:  Loic Quertenmont

#include "Analysis_Global.h"
#include "Analysis_PlotFunction.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////// 
// general purpose code 

// return the TypeMode from a string inputPattern
int TypeFromPattern(const std::string& InputPattern){
   if(InputPattern.find("Type0",0)<std::string::npos){       return 0;
   }else if(InputPattern.find("Type1",0)<std::string::npos){ return 1;
   }else if(InputPattern.find("Type2",0)<std::string::npos){ return 2;
   }else if(InputPattern.find("Type3",0)<std::string::npos){ return 3;
   }else if(InputPattern.find("Type4",0)<std::string::npos){ return 4;
   }else if(InputPattern.find("Type5",0)<std::string::npos){ return 5;
   }else{                                                    return 6;
   }
}

// define the legend corresponding to a Type
std::string LegendFromType(const std::string& InputPattern){
   switch(TypeFromPattern(InputPattern)){
      case 0:  return std::string("Tracker - Only"); break;
      case 1:  return std::string("Tracker + Muon"); break;
      case 2:  return std::string("Tracker + TOF" ); break;
      case 3:  return std::string("Muon - Only"); break;
      case 4:  return std::string("|Q|>1e"); break;
      case 5:  return std::string("|Q|<1e"); break;
      default : std::string("unknown");
   }
   return std::string("unknown");
}

// compute deltaR between two point (eta,phi) (eta,phi)
double deltaR(double eta1, double phi1, double eta2, double phi2) {
   double deta = eta1 - eta2;
   double dphi = phi1 - phi2;
   while (dphi >   M_PI) dphi -= 2*M_PI;
   while (dphi <= -M_PI) dphi += 2*M_PI;
   return sqrt(deta*deta + dphi*dphi);
}

// function to go form a TH3 plot with cut index on the x axis to a  TH2
TH2D* GetCutIndexSliceFromTH3(TH3D* tmp, unsigned int CutIndex, string Name="zy"){
   tmp->GetXaxis()->SetRange(CutIndex+1,CutIndex+1);
   return (TH2D*)tmp->Project3D(Name.c_str());
}

// function to go form a TH2 plot with cut index on the x axis to a  TH1
TH1D* GetCutIndexSliceFromTH2(TH2D* tmp, unsigned int CutIndex, string Name="_py"){
   return tmp->ProjectionY(Name.c_str(),CutIndex+1,CutIndex+1);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////// 
// Genertic code for beta/mass reconstruction starting from p&dEdx or p&TOF

// compute mass out of a beta and momentum value
double GetMassFromBeta(double P, double beta){
   double gamma = 1/sqrt(1-beta*beta);
   return P/(beta*gamma);
} 

// compute mass out of a momentum and tof value
double GetTOFMass(double P, double TOF){
   return GetMassFromBeta(P, 1/TOF);
}

// estimate beta from a dEdx value, if dEdx is below the allowed threshold returns a negative beta value
double GetIBeta(double I, bool MC){
   double& K = dEdxK_Data;
   double& C = dEdxC_Data;
   if(MC){ K = dEdxK_MC;
           C = dEdxC_MC;  }

   double a = K / (I-C);
   double b2 = a / (a+1);
   if(b2<0)return -1*sqrt(b2);
   return sqrt(b2);
}

// compute mass out of a momentum and dEdx value
double GetMass(double P, double I, bool MC){
   double& K = dEdxK_Data;
   double& C = dEdxC_Data;
   if(MC){ K = dEdxK_MC;
           C = dEdxC_MC;  }

   if(I-C<0)return -1;
   return sqrt((I-C)/K)*P;
}


// return a TF1 corresponding to a mass line in the momentum vs dEdx 2D plane
TF1* GetMassLine(double M, bool MC){
   double& K = dEdxK_Data;
   double& C = dEdxC_Data;
   if(MC){ K = dEdxK_MC;
           C = dEdxC_MC;  }

   double BetaMax = 0.9;
   double PMax = sqrt((BetaMax*BetaMax*M*M)/(1-BetaMax*BetaMax));

   double BetaMin = 0.2;
   double PMin = sqrt((BetaMin*BetaMin*M*M)/(1-BetaMin*BetaMin));

   TF1* MassLine = new TF1("MassLine","[2] + ([0]*[0]*[1])/(x*x)", PMin, PMax);
   MassLine->SetParName  (0,"M");
   MassLine->SetParName  (1,"K");
   MassLine->SetParName  (2,"C");
   MassLine->SetParameter(0, M);
   MassLine->SetParameter(1, K);
   MassLine->SetParameter(2, C);
   MassLine->SetLineWidth(2);
   return MassLine;
}


TF1* GetMassLineQ(double M, double Charge=1, bool MC=false)
{
   double K;   double C;
   if(MC){
      K = dEdxK_MC;
      C = dEdxC_MC;
   }else{ 
      K = dEdxK_Data;
      C = dEdxC_Data;
   }

   double BetaMax = 0.999;
   double PMax = sqrt((BetaMax*BetaMax*M*M)/(1-BetaMax*BetaMax));

   double BetaMin = 0.01;
   double PMin = sqrt((BetaMin*BetaMin*M*M)/(1-BetaMin*BetaMin));

   TF1* MassLine = new TF1("MassLine","[3] * ([2] + ([0]*[0]*[1])/(x*x*[3]))", PMin, PMax);
   MassLine->SetParName  (0,"M");
   MassLine->SetParName  (1,"K");
   MassLine->SetParName  (2,"C");
   MassLine->SetParName  (3,"z2");
   MassLine->SetParameter(0, M);
   MassLine->SetParameter(1, K);
   MassLine->SetParameter(2, C);
   MassLine->SetParameter(3, Charge*Charge);
   MassLine->SetLineWidth(2);
   return MassLine;
}







////////////////////////////////////////////////////////////////////////////////////////////////////////// 
// Functions below were used for the 2009 and 2010 papers, but are probably not used anymore 

// return the selection efficiency for a given disribution (TH1) and for a given cut... Nothing but the integral above the cut divided by the number of events
double Efficiency(TH1* Histo, double CutX){
   double Entries  = Histo->Integral(0,Histo->GetNbinsX()+1);
   double Integral = Histo->Integral(Histo->GetXaxis()->FindBin(CutX),Histo->GetNbinsX()+1);
   return Integral/Entries;
}

// same as before but also compute the error in the efficiency
double EfficiencyAndError(TH1* Histo, double CutX, double& error){
   double Entries  = Histo->Integral(0,Histo->GetNbinsX()+1);
   double Integral = 0;
          error    = 0;
   for(Int_t binx = Histo->GetXaxis()->FindBin(CutX); binx<= Histo->GetNbinsX()+1; ++binx){
      Integral += Histo->GetBinContent(binx);
      error    += Histo->GetBinError(binx)*Histo->GetBinError(binx);
   }
   error = sqrt(error);
   error /= Entries;
   return Integral/Entries;
}

// return the selection efficiency for a given disribution (TH2) and for a given recangular signal region ... Nothing but the integral above the cut divided by the number of events
double Efficiency(TH2* Histo, double CutX, double CutY){
   double Entries  = Histo->Integral(0,Histo->GetNbinsX()+1, 0,Histo->GetNbinsY()+1);
   double Integral = Histo->Integral(Histo->GetXaxis()->FindBin(CutX),Histo->GetNbinsX()+1, Histo->GetYaxis()->FindBin(CutY),Histo->GetNbinsY()+1);
   return Integral/Entries;
}

// return the number of entry in an histogram (and it's error) in a window defined by two cuts
double GetEventInRange(double min, double max, TH1D* hist, double& error){
  int binMin = hist->GetXaxis()->FindBin(min);
  int binMax = hist->GetXaxis()->FindBin(max);
  error = 0; for(int i=binMin;i<binMax;i++){ error += pow(hist->GetBinError(i),2); }  error = sqrt(error);
  return hist->Integral(binMin,binMax);
}

// not used anymore, was computing a scale factor between datadriven prediction and observation using the M<75GeV events
void GetPredictionRescale(std::string InputPattern, double& Rescale, double& RMS, bool ForceRecompute=false)
{
   size_t CutIndex = InputPattern.find("/Type");
   InputPattern    = InputPattern.substr(0,CutIndex+7);
   std::string Input    = InputPattern + "PredictionRescale.txt";


   FILE* pFile = fopen(Input.c_str(),"r");
   if(pFile && !ForceRecompute){
      float tmp1, tmp2;
      fscanf(pFile,"Rescale=%f RMS=%f\n",&tmp1,&tmp2);
      Rescale = tmp1;
      RMS = tmp2;
      fclose(pFile);
   }else{
      Rescale = 0;
      RMS     = 0;
      int    NPoints = 0;

      std::vector<double> DValue;
      std::vector<double> PValue;
  
      for(float WP_Pt=0;WP_Pt>=-5;WP_Pt-=0.5f){
      for(float WP_I =0;WP_I >=-5;WP_I -=0.5f){
         char Buffer[2048];
         sprintf(Buffer,"%sWPPt%+03i/WPI%+03i/DumpHistos.root",InputPattern.c_str(),(int)(10*WP_Pt),(int)(10*WP_I));
         TFile* InputFile = new TFile(Buffer); 
         if(!InputFile || InputFile->IsZombie() || !InputFile->IsOpen() || InputFile->TestBit(TFile::kRecovered) )continue;

         double d=0, p=0;//, m=0;
         double error =0;
         TH1D* Hd = (TH1D*)GetObjectFromPath(InputFile, "Mass_Data");if(Hd){d=GetEventInRange(0,75,Hd,error);delete Hd;}
         TH1D* Hp = (TH1D*)GetObjectFromPath(InputFile, "Mass_Pred");if(Hp){p=GetEventInRange(0,75,Hp,error);delete Hp;}
//       TH1D* Hm = (TH1D*)GetObjectFromPath(InputFile, "Mass_MCTr");if(Hm){m=GetEventInRange(0,75,Hm);delete Hm;}

//       if(!(d!=d) && p>0 && d>10 && (WP_Pt+WP_I)<=-3){
//         if(!(d!=d) && p>0 && d>20 && (WP_Pt+WP_I)<=-3){
         if(!(d!=d) && p>0 && d>20 && (WP_Pt+WP_I)<=-2){
//         if(!(d!=d) && p>0 && d>500 && (WP_Pt+WP_I)<=-2){
            DValue.push_back(d);
            PValue.push_back(p);
            printf("%6.2f %6.2f (eff=%6.2E) --> %f  (d=%6.2E)\n",WP_Pt,WP_I, pow(10,WP_Pt+WP_I),d/p, d);
            Rescale += (d/p);
            NPoints++;
         }
         InputFile->Close();
      }}
      printf("----------------------------\n");
      Rescale /= NPoints;

      for(unsigned int i=0;i<DValue.size();i++){
          RMS += pow( (DValue[i]/(PValue[i]*Rescale)) - 1.0 ,2);
      }
      RMS /= NPoints;
      RMS = sqrt(RMS);

      pFile = fopen(Input.c_str(),"w");
      if(!pFile)return;
      fprintf(pFile,"Rescale=%6.2f RMS=%6.2f\n",Rescale,RMS);
      fclose(pFile);
   }
   printf("Mean Rescale = %f   RMS = %f\n",Rescale, RMS);
}

// find the intersection between two graphs... very useful to know what is the excluded mass range from an observed xsection limit and theoretical xsection
double FindIntersectionBetweenTwoGraphs(TGraph* obs, TGraph* th, double Min, double Max, double Step, double ThUncertainty=0, bool debug=false){

   double Intersection = -1;
   double ThShift = 1.0-ThUncertainty;
   double PreviousX = Min;
   double PreviousV = obs->Eval(PreviousX, 0, "") - (ThShift * th->Eval(PreviousX, 0, "")) ;
   if(PreviousV>0){if(debug){printf("FindIntersectionBetweenTwoGraphs returns -1 because observed xsection is above th xsection for the first mass already : %f vs %f\n", obs->Eval(PreviousX, 0, ""), th->Eval(PreviousX, 0, ""));}return -2;}
   for(double x=Min+=Step;x<Max;x+=Step){                 
      double V = obs->Eval(x, 0, "") - (ThShift * th->Eval(x, 0, "") );
      if(debug){
         printf("%7.2f --> Obs=%6.2E  Th=%6.2E",x,obs->Eval(x, 0, ""),ThShift * th->Eval(x, 0, ""));
         if(V>=0)printf("   X\n");
         else printf("\n");
      }
      if(V<0){
         PreviousX = x;
         PreviousV = V;
      }else{
         Intersection = PreviousX;
      }
   }
   if(Intersection!=-1)return Intersection;
   return PreviousV>0 ? Intersection : -1*Max;
}


// find the range of excluded masses. Necessary when low mass not excluded but higher masses are
void FindRangeBetweenTwoGraphs(TGraph* obs, TGraph* th, double Min, double Max, double Step, double ThUncertainty, double& minExclMass, double& maxExclMass){

   double ThShift = 1.0-ThUncertainty;
   double PreviousX = Min;
   double PreviousV = obs->Eval(PreviousX, 0, "") - (ThShift * th->Eval(PreviousX, 0, "")) ;
   bool LowMassNeeded=true;
   if(PreviousV<0) LowMassNeeded=false;
   for(double x=Min+=Step;x<Max;x+=Step){                 
      double V = obs->Eval(x, 0, "") - (ThShift * th->Eval(x, 0, "") );
      if(LowMassNeeded && V<0) {
	minExclMass=x;
	LowMassNeeded=false;
      }
      if(V<0){
         PreviousX = x;
         PreviousV = V;
      }else{
         maxExclMass = PreviousX;
      }
   }
   return;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Genertic code related to samples processing in FWLITE --> functions below will be loaded only if FWLITE compiler variable is defined

#ifdef FWLITE
double DistToHSCP        (const susybsm::HSCParticle& hscp, const std::vector<reco::GenParticle>& genColl, int& IndexOfClosest);
int    HowManyChargedHSCP(const std::vector<reco::GenParticle>& genColl);
double FastestHSCP       (const fwlite::ChainEvent& ev);
void   GetGenHSCPBeta    (const std::vector<reco::GenParticle>& genColl, double& beta1, double& beta2, bool onlyCharged=true);

// compute the distance between a "reconstructed" HSCP candidate and the closest geenrated HSCP
double DistToHSCP (const susybsm::HSCParticle& hscp, const std::vector<reco::GenParticle>& genColl, int& IndexOfClosest){
   reco::TrackRef   track;
   if(TypeMode!=3) track = hscp.trackRef();
   else {
     reco::MuonRef muon = hscp.muonRef();
     if(muon.isNull()) return false;
     track = muon->standAloneMuon();
   }
   if(track.isNull())return false;

   double RMin = 9999; IndexOfClosest=-1;
   for(unsigned int g=0;g<genColl.size();g++){
      if(genColl[g].pt()<5)continue;
      if(genColl[g].status()!=1)continue;
      int AbsPdg=abs(genColl[g].pdgId());
      if(AbsPdg<1000000 && AbsPdg!=17)continue;    
      double dR = deltaR(track->eta(), track->phi(), genColl[g].eta(), genColl[g].phi());
      if(dR<RMin){RMin=dR;IndexOfClosest=g;}
   }
   return RMin;
}

// count the number of charged generated HSCP in the event --> this is needed to reweights the events for different gluino ball fraction starting from f=10% samples
int HowManyChargedHSCP (const std::vector<reco::GenParticle>& genColl){
   int toReturn = 0;
   for(unsigned int g=0;g<genColl.size();g++){
      if(genColl[g].pt()<5)continue;
      if(genColl[g].status()!=1)continue;
      int AbsPdg=abs(genColl[g].pdgId());
      if(AbsPdg<1000000 && AbsPdg!=17)continue;
      if(AbsPdg==1000993 || AbsPdg==1009313 || AbsPdg==1009113 || AbsPdg==1009223 || AbsPdg==1009333 || AbsPdg==1092114 || AbsPdg==1093214 || AbsPdg==1093324)continue; //Skip neutral gluino RHadrons
      if(AbsPdg==1000622 || AbsPdg==1000642 || AbsPdg==1006113 || AbsPdg==1006311 || AbsPdg==1006313 || AbsPdg==1006333)continue;  //skip neutral stop RHadrons
      toReturn++;
   }
   return toReturn;
}

// returns the generated beta of the two firsts HSCP in the events
void  GetGenHSCPBeta (const std::vector<reco::GenParticle>& genColl, double& beta1, double& beta2, bool onlyCharged){
   beta1=-1; beta2=-1;
   for(unsigned int g=0;g<genColl.size();g++){
      if(genColl[g].pt()<5)continue;
      if(genColl[g].status()!=1)continue;
      int AbsPdg=abs(genColl[g].pdgId());
      if(AbsPdg<1000000 && AbsPdg!=17)continue;
      if(onlyCharged && (AbsPdg==1000993 || AbsPdg==1009313 || AbsPdg==1009113 || AbsPdg==1009223 || AbsPdg==1009333 || AbsPdg==1092114 || AbsPdg==1093214 || AbsPdg==1093324))continue; //Skip neutral gluino RHadrons
      if(onlyCharged && (AbsPdg==1000622 || AbsPdg==1000642 || AbsPdg==1006113 || AbsPdg==1006311 || AbsPdg==1006313 || AbsPdg==1006333))continue;  //skip neutral stop RHadrons
      if(beta1<0){beta1=genColl[g].p()/genColl[g].energy();}else if(beta2<0){beta2=genColl[g].p()/genColl[g].energy();return;}
   }
}

double FastestHSCP(const fwlite::ChainEvent& ev){
   //get the collection of generated Particles
   fwlite::Handle< std::vector<reco::GenParticle> > genCollHandle;
   genCollHandle.getByLabel(ev, "genParticlesSkimmed");
   if(!genCollHandle.isValid()){
      genCollHandle.getByLabel(ev, "genParticles");
      if(!genCollHandle.isValid()){printf("GenParticle Collection NotFound\n");return -1;}
   }

   std::vector<reco::GenParticle> genColl = *genCollHandle;

   double MaxBeta=-1;
   for(unsigned int g=0;g<genColl.size();g++){
      if(genColl[g].pt()<5)continue;
      if(genColl[g].status()!=1)continue;
      int AbsPdg=abs(genColl[g].pdgId());
      if(AbsPdg<1000000 && AbsPdg!=17)continue;

      double beta=genColl[g].p()/genColl[g].energy();
      if(MaxBeta<beta)MaxBeta=beta;
   }
   return MaxBeta;
}

#include "FWCore/Utilities/interface/RegexMatch.h"
bool passTriggerPatterns(edm::TriggerResultsByName& tr, std::string pattern){
  if(edm::is_glob(pattern)){
     std::vector< std::vector<std::string>::const_iterator > matches = edm::regexMatch(tr.triggerNames(), pattern);
     for(size_t t=0;t<matches.size();t++){
        if(tr.accept( matches[t]->c_str() ) )return true;
     }
  }else{
     if(tr.accept( pattern.c_str() ) ) return true;
  }
  return false;
}


#include "TVector3.h"
double deltaROpositeTrack(const susybsm::HSCParticleCollection& hscpColl, const susybsm::HSCParticle& hscp){
   reco::TrackRef track1=hscp.trackRef();

   double maxDr=-0.1;
   for(unsigned int c=0;c<hscpColl.size();c++){
      reco::TrackRef track2;
      if(!hscpColl[c].trackRef().isNull()){
         track2=hscpColl[c].trackRef();
      }else if(!hscpColl[c].muonRef().isNull() && hscpColl[c].muonRef()->combinedQuality().updatedSta){
         track2= hscpColl[c].muonRef()->standAloneMuon();
      }else{
         continue;
      }

      if(fabs(track1->pt()-track2->pt())<1 && deltaR(track1->eta(), track1->phi(), track2->eta(), track2->phi())<0.1)continue; //Skip same tracks
//      double dR = deltaR(-1*track1->eta(), M_PI+track1->phi(), track2->eta(), track2->phi());
      TVector3 v1 = TVector3(track1->momentum().x(), track1->momentum().y(), track1->momentum().z());
      TVector3 v2 = TVector3(track2->momentum().x(), track2->momentum().y(), track2->momentum().z());
      double dR = v1.Angle(v2);
      if(dR>maxDr)maxDr=dR;
   }
   return maxDr;
}

#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Handfull class to check for duplicated events

class DuplicatesClass{
   private :
      typedef std::map<std::pair<unsigned int, unsigned int>, int > RunEventHashMap;
      RunEventHashMap map;
   public :
        DuplicatesClass(){}
        ~DuplicatesClass(){}
        void Clear(){map.clear();}
        bool isDuplicate(unsigned int Run, unsigned int Event){
	   RunEventHashMap::iterator it = map.find(std::make_pair(Run,Event));
           if(it==map.end()){
   	      map[std::make_pair(Run,Event)] = 1;
              return false;
           }else{
              map[std::make_pair(Run,Event)]++;
           }
           return true;
        }

        void printDuplicate(){
           printf("Duplicate event summary:\n##########################################");
           for(RunEventHashMap::iterator it = map.begin(); it != map.end(); it++){
              if(it->second>1)printf("Run %6i Event %10i is duplicated (%i times)\n",it->first.first, it->first.second, it->second);
           }          
           printf("##########################################");
        }
};
 

#ifdef FWLITE


TH3F* loadDeDxTemplate(string path, bool splitByModuleType=false);
reco::DeDxData* computedEdx(const DeDxHitInfo* dedxHits, double scaleFactor=1.0, TH3* templateHisto=NULL, bool usePixel=false, bool useClusterCleaning=true, bool reverseProb=false, bool useTruncated=false);
bool clusterCleaning(const SiStripCluster*   cluster,  bool crosstalkInv=false );
void printStripCluster(FILE* pFile, const SiStripCluster*   cluster, const DetId& DetId);

TH3F* loadDeDxTemplate(string path, bool splitByModuleType){
   TFile* InputFile = new TFile(path.c_str());
   TH3F* DeDxMap_ = (TH3F*)GetObjectFromPath(InputFile, "Charge_Vs_Path");
   if(!DeDxMap_){printf("dEdx templates in file %s can't be open\n", path.c_str()); exit(0);}

   TH3F* Prob_ChargePath  = (TH3F*)(DeDxMap_->Clone("Prob_ChargePath")); 
   Prob_ChargePath->Reset();
   Prob_ChargePath->SetDirectory(0); 

   if(!splitByModuleType){
      Prob_ChargePath->RebinX(Prob_ChargePath->GetNbinsX());
   }

   for(int i=0;i<=Prob_ChargePath->GetXaxis()->GetNbins()+1;i++){
      for(int j=0;j<=Prob_ChargePath->GetYaxis()->GetNbins()+1;j++){
         double Ni = 0;
         for(int k=0;k<=Prob_ChargePath->GetZaxis()->GetNbins()+1;k++){ Ni+=DeDxMap_->GetBinContent(i,j,k);} 

         for(int k=0;k<=Prob_ChargePath->GetZaxis()->GetNbins()+1;k++){
            double tmp = 0;
            for(int l=0;l<=k;l++){ tmp+=DeDxMap_->GetBinContent(i,j,l);}

            if(Ni>0){
               Prob_ChargePath->SetBinContent (i, j, k, tmp/Ni);
            }else{
               Prob_ChargePath->SetBinContent (i, j, k, 0);
            }
         }
      }
   }
   InputFile->Close();
   return Prob_ChargePath;
}

#include "DataFormats/SiStripDetId/interface/SiStripDetId.h"
DeDxData* computedEdx(const DeDxHitInfo* dedxHits, double scaleFactor, TH3* templateHisto, bool usePixel, bool useClusterCleaning, bool reverseProb, bool useTruncated){
     if(!dedxHits) return NULL;
     if(templateHisto)usePixel=false; //never use pixel for discriminator

     std::vector<double> vect;
     for(unsigned int h=0;h<dedxHits->size();h++){
        DetId detid(dedxHits->detId(h));  
        if(!usePixel && detid.subdetId()<3)continue; // skip pixels
//        if(useClusterCleaning && !clusterCleaning(dedxHits->stripCluster(h)))continue;
         //printStripCluster(stdout, dedxHits->stripCluster(h), dedxHits->detId(h));


        //Remove hits close to the border  //FIXME to be activated in this code
        //double absDistEdgeXNorm = 1-fabs(hscpHitsInfo.localx[h])/(hscpHitsInfo.modwidth [h]/2.0);
        //double absDistEdgeYNorm = 1-fabs(hscpHitsInfo.localy[h])/(hscpHitsInfo.modlength[h]/2.0);
        //if(detid.subdetId()==1 && (absDistEdgeXNorm<0.05  || absDistEdgeYNorm<0.01)) continue;
        //if(detid.subdetId()==2 && (absDistEdgeXNorm<0.05  || absDistEdgeYNorm<0.01)) continue; 
        //if(detid.subdetId()==3 && (absDistEdgeXNorm<0.005 || absDistEdgeYNorm<0.04)) continue;  
        //if(detid.subdetId()==4 && (absDistEdgeXNorm<0.005 || absDistEdgeYNorm<0.02)) continue;  
        //if(detid.subdetId()==5 && (absDistEdgeXNorm<0.005 || absDistEdgeYNorm<0.02 || absDistEdgeYNorm>0.97)) continue;
        //if(detid.subdetId()==6 && (absDistEdgeXNorm<0.005 || absDistEdgeYNorm<0.03 || absDistEdgeYNorm>0.8)) continue;

        if(templateHisto){  //save discriminator probability
           double ChargeOverPathlength = scaleFactor*dedxHits->charge(h)/(dedxHits->pathlength(h)*10.0);
           SiStripDetId SSdetId(detid); //we sure it's strip since template force the use of usePixel=false
           int    BinX   = templateHisto->GetXaxis()->FindBin(SSdetId.moduleGeometry());
           int    BinY   = templateHisto->GetYaxis()->FindBin(dedxHits->pathlength(h)*10.0); //*10 because of cm-->mm
           int    BinZ   = templateHisto->GetZaxis()->FindBin(ChargeOverPathlength);
           double Prob   = templateHisto->GetBinContent(BinX,BinY,BinZ);
           //printf("%i %i %i  %f\n", BinX, BinY, BinZ, Prob);
           if(reverseProb)Prob = 1.0 - Prob;
           vect.push_back(Prob); //save probability
        }else{              
           double Norm = (detid.subdetId()<3)?3.61e-06:3.61e-06*265;
           double ChargeOverPathlength = scaleFactor*Norm*dedxHits->charge(h)/dedxHits->pathlength(h);
           vect.push_back(ChargeOverPathlength); //save charge

//           printf("%i - %f / %f = %f\n", h, scaleFactor*Norm*dedxHits->charge(h), dedxHits->pathlength(h), ChargeOverPathlength);
        }
     }

     double result;
     int size = vect.size();

     if(size>0){
        if(templateHisto){  //dEdx discriminator
           //Prod discriminator
           //result = 1;
           //for(int i=0;i<size;i++){
           //   if(vect[i]<=0.0001){result *= pow(0.0001 , 1.0/size);}
           //   else               {result *= pow(vect[i], 1.0/size);}
           //}

           //Ias discriminator
           result = 1.0/(12*size);
           std::sort(vect.begin(), vect.end(), std::less<double>() );
           for(int i=1;i<=size;i++){
              result += vect[i-1] * pow(vect[i-1] - ((2.0*i-1.0)/(2.0*size)),2);
           }
           result *= (3.0/size);
        }else{  //dEdx estimator
           if(useTruncated){
              //truncated40 estimator
              result=0;
              int nTrunc = size*0.40;
              for(int i = 0; i+nTrunc<size; i ++){
                 result+=vect[i];
              }
              result /= (size-nTrunc);
           }else{
              //harmonic2 estimator           
              result=0;
              double expo = -2;
              for(int i = 0; i< size; i ++){
                 result+=pow(vect[i],expo);
              }
              result = pow(result/size,1./expo);
           }
//           printf("Ih = %f\n------------------\n",result);
        }
     }else{
        result = -1;
     }
     return new DeDxData(result, -1, size);  //Nsaturated must replace the -1 here
}



void printStripCluster(FILE* pFile, const SiStripCluster*   cluster, const DetId& DetId)
{
        if(!cluster)return;
        const vector<unsigned char>&  ampls       = cluster->amplitudes();

        int Charge=0;
        for(unsigned int i=0;i<ampls.size();i++){Charge+=ampls[i];}
        char clusterCleaningOutput = clusterCleaning(cluster) ? 'V' : 'X';

        fprintf(pFile,"DetId = %7i --> %4i = %3i ",DetId.rawId(),Charge,ampls[0]);
        for(unsigned int i=1;i<ampls.size();i++){
           fprintf(pFile,"%3i ",ampls[i]);
        }
        fprintf(pFile,"   %c\n", clusterCleaningOutput);
}




std::vector<int> convert(const vector<unsigned char>& input)
{
  std::vector<int> output;
  for(unsigned int i=0;i<input.size();i++){
        output.push_back((int)input[i]);
  }
  return output;
}


std::vector<int> CrossTalkInv(const std::vector<int>&  Q, const float x1=0.10, const float x2=0.04, bool way=true,float threshold=20,float thresholdSat=25);
std::vector<int> CrossTalkInv(const std::vector<int>&  Q, const float x1, const float x2, bool way,float threshold,float thresholdSat) {
  const unsigned N=Q.size();
  std::vector<int> QII;
  std::vector<float> QI(N,0);
  Double_t a=1-2*x1-2*x2;
//  bool debugbool=false;
  TMatrix A(N,N);

//---  que pour 1 max bien net 
 if(Q.size()<2 || Q.size()>8){
	for (unsigned int i=0;i<Q.size();i++){
		QII.push_back((int) Q[i]);
  	}
	return QII;
  }
 if(way){ 
	  vector<int>::const_iterator mQ = max_element(Q.begin(), Q.end())	;
	  if(*mQ>253){
	 	 if(*mQ==255 && *(mQ-1)>253 && *(mQ+1)>253 ) return Q ;
	 	 if(*(mQ-1)>thresholdSat && *(mQ+1)>thresholdSat && *(mQ-1)<254 && *(mQ+1)<254 &&  abs(*(mQ-1) - *(mQ+1)) < 40 ){
		     QII.push_back((10*(*(mQ-1))+10*(*(mQ+1)))/2); return QII;}
	  }
  }
//---

  for(unsigned int i=0; i<N; i++) {
        A(i,i) =a;
        if(i<N-1){ A(i+1,i)=x1;A(i,i+1)=x1;}
        else continue; 
        if(i<N-2){ A(i+2,i)=x2;A(i,i+2)=x2;}
  }

  if(N==1) A(0,0)=1/a;
  else  A.InvertFast();

  for(unsigned int i=0; i<N; i++) {
        for(unsigned int j=0; j<N; j++) {
        QI[i]+=A(i,j)*(float)Q[j];
        }
  }

 for (unsigned int i=0;i<QI.size();i++){
	if(QI[i]<threshold) QI[i]=0; 
	QII.push_back((int) QI[i]);
  }

return QII;
}


bool clusterCleaning(const SiStripCluster*   cluster,  bool crosstalkInv)
{
   if(!cluster) return true;
   vector<int>  ampls = convert(cluster->amplitudes());
   if(crosstalkInv)ampls = CrossTalkInv(ampls,0.10,0.04, true);
      

  // ----------------  COMPTAGE DU NOMBRE DE MAXIMA   --------------------------
  //----------------------------------------------------------------------------
         Int_t NofMax=0; Int_t recur255=1; Int_t recur254=1;
         bool MaxOnStart=false;bool MaxInMiddle=false, MaxOnEnd =false;
         Int_t MaxPos=0;
        // D�but avec max
         if(ampls.size()!=1 && ((ampls[0]>ampls[1])
            || (ampls.size()>2 && ampls[0]==ampls[1] && ampls[1]>ampls[2] && ampls[0]!=254 && ampls[0]!=255) 
            || (ampls.size()==2 && ampls[0]==ampls[1] && ampls[0]!=254 && ampls[0]!=255)) ){
          NofMax=NofMax+1;  MaxOnStart=true;  }

        // Maximum entour�
         if(ampls.size()>2){
          for (unsigned int i =1; i < ampls.size()-1; i++) {
                if( (ampls[i]>ampls[i-1] && ampls[i]>ampls[i+1]) 
                    || (ampls.size()>3 && i>0 && i<ampls.size()-2 && ampls[i]==ampls[i+1] && ampls[i]>ampls[i-1] && ampls[i]>ampls[i+2] && ampls[i]!=254 && ampls[i]!=255) ){ 
                 NofMax=NofMax+1; MaxInMiddle=true;  MaxPos=i; 
                }
                if(ampls[i]==255 && ampls[i]==ampls[i-1]) {
                        recur255=recur255+1;
                        MaxPos=i-(recur255/2);
                        if(ampls[i]>ampls[i+1]){NofMax=NofMax+1;MaxInMiddle=true;}
                }
                if(ampls[i]==254 && ampls[i]==ampls[i-1]) {
                        recur254=recur254+1;
                        MaxPos=i-(recur254/2);
                        if(ampls[i]>ampls[i+1]){NofMax=NofMax+1;MaxInMiddle=true;}
                }
            }
         }
        // Fin avec un max
         if(ampls.size()>1){
          if(ampls[ampls.size()-1]>ampls[ampls.size()-2]
             || (ampls.size()>2 && ampls[ampls.size()-1]==ampls[ampls.size()-2] && ampls[ampls.size()-2]>ampls[ampls.size()-3] ) 
             ||  ampls[ampls.size()-1]==255){
           NofMax=NofMax+1;  MaxOnEnd=true;   }
         }
        // Si une seule strip touch�e
        if(ampls.size()==1){    NofMax=1;}



  // ---  SELECTION EN FONCTION DE LA FORME POUR LES MAXIMA UNIQUES ---------
  //------------------------------------------------------------------------
//  
//               ____
//              |    |____
//          ____|    |    |
//         |    |    |    |____
//     ____|    |    |    |    |
//    |    |    |    |    |    |____
//  __|____|____|____|____|____|____|__
//    C_Mnn C_Mn C_M  C_D  C_Dn C_Dnn
//  
//   bool shapetest=true;
   bool shapecdtn=false;

      if(crosstalkInv){
        if(NofMax==1){shapecdtn=true;}
        return shapecdtn;
      }

//      Float_t C_M;    Float_t C_D;    Float_t C_Mn;   Float_t C_Dn;   Float_t C_Mnn;  Float_t C_Dnn;
        Float_t C_M=0.0;        Float_t C_D=0.0;        Float_t C_Mn=10000;     Float_t C_Dn=10000;     Float_t C_Mnn=10000;    Float_t C_Dnn=10000;
        Int_t CDPos;
        Float_t coeff1=1.7;     Float_t coeff2=2.0;
        Float_t coeffn=0.10;    Float_t coeffnn=0.02; Float_t noise=4.0;

        if(NofMax==1){

                if(MaxOnStart==true){
                        C_M=(Float_t)ampls[0]; C_D=(Float_t)ampls[1];
                                if(ampls.size()<3) shapecdtn=true ;
                                else if(ampls.size()==3){C_Dn=(Float_t)ampls[2] ; if(C_Dn<=coeff1*coeffn*C_D+coeff2*coeffnn*C_M+2*noise || C_D==255) shapecdtn=true;}
                                else if(ampls.size()>3){ C_Dn=(Float_t)ampls[2];  C_Dnn=(Float_t)ampls[3] ;
                                                        if((C_Dn<=coeff1*coeffn*C_D+coeff2*coeffnn*C_M+2*noise || C_D==255)
                                                           && C_Dnn<=coeff1*coeffn*C_Dn+coeff2*coeffnn*C_D+2*noise){
                                                         shapecdtn=true;}
                                }
                }

                if(MaxOnEnd==true){
                        C_M=(Float_t)ampls[ampls.size()-1]; C_D=(Float_t)ampls[ampls.size()-2];
                                if(ampls.size()<3) shapecdtn=true ;
                                else if(ampls.size()==3){C_Dn=(Float_t)ampls[0] ; if(C_Dn<=coeff1*coeffn*C_D+coeff2*coeffnn*C_M+2*noise || C_D==255) shapecdtn=true;}
                                else if(ampls.size()>3){C_Dn=(Float_t)ampls[ampls.size()-3] ; C_Dnn=(Float_t)ampls[ampls.size()-4] ; 
                                                        if((C_Dn<=coeff1*coeffn*C_D+coeff2*coeffnn*C_M+2*noise || C_D==255)
                                                           && C_Dnn<=coeff1*coeffn*C_Dn+coeff2*coeffnn*C_D+2*noise){ 
                                                         shapecdtn=true;}
                                }
                }

                if(MaxInMiddle==true){
                        C_M=(Float_t)ampls[MaxPos];
                        int LeftOfMaxPos=MaxPos-1;if(LeftOfMaxPos<=0)LeftOfMaxPos=0;
                        int RightOfMaxPos=MaxPos+1;if(RightOfMaxPos>=(int)ampls.size())RightOfMaxPos=ampls.size()-1;
                        //int after = RightOfMaxPos; int before = LeftOfMaxPos; if (after>=(int)ampls.size() ||  before<0)  std::cout<<"invalid read MaxPos:"<<MaxPos <<"size:"<<ampls.size() <<std::endl; 
                        if(ampls[LeftOfMaxPos]<ampls[RightOfMaxPos]){ C_D=(Float_t)ampls[RightOfMaxPos]; C_Mn=(Float_t)ampls[LeftOfMaxPos];CDPos=RightOfMaxPos;} else{ C_D=(Float_t)ampls[LeftOfMaxPos]; C_Mn=(Float_t)ampls[RightOfMaxPos];CDPos=LeftOfMaxPos;}
                        if(C_Mn<coeff1*coeffn*C_M+coeff2*coeffnn*C_D+2*noise || C_M==255){ 
                                if(ampls.size()==3) shapecdtn=true ;
                                else if(ampls.size()>3){
                                        if(CDPos>MaxPos){
                                                if(ampls.size()-CDPos-1==0){
                                                        C_Dn=0; C_Dnn=0;
                                                }
                                                if(ampls.size()-CDPos-1==1){
                                                        C_Dn=(Float_t)ampls[CDPos+1];
                                                        C_Dnn=0;
                                                }
                                                if(ampls.size()-CDPos-1>1){
                                                        C_Dn=(Float_t)ampls[CDPos+1];
                                                        C_Dnn=(Float_t)ampls[CDPos+2];
                                                }
                                                if(MaxPos>=2){
                                                        C_Mnn=(Float_t)ampls[MaxPos-2];
                                                }
                                                else if(MaxPos<2) C_Mnn=0;
                                        }
                                        if(CDPos<MaxPos){
                                                if(CDPos==0){
                                                        C_Dn=0; C_Dnn=0;
                                                }
                                                if(CDPos==1){
                                                        C_Dn=(Float_t)ampls[0];
                                                        C_Dnn=0;
                                                }
                                                if(CDPos>1){
                                                        C_Dn=(Float_t)ampls[CDPos-1];
                                                        C_Dnn=(Float_t)ampls[CDPos-2];
                                                }
                                                if(ampls.size()-LeftOfMaxPos>1 && MaxPos+2<(int)(ampls.size())-1){
                                                        C_Mnn=(Float_t)ampls[MaxPos+2];
                                                }else C_Mnn=0;                                                  
                                        }
                                        if((C_Dn<=coeff1*coeffn*C_D+coeff2*coeffnn*C_M+2*noise || C_D==255)
                                           && C_Mnn<=coeff1*coeffn*C_Mn+coeff2*coeffnn*C_M+2*noise
                                           && C_Dnn<=coeff1*coeffn*C_Dn+coeff2*coeffnn*C_D+2*noise) {
                                                shapecdtn=true;
                                        }

                                }
                        }                       
                }
        }
        if(ampls.size()==1){shapecdtn=true;}

   return shapecdtn;
}
#endif

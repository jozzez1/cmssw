#include <exception>
#include <vector>
#include <algorithm>

#include "TROOT.h"
#include "TFile.h"
#include "TDirectory.h"
#include "TChain.h"
#include "TObject.h"
#include "TCanvas.h"
#include "TMath.h"
#include "TLegend.h"
#include "TGraph.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TTree.h"
#include "TF1.h"
#include "TCutG.h"
#include "TGraphAsymmErrors.h"
#include "TProfile.h"
#include "TPaveText.h"


#if !defined(__CINT__) && !defined(__MAKECINT__)
#include "FWCore/FWLite/interface/FWLiteEnabler.h"
#include "DataFormats/FWLite/interface/Handle.h"
#include "DataFormats/FWLite/interface/Event.h"
#include "DataFormats/FWLite/interface/ChainEvent.h"

#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/TrackReco/interface/DeDxHitInfo.h"
#endif

using namespace fwlite;
using namespace reco;
using namespace std;
using namespace edm;

/*
 *          ^
 * dE/dx   /|\
 *          |
 *          +-----------+--------------
 *          |           |
 *          |     A     |      D
 *          |           |
 *  I_Cut   +-----------+--------------
 *          |           |
 *          |     B     |      C
 *          |           |
 *          +-----------+----------------->
 *                      p_Cut           p
 *
 *  default cuts
 *  I_Cut = 0.3 MeV/cm
 *  p_Cut = 65 GeV
 */

struct studyObj {
    TH1D* NEvents;
    TH1D* NumTracks;
    TH1D* A;
    TH1D* B;
    TH1D* C;
    TH1D* D;

    TH1D* Pt;
    TH1D* P;
    TH1D* clusterdEdx;
    TH1D* pixelClusterdEdx;
    TH1D* stripClusterdEdx;
    TH1D* trackdEdx;
    TH1D* pixeldEdx;
    TH1D* stripdEdx;
    TH2D* dEdxVsPt;
    TH2D* dEdxVsP;

    TH1I* track_hits;
    TH1I* strip_hits;
    TH1I* pixel_hits;

    studyObj (){
        NEvents          = new TH1D ("NEvents", "NEvetns", 1, 0, 1);
        NumTracks        = new TH1D ("NumTracks", "NumTracks", 1, 0, 1);
        A                = new TH1D ("A", "A", 1, 0, 1);
        B                = new TH1D ("B", "B", 1, 0, 1);
        C                = new TH1D ("C", "C", 1, 0, 1);
        D                = new TH1D ("D", "D", 1, 0, 1);
                         
        Pt               = new TH1D ("Pt", "Pt", 100, 0, 100);
        P                = new TH1D ("P", "P", 100, 0, 100);
        clusterdEdx      = new TH1D ("cluster_dEdx", "cluster_dEdx", 100, 0, 16);
        pixelClusterdEdx = new TH1D ("pixelCluster_dEdx", "pixelCluster_dEdx", 100, 0, 16);
        stripClusterdEdx = new TH1D ("stripCluster_dEdx", "stripCluster_dEdx", 100, 0, 16);
        trackdEdx        = new TH1D ("track_dEdx", "track_dEdx", 100, 0, 16);
        pixeldEdx        = new TH1D ("pixel_dEdx", "pixel_dEdx", 100, 0, 16);
        stripdEdx        = new TH1D ("strip_dEdx", "strip_dEdx", 100, 0, 16);
        dEdxVsPt         = new TH2D ("dEdxVsPt", "dEdxVsPt", 100, 0, 100, 100, 0, 16);
        dEdxVsP          = new TH2D ("dEdxVsP", "dEdxVsP", 100, 0, 100, 100, 0, 16);

        track_hits       = new TH1I ("track_hits", "track_hits", 30, 0, 30);
        pixel_hits       = new TH1I ("pixel_hits", "pixel_hits", 30, 0, 30);
        strip_hits       = new TH1I ("strip_hits", "strip_hits", 30, 0, 30);

    }
};

void RelValStudy (string DIRNAME="COMPILE", string INPUT="dEdx.root", string OUTPUT="out.root")
{
    std::cout << "TEST A" << std::endl;
    if(DIRNAME=="COMPILE") return;
    std::cout << "TEST B" << std::endl;
 
    /*
    setTDRStyle();
    gStyle->SetPadTopMargin   (0.06);
    gStyle->SetPadBottomMargin(0.15);
    gStyle->SetPadRightMargin (0.03);
    gStyle->SetPadLeftMargin  (0.07);
    gStyle->SetTitleSize(0.04, "XYZ");
    gStyle->SetTitleXOffset(1.1);
    gStyle->SetTitleYOffset(1.35);
    gStyle->SetPalette(1);
    gStyle->SetNdivisions(505,"X");
    TH1::AddDirectory(kTRUE);
    */
     
    // if you find a comma work it as a list of files, otherwise open just one file, you doofus!
    vector<string> FilesToProcess;
    size_t comma = INPUT.find(",");
    if (comma==string::npos) FilesToProcess.push_back(INPUT);
    else {
        size_t begin = 0,
               end   = comma;
 
        do {
            FilesToProcess.push_back (INPUT.substr(begin, end-begin));
            begin = end+1;
            end   = INPUT.find(",", begin);
        } while (end != string::npos);
        FilesToProcess.push_back (INPUT.substr(begin, INPUT.size()-begin));
    }

    for (auto filename : FilesToProcess)
        std::cout << filename << "\n" << std::endl;

    TFile* out = new TFile(OUTPUT.c_str(), "RECREATE");
    studyObj results;
    for (auto filename : FilesToProcess){ // file loop
        
        TFile* file = TFile::Open(filename.c_str());
        cout << "Processing \%\%\%" << filename << "\%\%\% ..." << endl;
        fwlite::Event ev (file);
        
        for (ev.toBegin(); !ev.atEnd(); ++ev){ // event loop

            fwlite::Handle<DeDxHitInfoAss> dedxCollH;
            dedxCollH.getByLabel(ev, "dedxHitInfo");
            if(!dedxCollH.isValid()){printf("Unable to find the dE/dx collection\n");continue;}

            fwlite::Handle< std::vector<reco::Track> > trackCollHandle;
            trackCollHandle.getByLabel(ev,"RefitterForDeDx");
            if(!trackCollHandle.isValid()){
                trackCollHandle.getByLabel(ev,"generalTracks");
                if (!trackCollHandle.isValid()){
                    printf("Invalid trackCollHandle\n");
                    continue;
                }
            }

            double Pt_Cut   = 65,
                   dEdx_Cut = 3.5;
            
            results.NEvents->Fill(0.5);

            // compute dEdx
            for (size_t t = 0; t < trackCollHandle->size(); t++){ // track loop
                reco::TrackRef track = reco::TrackRef (trackCollHandle.product(), t);
                if (track.isNull()) continue;
                
                const DeDxHitInfo* dedxHits = NULL;
                DeDxHitInfoRef dedxHitsRef = dedxCollH->get(track.key());
                if(!dedxHitsRef.isNull())dedxHits = &(*dedxHitsRef);
                if(!dedxHits)continue;

                results.NumTracks->Fill (0.5);
                results.Pt->Fill (track->pt());
                
                double dEdx       = 0,
                       pixel_dEdx = 0,
                       strip_dEdx = 0;
                unsigned int pixel_hits = 0,
                             strip_hits = 0;
                for (size_t h = 0; h < dedxHits->size(); h++){ // cluster loop
                    DetId detid (dedxHits->detId(h));
                    double cluster_dEdx = ((detid.subdetId()<3)?3.61e-06:3.61e-06*265) *
                                          dedxHits->charge(h) / dedxHits->pathlength(h);

                    results.clusterdEdx->Fill(cluster_dEdx);

                    dEdx += 1.0/(cluster_dEdx * cluster_dEdx);

                    if (detid.subdetId()<3){
                        results.pixelClusterdEdx->Fill (cluster_dEdx);
                        pixel_dEdx += dEdx;
                        pixel_hits ++;
                    } else {
                        results.stripClusterdEdx->Fill (cluster_dEdx);
                        strip_dEdx += dEdx;
                        strip_hits ++;
                    }
                } // end of cluster loop

                dEdx       = 1.0/sqrt(dEdx/dedxHits->size()); // harmonic-2 estimator for strips and pixels
                pixel_dEdx = 1.0/sqrt(pixel_dEdx/pixel_hits); // harmonic-2 estimator for pixels
                strip_dEdx = 1.0/sqrt(strip_dEdx/strip_hits); // harmonic-2 estimator for strips

                results.trackdEdx->Fill(dEdx);
                results.pixeldEdx->Fill(pixel_dEdx);
                results.stripdEdx->Fill(strip_dEdx);

                results.track_hits->Fill (dedxHits->size());
                results.pixel_hits->Fill (pixel_hits);
                results.strip_hits->Fill (strip_hits);
                
                results.dEdxVsPt->Fill (track->pt(), dEdx);
                results.dEdxVsP ->Fill (track->p (), dEdx);
                if      (dEdx > dEdx_Cut && track->pt() > Pt_Cut) results.D->Fill (0.5);
                else if (dEdx > dEdx_Cut && track->pt() < Pt_Cut) results.A->Fill (0.5);
                else if (dEdx < dEdx_Cut && track->pt() > Pt_Cut) results.C->Fill (0.5);
                else if (dEdx < dEdx_Cut && track->pt() < Pt_Cut) results.B->Fill (0.5);
            } // end of track loop
        } // end of event loop
        file->Close();
    } // end of file loop
    out->Write();
    out->Close();
}

// summary:
// from ABCD we add two new regions: alpha and omega for pT < 10 GeV
// alpha passes the dEdx cut and omega fails it
// alpha has a population between 500k - 1M wrt. the 2nd dataset
// C is the most populous region in that passes the cut with ~ 1000 tracks
// propose 1/1000 prescale

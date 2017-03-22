#include <vector>
#include <string>
#include <cstdio>
#include <iostream>
#include "TTree.h"
#include "TBranch.h"
#include "TFile.h"
#include "TProfile.h"
#include "TCanvas.h"
#include "TH1D.h"

typedef struct StripPayload {
    unsigned int detId;
    double   gains;

    StripPayload (unsigned int detId_, double gains_) : detId(detId_), gains(gains_) {};
} StripPayload;

char* getProgressBar (double percentage, unsigned int barLength=100);
void printProgressBar (const char* bar, const char* prefix=NULL);

void checkMissingDetId (TTree* t1, TTree* t2, std::vector<unsigned int>& missing, std::vector<unsigned int>& matched, TH1D* h=NULL)
{
    unsigned int detId1, detId2;
    double gains1, gains2;
    
    t1->SetBranchAddress ("DetId", &detId1);
    t2->SetBranchAddress ("DetId", &detId2);

    t1->SetBranchAddress ("Gains", &gains1);
    t2->SetBranchAddress ("Gains", &gains2);

    std::vector <StripPayload> payloads1;
    std::vector <StripPayload> payloads2;
    for (size_t i=0; i < (size_t) t1->GetEntries(); i++){
        t1->GetEntry(i);
        StripPayload p (detId1, gains1);
        payloads1.push_back(p);
    }

    for (size_t i=0; i < (size_t) t2->GetEntries(); i++){
        t2->GetEntry(i);
        StripPayload p (detId2, gains2);
        payloads2.push_back(p);
    }

    std::cout << payloads1.size() << "\t" << payloads2.size() << std::endl;
/*    for (size_t i=0; i<payloads2.size(); i++){
        std::cout << payloads2[i].detId;
        unsigned int lock_detId = payloads2[i].detId;
        do{
            std::cout << "   " << payloads2[i].gains;
        } while (payloads2[++i].detId == lock_detId);
        std::cout << std::endl;
    }
*/
    double percentage = 0.0;
    char* bar = NULL;
    size_t j = 0, jOld = 0;
    for (size_t i=0; i < payloads1.size(); i++){
        bool match = false;

        percentage = i*1.0/payloads1.size();
        bar = getProgressBar (percentage, 60);
        printProgressBar (bar);
        for (j=jOld; j < payloads2.size(); j++){
            if (payloads1[i].detId == payloads2[j].detId){
                match = true;
                jOld = j;
                if (matched.size() == 0) matched.push_back (payloads1[i].detId);
                else if (matched[matched.size()-1] != payloads1[i].detId) matched.push_back (payloads1[i].detId);
                break;
            }
        }

        unsigned int OldI = i;
        if (h && match && ((i > 0 && payloads1[i].detId != payloads1[i-1].detId) || (i == 0))){
            do {
                h->Fill (payloads1[OldI++].gains*1.0/payloads2[j++].gains);
            } while (payloads1[i].detId == payloads2[j].detId);
        }

        else if (!match){
            if (missing.size() == 0) missing.push_back (payloads1[i].detId);
            else if (missing[missing.size()-1] != payloads1[i].detId) missing.push_back (payloads1[i].detId);
        }
    }
}

void analyzer (){
    TFile* fin1 = new TFile ("OldTree.root", "READ");
    TFile* fin2 = new TFile ("NewTree.root", "READ");

    TTree* t1   = (TTree*) fin1->Get("OldTree");
    TTree* t2   = (TTree*) fin2->Get("NewTree");

    std::vector<unsigned int> missing;
    std::vector<unsigned int> matched;

    TH1D* h = new TH1D ("Ratio", "Ratio", 100, 1, 1.2);

    checkMissingDetId (t1, t2, missing, matched, h);

    FILE* fmissing_out = fopen ("missing.log", "w");
    FILE* fmatched_out = fopen ("matched.log", "w");

    TCanvas* c = new TCanvas ("Ratio", "Ratio", 600, 600);
    c->SetLogy();
    h->Scale(1.0/h->Integral());
    h->GetXaxis()->SetTitle ("G1_{old}/G1_{new}");
    h->GetYaxis()->SetTitle ("a.u.");
    h->SetStats(kFALSE);
    h->SetLineColor(kBlue);
    h->Draw("Hist");
    c->SaveAs("Ratio.png");
    c->SaveAs("Ratio.pdf");


    for (size_t i=0; i<missing.size(); i++)
        fprintf (fmissing_out, "%u\n", missing[i]);
    for (size_t j=0; j<matched.size(); j++)
        fprintf (fmatched_out, "%u\n", matched[j]);
}

char* getProgressBar (double percentage, unsigned int barLength)
{
   char* bar = new char [barLength+8];
   sprintf (bar, "[");
   unsigned int i = 0;
   for (; i+1 < (unsigned int) (percentage*barLength); i++)
      sprintf (bar, "%s=", bar);
   sprintf (bar, "%s%c", bar, percentage<0.99?'>':'=');
   for (; i < barLength-1; i++)
      sprintf (bar, "%s ", bar);
   sprintf (bar, "%s] %3u %% ", bar, (unsigned int) (100*percentage));
   return bar;
}

void printProgressBar (const char* bar, const char* prefix)
{
   if (prefix)
      fprintf (stdout, "\r%s%s", prefix, bar);
   else
      fprintf (stdout, "\r%s", bar);
   fflush (stdout);
}


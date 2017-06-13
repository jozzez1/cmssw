#include <iostream>
#include <string>
#include "TTree.h"
#include "TFile.h"
#include "TObject.h"
#include "TList.h"
#include "../../AnalysisCode/Analysis_CommonFunction.h"

void
ReorganizeGains (){
    TFile* fin = new TFile ("../../../data/Data13TeVGains_v2.root", "READ");
    TList* ObjList = fin->GetListOfKeys();
    for (int i=0; i<ObjList->GetSize(); i++)
    {
        TTree* tmp = (TTree*) GetObjectFromPath (fin, ObjList->At(i)->GetName(), false);
        if (tmp->InheritsFrom("TTree"))
        {
            std::string name  = std::string(tmp->GetName  ());
            std::string title = std::string(tmp->GetTitle ());

            if (name.find ("276870_to_999999") != std::string::npos
            && title.find ("276870_to_999999") != std::string::npos){
                name  = "Gains_276870_to_278405";
            }

            TTree* renamed = new TTree (name.c_str(), name.c_str());
            renamed->AddFriend(tmp);
            renamed->SaveAs (("Gains/" + name + ".root").c_str());
            delete renamed;
            delete tmp;
        }
    }
}

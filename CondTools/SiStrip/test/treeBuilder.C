#include <vector>
#include <string>
#include <cstdio>
#include <iostream>
#include "TTree.h"
#include "TBranch.h"
#include "TFile.h"

class CStringTokenizer {
    public:
        CStringTokenizer (const char* tokens_){
            tokens = string(tokens_);
        }

        std::vector<std::string> tokenize (const char* cstring);
    private:
        std::string tokens;
};

std::vector<std::string>
CStringTokenizer::tokenize (const char* cstring)
{
    std::vector<std::string> result;
    std::string part = "";
    for (unsigned int i=0; cstring[i] != '\n'; i++){
        bool match = false;
        for (auto token : tokens){
            if (cstring[i] == token) match = true;
        }

        if (!match)
            part += cstring[i];
        if (match){
            result.push_back (part);
            part = "";
        }
    }

    return result;
}

void treeBuilder (const char* inLogName, const char* treeName, const char* fileName)
{
    std::cout << inLogName << "\t"
              << treeName  << "\t"
              << fileName  << std::endl;
    CStringTokenizer tokenizer (",; \t");

    uint32_t index = 0,
             detid = 0,
             ApvId = 0;
    double   gain  = 1.0;;

    FILE* inLogfile = fopen (inLogName, "r");

    TFile* outfile = new TFile (fileName, "RECREATE");

    TTree* t = new TTree (treeName, treeName);
    TBranch* t_index = t->Branch ("Index", &index, "Index/i");
    TBranch* t_detid = t->Branch ("DetId", &detid, "DetId/i");
    TBranch* t_ApvId = t->Branch ("ApvId", &ApvId, "ApvId/i");
    TBranch* t_gain  = t->Branch ("Gains", &gain , "Gains/D");

    char tmp [99];
    std::vector<std::string> numberString;
    do{
        fgets (tmp, 99, inLogfile);
        numberString = tokenizer.tokenize(tmp);
        
        detid = (uint32_t) atol (numberString[0].c_str());
        std::cout << detid;
        for (unsigned int i=1; i<numberString.size(); i++){
            gain = (double) atof (numberString[i].c_str());
            std::cout << "\t" << numberString[i];

            t->Fill();
            ++index;
            ++ApvId;
        }
        std::cout << std::endl;
    } while (!feof(inLogfile));

    outfile->Write();
    outfile->Close();
}


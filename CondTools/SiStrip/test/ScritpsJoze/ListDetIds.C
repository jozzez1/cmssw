#include <iostream>
#include "usefulBits.h"

void ListDetIds (){
    FILE* txtFile = fopen ("DetIdList.log", "r");

    CStringTokenizer tokenizer ("/:,; \t");
    HexToDecimal hexConverter (8);
    std::vector<std::string> stringsInLine;
    std::vector<uint32_t> detids;
    std::vector<std::string> stuffs;
    std::vector<uint32_t> digits;
    do {
        char tmp [100];
        fgets (tmp, 100, txtFile);
        stringsInLine = tokenizer.tokenize(tmp);
        if (stringsInLine[stringsInLine.size()-1].find("0x")!=std::string::npos){
            std::string stuff = stringsInLine[stringsInLine.size()-1];
            stuffs.push_back(stuff);
            detids.push_back(hexConverter.hex2dec(stuff.c_str(), stuff.size()));
        }
        else if (stringsInLine[stringsInLine.size()-1].find("0x")==std::string::npos){
            std::string stuff = stringsInLine[stringsInLine.size()-2];
            stuffs.push_back(stuff);
            detids.push_back(hexConverter.hex2dec(stuff.c_str(), stuff.size()));
        }
    } while (!feof(txtFile));

    for (size_t i=0; i<stuffs.size(); i++){
        int subdetector = (int)(detids[i]>>25&0x7);
        if (subdetector > 0 && subdetector < 7)
            std::cout << "Tracker DetId: " << detids[i] << std::endl;
    }
}


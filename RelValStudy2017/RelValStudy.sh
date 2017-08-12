#!/bin/bash

echo "RelValStudy.C+(\"`pwd`\", \"$1\", \"$2\");"
echo
echo
echo

root -l << EOF
TString makeshared(gSystem->GetMakeSharedLib());
makeshared.ReplaceAll("-W ", "-Wno-deprecated-declarations -Wno-deprecated -Wno-unused-local-typedefs -Wno-attributes ");
makeshared.ReplaceAll("-Woverloaded-virtual ", " ");
makeshared.ReplaceAll("-Wshadow ", " -std=c++0x -D__USE_XOPEN2K8 ");
cout << "Compilling with the following arguments: " << makeshared << endl;
gSystem->SetMakeSharedLib(makeshared);
gSystem->Load("libFWCoreFWLite");
FWLiteEnabler::enable();
gSystem->Load("libDataFormatsFWLite.so");
gSystem->Load("libAnalysisDataFormatsSUSYBSMObjects.so");
gSystem->Load("libDataFormatsVertexReco.so");
gSystem->Load("libDataFormatsCommon.so");
gSystem->Load("libDataFormatsHepMCCandidate.so");
gSystem->Load("libPhysicsToolsUtilities.so");
gInterpreter->SetClassAutoparsing(false);
std::cout << "\nBEGIN" << std::endl;
.x RelValStudy.C+ ("`pwd`", "$1", "$2");
std::cout << "\nEXIT" << std::endl;
EOF



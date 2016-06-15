g++ -o findSignal    `root-config --cflags --glibs` AnalyzeADC_Signal.cxx    -lboost_system -lboost_filesystem -lboost_regex
g++ -o findTestPulse `root-config --cflags --glibs` AnalyzeADC_TestPulse.cxx -lboost_system -lboost_filesystem -lboost_regex

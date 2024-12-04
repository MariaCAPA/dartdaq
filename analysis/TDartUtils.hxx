2ifndef __DART_UTILS__H__
#define __DART_UTILS__H__

#include <string>
class TChain;

TChain * readDartRun(int run, std::string baseName="output");

#endif


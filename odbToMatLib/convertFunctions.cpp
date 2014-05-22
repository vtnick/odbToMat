#include <stdlib.h>
#include <algorithm>
#include <stdio.h>
#if (defined(HP) && (! defined(HKS_HPUXI)))
#include <iostream.h>
#include <iomanip.h>
#else
#include <iostream>
#include <iomanip>
using namespace std;
#endif

#include <odb_API.h>
#include <sys/stat.h>
#include"convertFunctions.h"

string convertName(const char *cname)
{
  string s;
  s = cname;
  std::replace( s.begin(), s.end(), '-', '_');
  std::replace( s.begin(), s.end(), ' ', '_');
  return s;
}

// const char** convertOdbSeqString(odb_SequenceString &inSeq)
// {
//     int totalNames = inSeq.size();
//     char *fieldNames[totalNames];
//     for (int name=0; name<totalNames; name++) {
//         const odb_String& iName = inSeq.constGet(name);
//         string s = convertName(iName.CStr());
//         char *cnametemp = new char[s.size() + 1];
//         std::strcpy ( cnametemp, s.c_str() );
//         fieldNames[name] = cnametemp;
//     }
//     const char **fields = (const char **) fieldNames;
//     return fields;
// }

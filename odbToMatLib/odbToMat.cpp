/***************************************************************
odbToMat.cpp
Code to extract odb information to MATLAB mat files
Usage: abaqus odbMaxMises -odb odbName
Requirements:
1. -odb      : Full path of the output database. (./work/odbName.odb)
2. -matPath  : Full path to save MAT file. (./work/odbName.mat)
3. -help     : Print usage
****************************************************************/
//
// System includes
//
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

//
// Abaqus Includes
//
#include <odb_API.h>
#include <sys/stat.h>

//
// MATLAB Includes
//
#include <string.h> /* For strcmp() */
#include <vector> /* For STL */
#include "mat.h"

// odbToMat Includes
#include "geometryFunctions.h"
#include "resultFunctions.h"

#ifdef WIN32
	string fileSep = "\\";
#else
	string fileSep = "/";
#endif
/*
***************
utility functions
***************
*/
void rightTrim(odb_String  &string,const char* char_set);
void printExecutionSummary();
bool fileExists(const odb_String  &string);
string getFileName(const char *odbstringName, const string& fileSep);

/***************************************************************/


int ABQmain(int argc, char **argv)
{
  odb_String odbPath;
  bool ifOdbName = false;
  bool ifMATPath = false;
  bool ifMATName = false;
  char msg[256];
  char *abaCmd = argv[0];
  odb_String matPath;
  for (int arg = 0; arg<argc; arg++)
    {
      if (strncmp(argv[arg],"-o**",2) == 0)
	{
	  arg++;
	  odbPath = argv[arg];
	  rightTrim(odbPath,".odb");
	  if (!fileExists(odbPath))
	    {
	      cerr << "**ERROR** output database  " << odbPath.CStr()
		   << " does not exist\n" << endl;
	      exit(1);
	    }
	  ifOdbName = true;
	}
      else if (strncmp(argv[arg],"-matP**",5)== 0)
	{
	  arg++;
	  matPath = argv[arg];
	  rightTrim(matPath, ".mat");
	  ifMATPath = true;
	}
      else if (strncmp(argv[arg],"-h**",2)== 0)
	{
	  printExecutionSummary();
	  exit(0);
	}
    }
	if (!ifOdbName)
    {
      cerr << "**ERROR** output database name is not provided\n";
      printExecutionSummary();
      exit(1);
    }
  const char *odbName = argv[2];

  // Open the output database
  odb_Odb& myOdb = openOdb(odbPath);
  odb_Assembly& myAssembly = myOdb.rootAssembly();

  // MAT File Name
  MATFile *pmat;
  if (!ifMATPath)
  {
    const char *odbName = myOdb.name().CStr();
  string fileName = getFileName(odbName, fileSep);
    pmat = matOpen(fileName.c_str(), "w");
  }
  else
  {
    pmat = matOpen(matPath.CStr(), "w");
  }

  // Extract ODB Information
  mxArray *odbGeometry, *odbResults;

  odbGeometry = extractGeometry(myAssembly);
  cout << "Geometry Extracted" << endl;
  int status;
  status = matPutVariable(pmat, "geometry", odbGeometry);
  cout << "Geometry Saved " << endl;

  odbResults = extractResults(myOdb);
  status = matPutVariable(pmat, "results", odbResults);
  matClose(pmat);

  // close the output database before exiting the program
  myOdb.close();
  return(0);
}

void printExecutionSummary()
{
  cout << "odbToMat.cpp \n"
	   << "Code to extract odb information to MATLAB mat files\n"
	   << "Usage: abaqus odbMaxMises -odb odbName\n"
	   << "Requirements:\n"
	   << "1. -odb      : Full path of the output database. (./work/odbName.odb)\n"
	   << "2. -matPath  : Full path to save MAT file. (./work/odbName.mat)\n"
	   << "3. -help     : Print usage";
}

int ABQmain(int argc, char** argv);
int main(int argc, char** argv)
{
    odb_initializeAPI();

    int status = 0;
    try {
        status = ABQmain(argc, argv);
    }
    catch (const nex_Exception& nex) {
    status = 1;
    fprintf(stderr, "%s\n", nex.UserReport().CStr());
    fprintf(stderr, "ODB Application exited with error(s)\n");
    }
    catch (...) {
    status = 1;
    fprintf(stderr, "ODB Application exited with error(s)\n");
    }
    odb_finalizeAPI();
    return (status);
}

void rightTrim(odb_String  &string,const char* char_set)
{
  int length = string.Length();
  if (string.Find(char_set)==length)
    string.append(odb_String(char_set));
}

bool fileExists(const odb_String  &string)
{
  bool exists = false;
  struct  stat  buf;
  if (stat(string.CStr(),&buf)==0)
    exists = true;
  return exists;
}

string getFileName(const char *odbstringName, const string& fileSep)
{
	string strPath = odbstringName;
	string fileExt = ".mat";
	size_t iLastSeparator = 0;
	iLastSeparator = strPath.find_last_of(fileSep) + 1;
	size_t iLastExt = strPath.find_last_of(".") - iLastSeparator;
	string fileName = strPath.substr(iLastSeparator, iLastExt);
	fileName.append(fileExt);
	return fileName;
}

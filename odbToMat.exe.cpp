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
#include <sstream>
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
string convertName(const char *cname);
/***************************************************************/
/*
***************
result function info
***************
*/
string resultsString = "results__step__";

// Struct Field Names
const char *fields[] = {"data", "position", "labels", "instance"};
int nfields = 4;
/***************************************************************/
/*
***************
geometry function info
***************
*/
string geoChar = "geometry__";
const char *nodefieldnames[] = {"label", "coords"};
const char *elementfieldnames[] = {"label", "connectivity", "eType", "instanceNames"};
const char *rootAssemblyName = "rootAssembly";
const char *rootnodeSetFieldNames[] = {"labels", "instanceNames"};
const char *nodeSetFieldNames[] = {"labels"};
const char *rootelementSetFieldNames[] = {"labels", "instanceNames"};
const char *elementSetFieldNames[] = {"labels"};
/***************************************************************/
/*
***************
geometry function declarations
***************
*/
void extractGeometry(MATFile *pmat, odb_Odb& myOdb);
void extractInstanceNodes(MATFile *pmat, odb_SequenceNode nodeList, odb_String iname);
void extractInstanceElements(MATFile *pmat, odb_SequenceElement elementList, odb_String iname);
void extractRootNodeSets(MATFile *pmat, odb_SetRepository nsets, const char *rname);
void extractPartNodeSets(MATFile *pmat, odb_SetRepository nsets, odb_String iname);
void extractInstanceNodeSets(MATFile *pmat, odb_Assembly& cassemb, odb_String iname);
void extractInstanceElementSets(MATFile *pmat, odb_Assembly& cassemb, odb_String iname);
void extractRootElementSets(MATFile *pmat, odb_SetRepository esets, const char *rname);
void extractPartElementSets(MATFile *pmat, odb_SetRepository esets, odb_String iname);
//*****************************************************************************
/*
***************
result function declarations
***************
*/
void extractResults(MATFile *pmat, odb_Odb& myOdb);
void extractStepResults(MATFile *pmat, odb_Step& step);
void extractFrameResults(MATFile *pmat, string& stepName, odb_Frame& cFrame);
void extractField(MATFile *pmat, string& stepName, int &frameNumber, odb_FieldOutput& field);
void getStepInfo(MATFile *pmat, odb_Step& step);
//*****************************************************************************

int ABQmain(int argc, char **argv)
{
  odb_String odbPath;
  bool ifOdbName = false;
  bool readOnly = true;
  bool ifMATPath = false;
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

  // Open the output database
  odb_Odb& myOdb = openOdb(odbPath, readOnly);
  odb_StepRepository stepRepo = myOdb.steps();
  odb_StepRepositoryIT stepIter( stepRepo );
  for (stepIter.first(); !stepIter.isDone(); stepIter.next())
  {
    odb_Step& step = stepRepo[stepIter.currentKey()];
    odb_SequenceFrame& allFramesInStep = step.frames();
    int numFrames = allFramesInStep.size();
    for (int i=0; i < numFrames; i++)
    {
      cout << "Extracting Step: " << step.name().CStr() << " Frame Number: " << i+1  << "/" << numFrames << endl;
      odb_Frame cFrame = allFramesInStep.constGet(i);
      odb_FieldOutputRepository& fieldOutputRep = cFrame.fieldOutputs();
      odb_FieldOutputRepositoryIT fieldIter( fieldOutputRep );
      for (fieldIter.first(); !fieldIter.isDone(); fieldIter.next())
      {
        cout << "Field: " << fieldIter.currentKey().CStr() << endl;
        odb_FieldOutput& field = fieldOutputRep[fieldIter.currentKey()];
        // const odb_SequenceFieldBulkData& seqDispBulkData = field.bulkDataBlocks();
        // int numDispBlocks = seqDispBulkData.size();
        // int numValues = field.values().size();
        // int i = 0;
        // for (int iblock=0; iblock<numDispBlocks; iblock++)
        // {
        //   const odb_FieldBulkData& bulkData = seqDispBulkData[iblock];
        //   int numV = bulkData.length();
        //   int numComps = bulkData.width();
        //   float* data = 0;
        //   data = bulkData.data();
        //   const odb_Instance& cinstance = bulkData.instance();
        //   // string ss = convertName(cinstance.name().CStr());
        //   // bool stringlength = ss.length();
        //   odb_Enum::odb_ResultPositionEnum position = bulkData.position();
        //   int* nodeLabels = bulkData.nodeLabels();
        //   int* elementLabels = bulkData.elementLabels();
        //   int label;
        //   for (int cvalue = 0, pos=0; cvalue < numV; cvalue++)
        //   {
        //     i++;
        //   }
        // }
        fieldOutputRep.release(fieldIter.currentKey());
      }
      fieldOutputRep.release();
      allFramesInStep.release(i);
    }
    allFramesInStep.release();
    stepRepo.release(stepIter.currentKey());
  }
  stepRepo.release();

  // MAT File Name
  const char *matFileName;
  if (!ifMATPath)
  {
    const char *odbName = myOdb.name().CStr();
  string fileName = getFileName(odbName, fileSep);
    matFileName = fileName.c_str();
  }
  else
  {
    matFileName = matPath.CStr();
  }
  MATFile *pmat;
  // pmat = matOpen(matFileName, "w");
  // Extract ODB Information
  // extractGeometry(pmat, myOdb);
  cout << "Geometry Saved " << endl;

  // extractResults(pmat, myOdb);
  cout << "Results Saved" << endl;

  // close the output database before exiting the program
  // matClose(pmat);
  myOdb.release();
  myOdb.close();
  return(0);
}

void printExecutionSummary()
{
  cout << "odbToMat.cpp \n"
	   << "Code to extract odb information to MATLAB mat files\n"
	   << "Usage: abaqus odbToMat -odb odbName -matPath matName\n"
	   << "Requirements:\n"
	   << "1. -odb      : Full path of the output database. (./work/odbName.odb)\n"
	   << "2. -matPath  : Full path to save MAT file. (./work/odbName.mat)\n"
	   << "3. -help     : Print usage";
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

string convertName(const char *cname)
{
  string s;
  s = cname;
  std::replace( s.begin(), s.end(), '-', '_');
  std::replace( s.begin(), s.end(), ' ', '_');
  return s;
}

void extractResults(MATFile *pmat, odb_Odb& myOdb)
{
    odb_StepRepository& stepRepo = myOdb.steps();
    odb_StepRepositoryIT stepIter( stepRepo );
    for (stepIter.first(); !stepIter.isDone(); stepIter.next())
    {
        // odb_Step& step = stepRepo[stepIter.currentKey()];
        extractStepResults(pmat, stepRepo[stepIter.currentKey()]);
        stepRepo.release(stepIter.currentKey());
    }
    stepRepo.release();
}

void extractStepResults(MATFile *pmat, odb_Step& step)
{
    string stepName = convertName(step.name().CStr());
    odb_SequenceFrame& allFramesInStep = step.frames();
    int numFrames = allFramesInStep.size();
    //-------------------------------------------------------------------------

    // stepInfo----------------------------------------------------------------
    getStepInfo(pmat, step);
    //-------------------------------------------------------------------------

    // framesArray-------------------------------------------------------------
    const char *framesArray_Fields[] = {"frequency", "mode"};
    mwSize dim_framesArray[2] = {1 , 1};
    //-------------------------------------------------------------------------
    int status;
    for (int i=0; i < numFrames; i++)
    {
        // MATFile *pmat;
        mxArray *freq, *eigmode;
        cout << "Extracting Step: " << stepName.c_str() << " Frame Number: " << i+1  << "/" << numFrames << endl;
        mxArray *framesArray = mxCreateStructArray(1, dim_framesArray, 2, framesArray_Fields);
        odb_Frame cFrame = allFramesInStep.get(i);
        extractFrameResults(pmat, stepName, cFrame);
        freq = mxCreateDoubleMatrix(1, 1, mxREAL);
        eigmode = mxCreateDoubleMatrix(1, 1, mxREAL);
        *mxGetPr(freq) = cFrame.frequency();
        *mxGetPr(eigmode) = cFrame.mode();
        mxSetFieldByNumber(framesArray, 0, 0, freq);
        mxSetFieldByNumber(framesArray, 0, 1, eigmode);
        stringstream convert;
        convert << i;
        // pmat = matOpen(matname, "u");
        string frameString = string("__frame__") + convert.str();
        string saveName = resultsString + stepName + frameString;
        status = matPutVariable(pmat, saveName.c_str(), framesArray);
        // matClose(pmat);
        mxDestroyArray(framesArray);
        allFramesInStep.release(i);
    }
    allFramesInStep.release();

}

void extractFrameResults(MATFile *pmat, string& stepName, odb_Frame& cFrame)
{
    odb_FieldOutputRepository& fieldOutputRep = cFrame.fieldOutputs();
    int frameNumber = cFrame.incrementNumber();
    odb_FieldOutputRepositoryIT fieldIter( fieldOutputRep );
    for (fieldIter.first(); !fieldIter.isDone(); fieldIter.next())
    {
      cout << "Field: " << fieldIter.currentKey().CStr() << endl;
      odb_FieldOutput& field = fieldOutputRep[fieldIter.currentKey()];
      extractField(pmat, stepName, frameNumber, field);
      fieldOutputRep.release(fieldIter.currentKey());
    }
    fieldOutputRep.release();
}

void extractField(MATFile *pmat, string& stepName, int &frameNumber, odb_FieldOutput& field)
{
    const odb_SequenceString componentLabels = field.componentLabels();
    const int numLabels = componentLabels.size();
    mxArray *compLabels = mxCreateCellMatrix(1, numLabels);
    for (int jj=0; jj<numLabels; jj++){
      string sss = convertName(componentLabels[jj].CStr());
      mxArray *q = mxCreateString(sss.c_str());
      mxSetCell(compLabels, jj, q);
    }
    mxArray *fieldDescription = mxCreateString(field.description().CStr());
    const odb_SequenceFieldBulkData& seqDispBulkData = field.bulkDataBlocks();
    int numDispBlocks = seqDispBulkData.size();
    int numValues = field.values().size();
    mwSize dimz[2] = {1, numValues};
    int i = 0;
    mxArray *fieldsStruct = mxCreateStructArray(2, dimz, nfields, fields);
    double *convertDouble;
    // MATFile *pmat;
    mxArray *cinstanceName, *c, *labels, *d;
    int status;
    for (int iblock=0; iblock<numDispBlocks; iblock++)
    {
        const odb_FieldBulkData& bulkData = seqDispBulkData[iblock];
        int numV = bulkData.length();
        int numComps = bulkData.width();
        float* data = 0;
        data = bulkData.data();
        const odb_Instance& cinstance = bulkData.instance();
        string ss = convertName(cinstance.name().CStr());
        bool stringlength = ss.length();
          if (!stringlength)
             cinstanceName = mxCreateString("rootAssembly");
          else
            cinstanceName = mxCreateString(ss.c_str());
        odb_Enum::odb_ResultPositionEnum position = bulkData.position();
        int* positionLabels;
        if (position == odb_Enum::NODAL)
        {
            c = mxCreateString("NODAL");
            positionLabels = bulkData.nodeLabels();
        }
        else
        {
            c = mxCreateString("ELEMENT");
            positionLabels = bulkData.elementLabels();
        }
        for (int cvalue = 0, pos=0; cvalue < numV; cvalue++)
        {
            int label;
            label = positionLabels[cvalue];
            labels = mxCreateDoubleMatrix(1, 1, mxREAL);
            // d = mxCreateDoubleMatrix(1, numComps, mxREAL);
            // convertDouble = new double[numComps];
            // for (int ccmp = 0; ccmp < numComps; ccmp++)
            // {
            //     const double cc = data[pos++];
                // convertDouble[ccmp] = cc;
            // }
            // memcpy(mxGetPr(d), convertDouble, numComps*sizeof(convertDouble));
            *mxGetPr(labels) = label;
            // mxSetFieldByNumber(fieldsStruct, i, 0, d);
            mxSetFieldByNumber(fieldsStruct, i, 1, c);
            mxSetFieldByNumber(fieldsStruct, i, 2, labels);
            mxSetFieldByNumber(fieldsStruct, i, 3, cinstanceName);
            i++;
        }
        // delete [] convertDouble;
    }
    stringstream convert;
    convert << frameNumber;
    // pmat = matOpen(matname, "u");
    string fieldString = string("__frame__") + convert.str() + string("__fieldOutput__") + field.name().CStr();
    string valueString = fieldString + string("__values");
    string compString = fieldString + string("__componentLabels");
    string desString = fieldString + string("__description");
    string saveNameValue = resultsString + stepName + valueString;
    string saveNameComp = resultsString + stepName + compString;
    string saveNameDes = resultsString + stepName + desString;
    status = matPutVariable(pmat, saveNameValue.c_str(), fieldsStruct);
    status = matPutVariable(pmat, saveNameComp.c_str(), compLabels);
    status = matPutVariable(pmat, saveNameDes.c_str(), fieldDescription);
    // matClose(pmat);
    mxDestroyArray(compLabels);
    mxDestroyArray(fieldDescription);
    // mxDestroyArray(fieldsStruct);
}

void getStepInfo(MATFile *pmat, odb_Step& step)
{
    int status;
    mwSize dim_stepInfoArray[2] = {1, 1};
    const char *stepInfoArray_fields[] = {"name", "number", "description", "nlgeom", "mass", "acousticMass", "massCenter", "inertiaAboutCenter", "inertiaAboutOrigin", "acousticMassCenter"};
    int num_stepInfoArray_fields = (sizeof(stepInfoArray_fields)/sizeof(*stepInfoArray_fields));
    mxArray *stepInfoArray = mxCreateStructArray(1, dim_stepInfoArray, num_stepInfoArray_fields, stepInfoArray_fields);
    mxArray *nameString = mxCreateString(step.name().CStr());
    mxArray *numberMat = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(numberMat) = step.number();
    mxArray *desString = mxCreateString(step.description().CStr());
    mxArray *nlgeomat = mxCreateLogicalScalar(1);
    mxArray *massMat = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(massMat) = step.mass();
    mxArray *amassMat = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(amassMat) = step.acousticMass();
    odb_SequenceDouble massCenterD = step.massCenter();
    int massSize = massCenterD.size();
    double *convertInt;
    mxArray *massC = mxCreateDoubleMatrix(1, massSize, mxREAL);
    convertInt = new double[massSize];
    for (int jj = 0; jj < massSize; jj++)
    {
        const double ccon = massCenterD[jj];
        convertInt[jj] = ccon;
    }
    memcpy(mxGetPr(massC), convertInt, massSize*sizeof(convertInt));
    delete [] convertInt;
    odb_SequenceDouble inertiaCenter = step.inertiaAboutCenter();
    int intSize2 = inertiaCenter.size();
    mxArray *intC = mxCreateDoubleMatrix(1, intSize2, mxREAL);
    convertInt = new double[intSize2];
    for (int jj = 0; jj < intSize2; jj++)
    {
        const double ccon = inertiaCenter[jj];
        convertInt[jj] = ccon;
    }
    memcpy(mxGetPr(intC), convertInt, intSize2*sizeof(convertInt));
    delete [] convertInt;
    odb_SequenceDouble inertiaOrigin = step.inertiaAboutOrigin();
    int intSize = inertiaOrigin.size();
    mxArray *int0 = mxCreateDoubleMatrix(1, intSize, mxREAL);
    convertInt = new double[intSize];
    for (int jj = 0; jj < intSize; jj++)
    {
        const double ccon = inertiaOrigin[jj];
        convertInt[jj] = ccon;
    }
    memcpy(mxGetPr(int0), convertInt, intSize*sizeof(convertInt));
    delete [] convertInt;
    odb_SequenceDouble amassCenter = step.acousticMassCenter();
    int amassCenterSize = amassCenter.size();
    mxArray *amassC = mxCreateDoubleMatrix(1, amassCenterSize, mxREAL);
    convertInt = new double[amassCenterSize];
    for (int jj = 0; jj < amassCenterSize; jj++)
    {
        const double ccon = amassCenter[jj];
        convertInt[jj] = ccon;
    }
    memcpy(mxGetPr(amassC), convertInt, amassCenterSize*sizeof(convertInt));
    delete [] convertInt;
    mxSetFieldByNumber(stepInfoArray, 0, 0, nameString);
    mxSetFieldByNumber(stepInfoArray, 0, 1, numberMat);
    mxSetFieldByNumber(stepInfoArray, 0, 2, desString);
    mxSetFieldByNumber(stepInfoArray, 0, 3, nlgeomat);
    mxSetFieldByNumber(stepInfoArray, 0, 4, massMat);
    mxSetFieldByNumber(stepInfoArray, 0, 5, amassMat);
    mxSetFieldByNumber(stepInfoArray, 0, 6, massC);
    mxSetFieldByNumber(stepInfoArray, 0, 7, intC);
    mxSetFieldByNumber(stepInfoArray, 0, 8, int0);
    mxSetFieldByNumber(stepInfoArray, 0, 9, amassC);
    // MATFile *pmat;
    // pmat = matOpen(matname, "u");
    string stepInfoString = "__stepInfo";
    string stepName = convertName(step.name().CStr());
    string saveName = resultsString + stepName + stepInfoString;
    status = matPutVariable(pmat, saveName.c_str(), stepInfoArray);
    // matClose(pmat);
    mxDestroyArray(nameString);
    mxDestroyArray(numberMat);
    mxDestroyArray(desString);
    mxDestroyArray(nlgeomat);
    mxDestroyArray(massMat);
    mxDestroyArray(amassMat);
    mxDestroyArray(massC);
    mxDestroyArray(intC);
    mxDestroyArray(int0);
    mxDestroyArray(amassC);
}

void extractGeometry(MATFile *pmat, odb_Odb& myOdb)
{
  odb_Assembly& cassemb = myOdb.rootAssembly();
  odb_SetRepository& nodeSetRepo = cassemb.nodeSets();
  odb_SetRepository& elementSetRepo = cassemb.elementSets();
  odb_Set& allnodes = nodeSetRepo[" ALL NODES"];
  odb_Set& allelements = elementSetRepo[" ALL ELEMENTS"];
  odb_SequenceString instanceNames = allnodes.instanceNames();
  int totalNames = instanceNames.size();
  for (int name=0; name < totalNames; name++) {
    const odb_String& iName = instanceNames.constGet(name);
    const odb_SequenceNode& cnodes = allnodes.nodes(iName);
    const odb_SequenceElement& celements = allelements.elements(iName);
    const char *cname = iName.cStr();
    extractInstanceNodes(pmat, cnodes, iName);
    extractInstanceElements(pmat, celements, iName);
    extractInstanceNodeSets(pmat, cassemb, iName);
    extractInstanceElementSets(pmat, cassemb, iName);
  }
}
/*
*******************************************************************************
*/
void extractInstanceNodes(MATFile *pmat, odb_SequenceNode nodeList, odb_String iname)
{
  int status;
  const char *instanceName = iname.cStr();
  int nodeListSize = nodeList.size();
  mwSize dims[2] = {1, nodeListSize};
  int label_field, coord_field;
  mwIndex i;
  mxArray *y;
  y = mxCreateStructArray(2, dims, 2, nodefieldnames);
  mxArray *label_field_value, *coord_field_value;
  for (i=0; i<nodeListSize; i++) {
    const odb_Node node = nodeList[i];
    int nodeLabel = node.label();
    const float* const coord = node.coordinates();
    label_field_value = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(label_field_value) = nodeLabel;
    mxSetFieldByNumber(y,i,0,label_field_value);
    coord_field_value = mxCreateDoubleMatrix(1,3,mxREAL);
    double convertFloat[3] = {};
    for (int j=0; j < 3; j++){
      convertFloat[j] = (double)coord[j];
    }
    memcpy(mxGetPr(coord_field_value), convertFloat, sizeof(convertFloat));
    mxSetFieldByNumber(y,i,1,coord_field_value);
    // delete [] convertFloat;
  }
  // MATFile *pmat;
  // pmat = matOpen(matname, "u");
  string saveChar = "__nodes";
  string saveName = geoChar + convertName(iname.CStr()) + saveChar;
  status = matPutVariable(pmat, saveName.c_str(), y);
  // matClose(pmat);
  // delete [] instanceName;
}
/*
*******************************************************************************
*/
void extractInstanceElements(MATFile *pmat, odb_SequenceElement elementList, odb_String iname)
{
  int status;
  const char *instanceName = iname.cStr();
  mxArray *intlabel, *connect, *eType, *instanceNames, *cinstanceName;
  double *convertInt;
  int elementListSize = elementList.size();
  mwSize dims[2] = {1, elementListSize};
  mwIndex i;
  mxArray *y;
  y = mxCreateStructArray(2, dims, 4, elementfieldnames);
  for (i=0; i<elementListSize; i++) {
    intlabel = mxCreateDoubleMatrix(1,1,mxREAL);
    const odb_Element element = elementList[i];
    int elementLabel = element.label();
    *mxGetPr(intlabel) = elementLabel;
    mxSetFieldByNumber(y,i,0,intlabel);
    const odb_String& eTypeOdb = element.type();
    eType = mxCreateString(eTypeOdb.cStr());
    mxSetFieldByNumber(y, i, 2, eType);
    const odb_SequenceString& elementINames = element.instanceNames();
    const int numInstances = elementINames.size();
    instanceNames = mxCreateCellMatrix(1, numInstances);
    for (int j=0; j<numInstances; j++){
      string s = convertName(elementINames[j].CStr());
      bool stringlength = s.length();
      if (!stringlength)
        cinstanceName = mxCreateString("rootAssembly");
      else
        cinstanceName = mxCreateString(s.c_str());
      mxSetCell(instanceNames, j, cinstanceName);
      }
    mxSetFieldByNumber(y, i, 3, instanceNames);
    const odb_SequenceInt& connectList = element.connectivity();
    int connectSize = connectList.size();
    connect = mxCreateDoubleMatrix(1,connectSize, mxREAL);
    convertInt = new double[connectSize];
    for (int j=0; j < connectSize; j++){
      const double ccon = connectList[j];
      convertInt[j] = ccon;
    }
    memcpy(mxGetPr(connect), convertInt, connectSize*sizeof(convertInt));
    mxSetFieldByNumber(y, i, 1, connect);
    delete [] convertInt;
  }
  // MATFile *pmat;
  // pmat = matOpen(matname, "u");
  string saveChar = "__elements";
  string saveName = geoChar + convertName(iname.CStr()) + saveChar;
  status = matPutVariable(pmat, saveName.c_str(), y);
  // matClose(pmat);
}
/*
*******************************************************************************
*/
void extractInstanceNodeSets(MATFile *pmat, odb_Assembly& cassemb, odb_String iname)
{
  const char *instanceName = iname.cStr();
  if (!strcmp(instanceName, rootAssemblyName))
  {
    odb_Assembly& instance = cassemb;
    odb_SetRepository nodeSets = instance.nodeSets();
    bool nsets = nodeSets.size();
    if (nsets)
      extractRootNodeSets(pmat, nodeSets, rootAssemblyName);
  }
  else
  {
    odb_InstanceRepository& iCon =  cassemb.instances();
    odb_Instance& instance = iCon[instanceName];
    odb_SetRepository nodeSets = instance.nodeSets();
    bool nsets = nodeSets.size();
    if (nsets)
      extractPartNodeSets(pmat, nodeSets, iname);
  }
}
/*
*******************************************************************************
*/
void extractRootNodeSets(MATFile *pmat, odb_SetRepository nsets, const char *rname)
{
  int status;
  const int nodeSetSize = nsets.size();
  mxArray *z, *nodeArray, *y, *cinstanceName, *instanceCellArray;
  mwSize dimsz[2] = {1, 1};
  odb_SetRepositoryIT setItN( nsets);
  const char **fieldNames = new const char*[nodeSetSize];
  int i = 0;
  for (setItN.first(); !setItN.isDone(); setItN.next())
  {
    string s;
    s = setItN.currentKey().CStr();
    std::replace( s.begin(), s.end(), '-', '_');
    std::replace( s.begin(), s.end(), ' ', '_');
    fieldNames[i] = s.c_str();
    i++;
  }
  const char **fields = (const char **) fieldNames;
  z = mxCreateStructArray(1, dimsz, nodeSetSize, fields);
  delete [] fieldNames;
  mwIndex k = 0;
  for (setItN.first(); !setItN.isDone(); setItN.next())
  {
    string sss;
    sss = setItN.currentKey().CStr();
    std::replace( sss.begin(), sss.end(), '-', '_');
    std::replace( sss.begin(), sss.end(), ' ', '_');
    if (strcmp(sss.c_str(), "_ALL_NODES")){
      odb_Set set = setItN.currentValue();
      const odb_SequenceString& nsetINames = set.instanceNames();
      const int numInstances = nsetINames.size();
      mwSize dims[2] = {1, 1};
      y = mxCreateStructArray(1, dims, 2, rootnodeSetFieldNames);
      int numNodes = 0;
      for (mwIndex j=0; j<numInstances; j++){
        odb_String name = nsetINames.constGet(j);
        const odb_SequenceNode& nodeList = set.nodes(name);
        numNodes += nodeList.size();
      }
      double *labels;
      labels = new double[numNodes];
      nodeArray = mxCreateDoubleMatrix(1,numNodes, mxREAL);
      instanceCellArray = mxCreateCellMatrix(1, numNodes);
      int nc = 0;
      for (mwIndex j=0; j<numInstances; j++){
        odb_String name = nsetINames.constGet(j);
        const odb_SequenceNode& nodeList = set.nodes(name);
        int nodeListSize = nodeList.size();
        string ss = convertName(name.cStr());
        const char *cstring = ss.c_str();
        bool stringlength = strlen(cstring);
        if (!stringlength)
          cinstanceName = mxCreateString("rootAssembly");
        else
          cinstanceName = mxCreateString(cstring);
        for (int i=0; i<nodeListSize; i++)
        {
          const odb_Node node = nodeList[i];
          int nodeLabel = node.label();
          const double nlabel = nodeLabel;
          labels[nc] = nlabel;
          mxSetCell(instanceCellArray, nc, cinstanceName);
          nc++;
        }
        // delete [] cstring;
      }
      mxSetFieldByNumber(y, 0, 1, instanceCellArray);
      memcpy(mxGetPr(nodeArray), labels, numNodes*sizeof(labels));
      mxSetFieldByNumber(y, 0 ,0, nodeArray);
      mxSetFieldByNumber(z,0,k,y);
      delete [] labels;
    }
  k ++;
  }
  mwIndex q = mxGetFieldNumber(z, "_ALL_NODES");
  mxRemoveField(z, q);
  // MATFile *pmat;
  // pmat = matOpen(matname, "u");
  string saveChar = "__nodeSets";
  string saveName = geoChar + (string)rname + saveChar;
  status = matPutVariable(pmat, saveName.c_str(), z);
  // matClose(pmat);
}
/*
*******************************************************************************
*/
void extractPartNodeSets(MATFile *pmat, odb_SetRepository nsets, odb_String iname)
{
  int status;
  const int nodeSetSize = nsets.size();
  mxArray *nodeArray, *y, *z;
  mwSize dims[2] = {1, 1};
  odb_SetRepositoryIT setItN( nsets);
  const char **fieldNames = new const char*[nodeSetSize];
  int i = 0;
  for (setItN.first(); !setItN.isDone(); setItN.next())
  {
    string s;
    s = setItN.currentKey().CStr();
    std::replace( s.begin(), s.end(), '-', '_');
    std::replace( s.begin(), s.end(), ' ', '_');
    fieldNames[i] = s.c_str();
    i++;
  }
  const char **fields = (const char **) fieldNames;
  z = mxCreateStructArray(1, dims, nodeSetSize, fields);
  delete [] fieldNames;
  mwIndex k = 0;
  for (setItN.first(); !setItN.isDone(); setItN.next())
  {
    y = mxCreateStructArray(1, dims, 1, nodeSetFieldNames);
    odb_Set set = setItN.currentValue();
    const odb_SequenceNode& nodeList = set.nodes();
    int nodeListSize = nodeList.size();
    double *labels;
    labels = new double[nodeListSize];
    nodeArray = mxCreateDoubleMatrix(1,nodeListSize, mxREAL);
    for (int i=0; i<nodeListSize; i++)
    {
      const odb_Node node = nodeList[i];
      int nodeLabel = node.label();
      const double nlabel = nodeLabel;
      labels[i] = nlabel;
    }
    memcpy(mxGetPr(nodeArray), labels, nodeListSize*sizeof(labels));
    mxSetFieldByNumber(y,0,0,nodeArray);
    mxSetFieldByNumber(z,0,k,y);
    k++;
    delete [] labels;
  }
  // MATFile *pmat;
  // pmat = matOpen(matname, "u");
  string saveChar = "__nodeSets";
  string saveName = geoChar + convertName(iname.CStr()) + saveChar;
  status = matPutVariable(pmat, saveName.c_str(), y);
  // matClose(pmat);
}
/*
*******************************************************************************
*/
void extractInstanceElementSets(MATFile *pmat, odb_Assembly& cassemb, odb_String iname)
{
  const char *instanceName = iname.cStr();
  if (!strcmp(instanceName, rootAssemblyName))
  {
    odb_Assembly& instance = cassemb;
    odb_SetRepository elementSets = instance.elementSets();
    bool esets = elementSets.size();
    if (esets)
      extractRootElementSets(pmat, elementSets, rootAssemblyName);
  }
  else
  {
    odb_InstanceRepository& iCon =  cassemb.instances();
    odb_Instance& instance = iCon[instanceName];
    odb_SetRepository elementSets = instance.elementSets();
    bool esets = elementSets.size();
    if (esets)
      extractPartElementSets(pmat, elementSets, iname);
  }
}
/*
*******************************************************************************
*/
void extractRootElementSets(MATFile *pmat, odb_SetRepository esets, const char *rname)
{
  int status;
  const int elementSetSize = esets.size();
  mxArray *z, *elementArray, *y, *cinstanceName, *instanceCellArray;
  mwSize dimsz[2] = {1, 1};
  odb_SetRepositoryIT setItN( esets);
  const char **fieldNames = new const char*[elementSetSize];
  int i = 0;
  for (setItN.first(); !setItN.isDone(); setItN.next())
  {
    string s;
    s = setItN.currentKey().CStr();
    std::replace( s.begin(), s.end(), '-', '_');
    std::replace( s.begin(), s.end(), ' ', '_');
    fieldNames[i] = s.c_str();
    i++;
  }
  const char **fields = (const char **) fieldNames;
  z = mxCreateStructArray(1, dimsz, elementSetSize, fields);
  delete [] fieldNames;
  mwIndex k = 0;
  for (setItN.first(); !setItN.isDone(); setItN.next())
  {
    string s;
    s = setItN.currentKey().CStr();
    std::replace( s.begin(), s.end(), '-', '_');
    std::replace( s.begin(), s.end(), ' ', '_');
    if (strcmp(s.c_str(), "_ALL_ELEMENTS")){
      odb_Set set = setItN.currentValue();
      const odb_SequenceString& esetINames = set.instanceNames();
      const int numInstances = esetINames.size();
      mwSize dims[2] = {1, 1};
      y = mxCreateStructArray(1, dims, 2, rootelementSetFieldNames);
      int numelements = 0;
      for (mwIndex j=0; j<numInstances; j++){
        odb_String name = esetINames.constGet(j);
        const odb_SequenceElement& elementList = set.elements(name);
        numelements += elementList.size();
      }
      double *labels;
      labels = new double[numelements];
      elementArray = mxCreateDoubleMatrix(1,numelements, mxREAL);
      instanceCellArray = mxCreateCellMatrix(1, numelements);
      int nc = 0;
      for (mwIndex j=0; j<numInstances; j++){
        odb_String name = esetINames.constGet(j);
        const odb_SequenceElement& elementList = set.elements(name);
        int elementListSize = elementList.size();
        string s = convertName(name.cStr());
        const char *cstring = s.c_str();
        bool stringlength = strlen(cstring);
        if (!stringlength)
          cinstanceName = mxCreateString("rootAssembly");
        else
          cinstanceName = mxCreateString(cstring);
        for (int i=0; i<elementListSize; i++)
        {
          const odb_Element element = elementList[i];
          int elementLabel = element.label();
          const double elabel = elementLabel;
          labels[nc] = elabel;
          mxSetCell(instanceCellArray, nc, cinstanceName);
          nc++;
        }
      }
      mxSetFieldByNumber(y, 0, 1, instanceCellArray);
      memcpy(mxGetPr(elementArray), labels, numelements*sizeof(labels));
      mxSetFieldByNumber(y, 0 ,0, elementArray);
      mxSetFieldByNumber(z,0,k,y);
      delete [] labels;
    }
  k++;
  }
  mwIndex q = mxGetFieldNumber(z, "_ALL_ELEMENTS");
  mxRemoveField(z, q);
  // MATFile *pmat;
  // pmat = matOpen(matname, "u");
  string saveChar = "__elementSets";
  string saveName = geoChar + (string)rname + saveChar;
  status = matPutVariable(pmat, saveName.c_str(), z);
  // matClose(pmat);
  // mxDestroyArray(z);
}
/*
*******************************************************************************
*/
void extractPartElementSets(MATFile *pmat, odb_SetRepository esets, odb_String iname)
{
  int status;
  const int elementSetSize = esets.size();
  mxArray *elementArray, *y, *z;
  mwSize dims[2] = {1, 1};
  odb_SetRepositoryIT setItN( esets);
  const char **fieldNames = new const char*[elementSetSize];
  int i = 0;
  for (setItN.first(); !setItN.isDone(); setItN.next())
  {
    string s;
    s = setItN.currentKey().CStr();
    std::replace( s.begin(), s.end(), '-', '_');
    std::replace( s.begin(), s.end(), ' ', '_');
    fieldNames[i] = s.c_str();
    i++;
  }
  const char **fields = (const char **) fieldNames;
  z = mxCreateStructArray(1, dims, elementSetSize, fields);
  delete [] fieldNames;
  mwIndex k = 0;
  for (setItN.first(); !setItN.isDone(); setItN.next())
  {
    y = mxCreateStructArray(1, dims, 1, elementSetFieldNames);
    odb_Set set = setItN.currentValue();
    const odb_SequenceElement& elementList = set.elements();
    int elementListSize = elementList.size();
    double *labels;
    labels = new double[elementListSize];
    elementArray = mxCreateDoubleMatrix(1,elementListSize, mxREAL);
    for (int i=0; i<elementListSize; i++)
    {
      const odb_Element element = elementList[i];
      int elementLabel = element.label();
      const double nlabel = elementLabel;
      labels[i] = nlabel;
    }
    memcpy(mxGetPr(elementArray), labels, elementListSize*sizeof(labels));
    mxSetFieldByNumber(y,0,0,elementArray);
    mxSetFieldByNumber(z,0,k,y);
    k ++;
    delete [] labels;
  }
  // MATFile *pmat;
  // pmat = matOpen(matname, "u");
  string saveChar = "__elementSets";
  string saveName = geoChar + convertName(iname.CStr()) + saveChar;
  status = matPutVariable(pmat, saveName.c_str(), z);
  // matClose(pmat);
}

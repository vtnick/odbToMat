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
// Abaqus includes
//
#include <odb_API.h>
#include <sys/stat.h>

//
// MATLAB Includes
//
#include <string.h> /* For strcmp() */
#include <vector> /* For STL */
#include "mat.h"

#include "geometryFunctions.h"
#include "convertFunctions.h"

mxArray* extractGeometry(odb_Assembly& cassemb)
{
  odb_SetRepository& nodeSetRepo = cassemb.nodeSets();
  odb_SetRepository& elementSetRepo = cassemb.elementSets();
  odb_Set& allnodes = nodeSetRepo[" ALL NODES"];
  odb_Set& allelements = elementSetRepo[" ALL ELEMENTS"];
  odb_SequenceString instanceNames = allnodes.instanceNames();
  int totalNames = instanceNames.size();
  char **fieldNames = new char*[totalNames];
  char *cnametemp;
  for (int i=0; i < totalNames; i++)
  {
    const odb_String& iName = instanceNames.constGet(i);
    string s = convertName(iName.CStr());
    cnametemp = new char[s.size() + 1];
    strcpy ( cnametemp, s.c_str() );
    fieldNames[i] = cnametemp;
  }
  mxArray *y, *z;
  mwSize dims[2] = {1,1};
  // const char **fields = (const char **) fieldNames;
  const char *fields[] = {"root1", "root2","root3","root4","root5","root6","root7","root8","root9","root10","root11","root12"};
  const char *instanceFieldNames[] = {"nodes", "elements", "nodeSets", "elementSets"};
  y = mxCreateStructArray(1, dims, totalNames, fields);
  delete [] cnametemp;
  delete [] fieldNames;
  for (int name=0; name<totalNames; name++) {
    mxArray *nodesStruct, *elementStruct, *nodeSetStruct, *elementSetStruct;
    z = mxCreateStructArray(1, dims, 4, instanceFieldNames);
    mwIndex k = name;
    const odb_String& iName = instanceNames.constGet(name);
    const odb_SequenceNode& cnodes = allnodes.nodes(iName);
    const odb_SequenceElement& celements = allelements.elements(iName);
    const char *cname = iName.cStr();
    nodesStruct = extractInstanceNodes(cnodes, iName);
    elementStruct = extractInstanceElements(celements, iName);
    nodeSetStruct = extractInstanceNodeSets(cassemb, iName);
    elementSetStruct = extractInstanceElementSets(cassemb, iName);
    mxSetFieldByNumber(z, 0, 0, nodesStruct);
    mxSetFieldByNumber(z, 0, 1, elementStruct);
    mxSetFieldByNumber(z, 0, 2, nodeSetStruct);
    mxSetFieldByNumber(z, 0, 3, elementSetStruct);
    mxSetFieldByNumber(y, 0, k, z);
    // mxDestroyArray(nodesStruct);
    // mxDestroyArray(elementStruct);
    // mxDestroyArray(nodeSetStruct);
    // mxDestroyArray(elementSetStruct);
  }
  // mxDestroyArray(z);
  return y;
}
/*
*******************************************************************************
*/
mxArray* extractInstanceNodes(odb_SequenceNode nodeList, odb_String iname)
{
  const char *nodefieldnames[] = {"label", "coords"};
  const char *instanceName = iname.cStr();
  int nodeListSize = nodeList.size();
  mwSize dims[2] = {1, nodeListSize};
  int label_field, coord_field;
  mwIndex i;
  mxArray *y;
  y = mxCreateStructArray(2, dims, 2, nodefieldnames);
  label_field = mxGetFieldNumber(y, "label");
  coord_field = mxGetFieldNumber(y, "coords");
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
    }
  // mxSetFieldByNumber(x, 0, 0, y);
  // mxDestroyArray(y);
  //   MATFile *pmat;
  // pmat = matOpen("test.mat", "w");
  // int status = matPutVariable(pmat, "geometry", y);
  // matClose(pmat);
  return y;
}
/*
*******************************************************************************
*/
mxArray* extractInstanceElements(odb_SequenceElement elementList, odb_String iname)
{
  const char *instanceName = iname.cStr();
  mxArray *intlabel, *connect, *eType, *instanceNames, *cinstanceName;
  double *convertInt;
  const char *elementfieldnames[] = {"label", "connectivity", "eType", "instanceNames"};
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
      // cout << cstring << elementINames[j].CStr() << endl;
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
  }
  // mxSetFieldByNumber(x, 0, 1, y);
  // mxDestroyArray(y);
  return y;
}
/*
*******************************************************************************
*/
mxArray* extractInstanceNodeSets(odb_Assembly& cassemb, odb_String iname)
{
  mxArray *x;
  const char *instanceName = iname.cStr();
  const char *rootAssemblyName = "rootAssembly";
  if (!strcmp(instanceName, rootAssemblyName))
  {
    odb_Assembly& instance = cassemb;
    odb_SetRepository nodeSets = instance.nodeSets();
    bool nsets = nodeSets.size();
    if (nsets)
      x = extractRootNodeSets(nodeSets);
  }
  else
  {
    odb_InstanceRepository& iCon =  cassemb.instances();
    odb_Instance& instance = iCon[instanceName];
    odb_SetRepository nodeSets = instance.nodeSets();
    bool nsets = nodeSets.size();
    if (nsets)
      x = extractPartNodeSets(nodeSets);
  }
  return x;
}
/*
*******************************************************************************
*/
mxArray* extractRootNodeSets(odb_SetRepository nsets)
{
  const int nodeSetSize = nsets.size();
  const char *nodeSetFieldNames[] = {"labels", "instanceNames"};
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
    i += 1;
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
      y = mxCreateStructArray(1, dims, 2, nodeSetFieldNames);
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
          nc += 1;
        }
      }
      mxSetFieldByNumber(y, 0, 1, instanceCellArray);
      memcpy(mxGetPr(nodeArray), labels, numNodes*sizeof(labels));
      mxSetFieldByNumber(y, 0 ,0, nodeArray);
      mxSetFieldByNumber(z,0,k,y);
      // delete [] labels;
    }
  k ++;
  }
  mwIndex q = mxGetFieldNumber(z, "_ALL_NODES");
  mxRemoveField(z, q);
  // mxSetFieldByNumber(x, 0, 2, z);
  // mxDestroyArray(y);
  // mxDestroyArray(z);
  return z;
}
/*
*******************************************************************************
*/
mxArray* extractPartNodeSets(odb_SetRepository nsets)
{
  const int nodeSetSize = nsets.size();
  const char *nodeSetFieldNames[] = {"labels"};
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
    i += 1;
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
    k += 1;
  }
  // mxSetFieldByNumber(x, 0, 2, z);
  // mxDestroyArray(z);
  return z;
}
/*
*******************************************************************************
*/
mxArray* extractInstanceElementSets(odb_Assembly& cassemb, odb_String iname)
{
  mxArray *x;
  const char *instanceName = iname.cStr();
  const char *rootAssemblyName = "rootAssembly";
  if (!strcmp(instanceName, rootAssemblyName))
  {
    odb_Assembly& instance = cassemb;
    odb_SetRepository elementSets = instance.elementSets();
    bool esets = elementSets.size();
    if (esets)
      x = extractRootElementSets(elementSets);
  }
  else
  {
    odb_InstanceRepository& iCon =  cassemb.instances();
    odb_Instance& instance = iCon[instanceName];
    odb_SetRepository elementSets = instance.elementSets();
    bool esets = elementSets.size();
    if (esets)
      x = extractPartElementSets(elementSets);
  }
  return x;
}
/*
*******************************************************************************
*/
mxArray* extractRootElementSets(odb_SetRepository esets)
{
  const int elementSetSize = esets.size();
  const char *elementSetFieldNames[] = {"labels", "instanceNames"};
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
    i += 1;
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
      y = mxCreateStructArray(1, dims, 2, elementSetFieldNames);
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
          nc += 1;
        }
      }
      mxSetFieldByNumber(y, 0, 1, instanceCellArray);
      memcpy(mxGetPr(elementArray), labels, numelements*sizeof(labels));
      mxSetFieldByNumber(y, 0 ,0, elementArray);
      mxSetFieldByNumber(z,0,k,y);
    }
  k += 1;
  }
  mwIndex q = mxGetFieldNumber(z, "_ALL_ELEMENTS");
  mxRemoveField(z, q);
  // mxSetFieldByNumber(x, 0, 3, z);
  // mxDestroyArray(z);
  return z;
}
/*
*******************************************************************************
*/
mxArray* extractPartElementSets(odb_SetRepository esets)
{
  const int elementSetSize = esets.size();
  const char *elementSetFieldNames[] = {"labels"};
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
    i += 1;
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
  }
  // mxSetFieldByNumber(x, 0, 3, z);
  // mxDestroyArray(z);
  return z;
}

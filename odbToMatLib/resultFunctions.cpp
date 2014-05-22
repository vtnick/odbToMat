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

#include "resultFunctions.h"
#include "convertFunctions.h"


mxArray * extractResults(odb_Odb& myOdb)
{
    odb_StepRepository& sRep1 = myOdb.steps();
    odb_StepRepositoryIT stepIter( sRep1 );
    mxArray *stepsArray, *odbArray;
    int totalSteps = sRep1.size();
    int i = 0;
    char **fieldNames = new char*[totalSteps];
    char *cnametemp;
    for (stepIter.first(); !stepIter.isDone(); stepIter.next())
    {
        odb_Step& step = sRep1[stepIter.currentKey()];
        string s = convertName(step.name().CStr());
        cnametemp = new char[s.size() + 1];
        strcpy ( cnametemp, s.c_str() );
        fieldNames[i] = cnametemp;
        i += 1;
    }
    const char **fields = (const char **) fieldNames;
    mwSize dims[2] = {1, 1};
    stepsArray = mxCreateStructArray(1, dims, totalSteps, fields);
    delete [] cnametemp;
    delete [] fieldNames;
    const char *zfields[] = {"steps"};
    odbArray = mxCreateStructArray(1, dims, 1, zfields);
    mwIndex k = 0;
    for (stepIter.first(); !stepIter.isDone(); stepIter.next())
    {
        odb_Step& step = sRep1[stepIter.currentKey()];
        stepsArray = extractStepResults(stepsArray, step, k);
        k += 1;
    }
    mxSetFieldByNumber(odbArray, 0, 0, stepsArray);
    return odbArray;
}

mxArray * extractStepResults(mxArray *stepArray, odb_Step& step, mwIndex k)
{
    mxArray *frameStruct;
    odb_SequenceFrame& allFramesInStep = step.frames();
    int numFrames = allFramesInStep.size();

    // cStepArray -------------------------------------------------------------
    const char *cStepArray_fields[] = {"stepInfo", "frames"};
    int num_cStepArray_fields = (sizeof(cStepArray_fields)/sizeof(*cStepArray_fields));
    mwSize dim_cStepArray[2] = {1, 1};
    mxArray *cStepArray = mxCreateStructArray(1, dim_cStepArray, num_cStepArray_fields, cStepArray_fields);
    //-------------------------------------------------------------------------

    // stepInfoArray-----------------------------------------------------------
    mwSize dim_stepInfoArray[2] = {1, 1};
    const char *stepInfoArray_fields[] = {"name", "number", "description", "nlgeom", "mass", "acousticMass", "massCenter", "inertiaAboutCenter", "inertiaAboutOrigin", "acousticMassCenter"};
    int num_stepInfoArray_fields = (sizeof(stepInfoArray_fields)/sizeof(*stepInfoArray_fields));
    mxArray *stepInfoArray = mxCreateStructArray(1, dim_stepInfoArray, num_stepInfoArray_fields, stepInfoArray_fields);
    stepInfoArray = getStepInfo(step, stepInfoArray);
    //-------------------------------------------------------------------------

    // framesArray-------------------------------------------------------------
    const char *framesArray_Fields[] = {"fieldOutput", "frequency", "mode"};
    mwSize dim_framesArray[2] = {1 , numFrames};
    mxArray *framesArray = mxCreateStructArray(2, dim_framesArray, 3, framesArray_Fields);
    //-------------------------------------------------------------------------

    odb_FieldOutputRepository& fieldOutputRep = allFramesInStep[numFrames-1].fieldOutputs();
    odb_FieldOutputRepositoryIT fieldIter( fieldOutputRep );
    int totalFields = fieldOutputRep.size();
    int i = 0;
    char **fieldNames = new char*[totalFields];
    char *cnametemp;
    for (fieldIter.first(); !fieldIter.isDone(); fieldIter.next())
    {
        string s = convertName(fieldIter.currentKey().CStr());
        cnametemp = new char[s.size() + 1];
        strcpy ( cnametemp, s.c_str() );
        fieldNames[i] = cnametemp;
        i += 1;
    }
    const char **fields = (const char **) fieldNames;
    mwSize dimf[2] = {1, 1};
    mxArray *freq, *eigmode;
    for (int i=0; i < numFrames; i++)
    {
        frameStruct = mxCreateStructArray(1, dimf, totalFields, fields);
        odb_Frame& cFrame = allFramesInStep[i];
        frameStruct = extractFrameResults(frameStruct, cFrame);
        mxSetFieldByNumber(framesArray, i, 0, frameStruct);
        freq = mxCreateDoubleMatrix(1, 1, mxREAL);
        eigmode = mxCreateDoubleMatrix(1, 1, mxREAL);
        *mxGetPr(freq) = cFrame.frequency();
        *mxGetPr(eigmode) = cFrame.mode();
        mxSetFieldByNumber(framesArray, i, 1, freq);
        mxSetFieldByNumber(framesArray, i, 2, eigmode);
    }
    delete [] cnametemp;
    delete [] fieldNames;
    mxSetFieldByNumber(cStepArray, 0, 0, stepInfoArray);
    mxSetFieldByNumber(cStepArray, 0, 1, framesArray);
    mxSetFieldByNumber(stepArray, 0, k, cStepArray);
    return stepArray;

}

mxArray * extractFrameResults(mxArray *frameStruct, odb_Frame& cFrame)
{
    mxArray *fieldsArray;
    const char *fieldFields[] = {"values", "componentLabels", "description"};
    int numfieldFields = (sizeof(fieldFields)/sizeof(*fieldFields));
    odb_FieldOutputRepository& fieldOutputRep = cFrame.fieldOutputs();
    odb_FieldOutputRepositoryIT fieldIter( fieldOutputRep );
    int totalFields = fieldOutputRep.size();
    mwSize dims[2] = {1,1};
    mwIndex j = 0;
    for (fieldIter.first(); !fieldIter.isDone(); fieldIter.next())
    {
        mxArray *cFrame = mxCreateStructArray(1, dims, numfieldFields, fieldFields);
        fieldsArray = extractField(fieldOutputRep, fieldIter.currentKey());
        mxSetFieldByNumber(cFrame, 0, 0, fieldsArray);
        odb_FieldOutput& field = fieldOutputRep[fieldIter.currentKey()];
        const odb_SequenceString& componentLabels = field.componentLabels();
        const int numLabels = componentLabels.size();
        mxArray *compLabels = mxCreateCellMatrix(1, numLabels);
        for (int j=0; j<numLabels; j++){
          string sss = convertName(componentLabels[j].CStr());
          mxArray *q = mxCreateString(sss.c_str());
          mxSetCell(compLabels, j, q);
          // mxDestroyArray(q);
        }
        mxArray *fieldDescription = mxCreateString(field.description().CStr());
        mxSetFieldByNumber(cFrame, 0, 1, compLabels);
        mxSetFieldByNumber(cFrame, 0, 2, fieldDescription);
        mxSetFieldByNumber(frameStruct, 0, j, cFrame);
        j += 1;
    }
    return frameStruct;
}

mxArray * extractField(odb_FieldOutputRepository& fieldOutputRep, odb_String fieldKey)
{
    const char *fields[] = {"data", "position", "labels", "instance"};
    int nfields = 4;
    mxArray *c ;
    odb_FieldOutput& field = fieldOutputRep[fieldKey];
    const odb_SequenceFieldBulkData& seqDispBulkData = field.bulkDataBlocks();
    const odb_SequenceString& componentLabels = field.componentLabels();
    const int numLabels = componentLabels.size();
    int numDispBlocks = seqDispBulkData.size();
    int numValues = field.values().size();
    mwSize dimz[2] = {1, numValues};
    int i = 0;
    mxArray *fieldsStruct = mxCreateStructArray(2, dimz, nfields, fields);
    int j = 0;
    double *convertDouble, convertLabels;
    for (int iblock=0; iblock<numDispBlocks; iblock++)
    {
        const odb_FieldBulkData& bulkData = seqDispBulkData[iblock];
        int numV = bulkData.length();
        j += numV;
        int numComps = bulkData.width();
        float* data = bulkData.data();
        const odb_Instance& cinstance = bulkData.instance();
        string ss = convertName(cinstance.name().CStr());
        bool stringlength = ss.length();
        mxArray *cinstanceName;
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
            int label = positionLabels[cvalue];
            // convertLabels = (double) positionLabels[cvalue];
            mxArray *labels = mxCreateDoubleMatrix(1, 1, mxREAL);
            mxArray *d = mxCreateDoubleMatrix(1, numComps, mxREAL);
            convertDouble = new double[numComps];
            for (int ccmp = 0; ccmp < numComps; ccmp++)
            {
                const double cc = data[pos++];
                convertDouble[ccmp] = cc;
            }
            memcpy(mxGetPr(d), convertDouble, numComps*sizeof(convertDouble));
            *mxGetPr(labels) = label;
            mxSetFieldByNumber(fieldsStruct, i, 0, d);
            mxSetFieldByNumber(fieldsStruct, i, 1, c);
            mxSetFieldByNumber(fieldsStruct, i, 2, labels);
            mxSetFieldByNumber(fieldsStruct, i, 3, cinstanceName);
            i += 1;
        }
    }
    return fieldsStruct;
}

mxArray * getStepInfo(odb_Step& step, mxArray *stepInfoArray)
{
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
    return stepInfoArray;
}

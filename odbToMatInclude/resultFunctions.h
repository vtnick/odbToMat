extern mxArray * extractResults(odb_Odb& myOdb);

extern mxArray * extractStepResults(mxArray *x, odb_Step& step, mwIndex k);

extern mxArray * extractFrameResults(mxArray *f, odb_Frame& cFrame);

extern mxArray * extractField(odb_FieldOutputRepository& fieldOutputRep, odb_String fieldKey);

extern mxArray * getStepInfo(odb_Step& step, mxArray *stepInfoArray);

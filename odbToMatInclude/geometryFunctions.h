//*****************************************************************************
extern mxArray* extractGeometry(odb_Assembly& cassemb);

//*****************************************************************************
extern mxArray* extractInstanceNodes(odb_SequenceNode nodeList, odb_String iname);

//*****************************************************************************
extern mxArray* extractInstanceElements(odb_SequenceElement elementList, odb_String iname);

//*****************************************************************************
extern mxArray* extractRootNodeSets(odb_SetRepository nsets);

//*****************************************************************************
extern mxArray* extractPartNodeSets(odb_SetRepository nsets);

//*****************************************************************************
extern mxArray* extractInstanceNodeSets(odb_Assembly& cassemb, odb_String iname);

//*****************************************************************************
extern mxArray* extractInstanceElementSets(odb_Assembly& cassemb, odb_String iname);

//*****************************************************************************
extern mxArray* extractRootElementSets(odb_SetRepository esets);

//*****************************************************************************
extern mxArray* extractPartElementSets(odb_SetRepository esets);

//*****************************************************************************

from os import path, chdir, makedirs, walk, unlink, getcwd
from shutil import rmtree
from numpy import ix_
import numpy as np
import scipy.io as sio
from sys import argv
from odbAccess import *
from abaqusConstants import *
import inpParser


def extractAssembly(odb, work_dir):
    # Make Folders
    odbName = odb.name.split('/')[-1].split('.')[0]
    odbWorkPath = path.join(work_dir, odbName)
    assemblyPath = path.join(odbWorkPath, 'assembly')
    assembly = odb.rootAssembly
    if path.isdir(assemblyPath):
        for root, dirs, files in walk(odbWorkPath):
            for f in files:
                unlink(path.join(root, f))
            for d in dirs:
                rmtree(path.join(root, d))
    makedirs(assemblyPath)
    p = path.abspath(assemblyPath)
    chdir(p)
    pathToOdb, odbName = path.split(odb.path)
    pathToPes = path.join(pathToOdb, odbName.split('.')[0] + '.pes')
    pathToInp = path.join(pathToOdb, odbName.split('.')[0] + '.inp')
    if path.exists(pathToPes):
        inputFile = inpParser.InputFile(pathToPes)
    elif path.exists(pathToInp):
        inputFile = inpParser.InputFile(pathToInp)
    else:
        print "No input file found in ODB directory found"
    p = inputFile.parse(usePyArray=True)
    nodes = []
    nodeLabels = []
    elementLabels = []
    for i, j in enumerate(p):
        if j.name == 'assembly':
            lower = i
        elif j.name == 'endassembly':
            upper = i
    for i in range(lower+1, upper):
        if p[i].name.lower() == 'node':
            nodeLabels.append(p[i].data[0][0])
            nodes.append(p[i].data[0][0:4])
            sio.savemat('nodes', {'nodes': nodes})
        elif p[i].name.lower() == 'element':
            elementNumber = p[i].data[0][0]
            elementConnect = p[i].data[0][1:]
            ccon = {}
            for k, c in enumerate(elementConnect):
                ccon['node_%d' % (k+1)] = c
            ccon['eType'] = p[i].parameter['type']
            sio.savemat('element_%d' % (elementNumber), ccon, appendmat=True)
            elementLabels.append(elementNumber)
        elif p[i].name.lower() == 'nset':
            nodes = p[i].data[0]
            parameters = p[i].parameter
            parameters['nodes'] = [q for q in nodes if q]
            sio.savemat('nset_' + p[i].parameter['nset'], parameters)
        elif p[i].name.lower() == 'elset':
            elements = p[i].data[0]
            parameters = p[i].parameter
            if 'interal' in parameters.keys():
                parameters['internal'] = p[i].parameter['internal'].name
            elif 'generate' in parameters.keys():
                parameters['generate'] = p[i].parameter['generate'].name
            parameters['elements'] = [q for q in elements if q]
            sio.savemat('elset_' + p[i].parameter['elset'], parameters)

    if nodeLabels:
        assembly.NodeSetFromNodeLabels(name='ALL_NODES_CREATED', nodeLabels=(('', nodeLabels), ))

    if elementLabels:
        assembly.ElementSetFromElementLabels(name='ALL_ELEMENTS_CREATED', elementLabels=(('', elementLabels), ))

    for root, dirs, files in walk(assemblyPath):
        if not files:
            rmtree(assemblyPath)

def extractParts(odb, work_dir):
    # Get Instance Names
    instances = odb.rootAssembly.instances
    instanceKeys = instances.keys()
    if instanceKeys:
        # Make Folders
        odbName = odb.name.split('/')[-1].split('.')[0]
        odbWorkPath = path.join(work_dir, odbName)
        partsPath = path.join(odbWorkPath, 'parts')
        if path.isdir(partsPath):
            for root, dirs, files in walk(odbWorkPath):
                for f in files:
                    unlink(path.join(root, f))
                for d in dirs:
                    rmtree(path.join(root, d))
        makedirs(partsPath)
        p = path.abspath(partsPath)
        chdir(partsPath)

        # Extract Values
        for instance in instanceKeys:
            # Setup Part folders
            instanceName = instance.replace('-', '_')
            cPartPath = path.join(instanceName)
            makedirs(cPartPath)
            chdir(cPartPath)

            # Create Current Part
            cInstance = instances[instance]
            nodes = cInstance.nodes
            elements = cInstance.elements
            nodeSets = cInstance.nodeSets
            elementSets = cInstance.elementSets

            # Extract Nodes
            nodeArray = np.zeros((len(nodes), 4))
            for i, node in enumerate(nodes):
                nodeArray[i, 0] = node.label
                nodeArray[i, 1:] = node.coordinates
            sio.savemat('nodes.mat', {'nodes': nodeArray})

            # Write Elements To File
            nelements = len(elements)
            maxC = max([len(i.connectivity) for i in elements])
            eLabels = np.zeros((nelements, 1))
            eConn = np.zeros((nelements, maxC))
            eTypes = np.zeros((nelements,), dtype=np.object)
            for i, element in enumerate(elements):
                eLabels[i] = element.label
                eConn[i, :] = list(element.connectivity)
                eTypes[i] = element.type
            sio.savemat('elements.mat', {'labels': eLabels, 'connect': eConn, 'eTypes': eTypes})

            # Write Node Sets To File
            nKeys = nodeSets.keys()
            if nKeys:
                maxNodes = max([len(nodeSets[i].nodes) for i in nKeys])
                nSets = np.array(nKeys, dtype=np.object)
                nSetNodes = np.zeros((len(nKeys), maxNodes))
                for i, nset in enumerate(nKeys):
                    cset = nodeSets[nset].nodes
                    cnodes = []
                    for node in cset:
                        cnodes.append(node.label)
                    nSetNodes[ix_([i], np.arange(0, len(cnodes)))] = cnodes
                sio.savemat('nodeSets.mat', {'nSets': nSets, 'nSetNodes': nSetNodes})

            # Write Element Sets To File
            eSetKeys = elementSets.keys()
            if eSetKeys:
                maxElems = max([len(elementSets[i].elements) for i in eSetKeys])
                eSets = np.array(eSetKeys, dtype=np.object)
                eSetElems = np.zeros((len(eSetKeys), maxElems))
                for i, eset in enumerate(eSetKeys):
                    cset = elementSets[eset].elements
                    celements = []
                    for element in cset:
                        celements.append(element.label)
                    eSetElems[ix_([i], np.arange(0, len(celements)))] = celements
                sio.savemat('elementSets.mat', {'eSets': eSets, 'eSetElems': eSetElems})

            # Change Back To Root Parts Directory
            chdir(p)


def extractFieldOutputs(currentFrame, frameNumber, instances, assembly):
    assemblySets = ['ALL_NODES_CREATED', 'ALL_ELEMENTS_CREATED']
    instanceKeys = instances.keys()
    fieldOutputs = currentFrame.fieldOutputs
    fields = fieldOutputs.keys()
    for field in fields:
        cfield = fieldOutputs[field]
        position = cfield.locations[0].position.name
        if position == 'NODAL':
            location = 'nodeLabel'
            n = True
        else:
            location = 'elementLabel'
            n = False
        for instance in instanceKeys:
            instanceValues = cfield.getSubset(region=instances[instance])
            componentLabels = instanceValues.componentLabels
            description = instanceValues.description
            instanceName = instance.replace('-', '_')
            nValues = len(instanceValues.values)
            # length of data array
            nCols = max(1, len(np.atleast_1d(np.array(instanceValues.values[0].data))))
            valueArray = np.zeros((nValues, nCols + 1))
            # print len(instanceValues.values[0].data)
            for k, cvalue in enumerate(instanceValues.values):
                valueArray[k, 0] = cvalue.__getattribute__(location)
                valueArray[k, 1:] = cvalue.data
            if nValues:
                valueArraySorted = valueArray[valueArray[:, 0].argsort()]
            sio.savemat('frame_%d_%s_%s.mat' % (frameNumber, instanceName, field.replace(' ', '_')), {'componentLabels': np.array([componentLabels], dtype=object), 'description': description, 'values': valueArraySorted})
        a = False
        if n:
            if assemblySets[0] in assembly.nodeSets.keys():
                a = True
                assemblyValues = cfield.getSubset(region=assembly.nodeSets[assemblySets[0]])
        else:
            if assemblySets[0] in assembly.elementSets.keys():
                a = True
                assemblyValues = cfield.getSubset(region=assembly.elementSets[assemblySets[1]])
        if a:
            componentLabels = assemblyValues.componentLabels
            description = assemblyValues.description
            instanceName = instance.replace('-', '_')
            nValues = len(assemblyValues.values)
            nCols = max(1, len(componentLabels))
            valueArray = np.zeros((nValues, nCols + 1))
            for k, cvalue in enumerate(assemblyValues.values):
                valueArray[k, 0] = cvalue.__getattribute__(location)
                valueArray[k, 1:] = cvalue.data
            if nValues:
                valueArraySorted = valueArray[valueArray[:, 0].argsort()]
            sio.savemat('frame_%d_%s_%s.mat' % (frameNumber, 'Assembly', field.replace(' ', '_')), {'componentLabels': np.array([componentLabels], dtype=object), 'description': description, 'values': valueArraySorted})


def extractFrames(frames, stepName, instances, assembly):
    p = path.abspath(getcwd())
    stepDir = path.abspath(stepName)
    chdir(stepDir)
    nFrameMembers = ['cyclicModeNumber', 'description', 'incrementNumber', 'frameValue', 'domain', 'frequency', 'mode']
    aFrameMembers = ['fieldOutputs', 'loadCase', 'associatedFrame']
    nframes = len(frames)
    for frameCount, cFrame in enumerate(frames):
        frameDir = 'frame_%d' % (frameCount)
        makedirs(frameDir)
        chdir(frameDir)
        cFrameDict = {}
        for i in nFrameMembers:
            if i == 'domain':
                cFrameDict[i] = np.array([cFrame.__getattribute__(i).name])
            else:
                x = cFrame.__getattribute__(i)
                if not x:
                    cFrameDict[i] = np.array([0])
                else:
                    cFrameDict[i] = np.array([x])
        sio.savemat('frame_%d_Dict.mat' % (frameCount), cFrameDict)
        extractFieldOutputs(cFrame, frameCount, instances, assembly)
        chdir(stepDir)

    chdir(p)


def extractHistoryRegions(historyRegions, stepName):
    print 'historyRegions'


def extractLoadCases(loadCases, stepName):
    print 'loadCases'


def extractRetainedNodalDofs(retainedNodalDofs, stepName):
    print 'retainedNodalDofs'


def extractEliminatedNodalDofs(eliminatedNodalDofs, stepName):
    print 'eliminatedNodalDofs'


def extractResults(odb, work_dir):
    # Make Folders
    instances = odb.rootAssembly.instances
    odbName = odb.name.split('/')[-1].split('.')[0]
    odbWorkPath = path.join(work_dir, odbName)
    fieldResultsPath = path.join(odbWorkPath, 'results')
    if path.isdir(fieldResultsPath):
        for root, dirs, files in walk(odbWorkPath):
            for f in files:
                unlink(path.join(root, f))
            for d in dirs:
                rmtree(path.join(root, d))
    makedirs(fieldResultsPath)
    p = path.abspath(fieldResultsPath)
    chdir(p)

    # Extract step keys
    steps = odb.steps
    stepKeys = steps.keys()
    nonArrayMembers = ['name', 'number', 'nlgeom', 'mass', 'acousticMass', 'massCenter', 'inertiaAboutCenter', 'inertiaAboutOrigin', 'acousticMassCenter', 'retainedEigenModes', 'description', 'procedure', 'domain', 'timePeriod', 'previousStepName', 'totalTime']
    arrayMembers = ['frames', 'historyRegions', 'loadCases', 'retainedNodalDofs', 'eliminatedNodalDofs']

    # loop through steps
    for k in stepKeys:
        # makedirs(i)
        # chdir(i)
        cstep = steps[k]
        makedirs(k)
        stepMembers = cstep.__members__
        nonArray = {}
        for i in stepMembers:
            if i in nonArrayMembers:
                if i == 'domain':
                    nonArray[i] = np.array([cstep.__getattribute__(i).name])
                else:
                    nonArray[i] = np.array([cstep.__getattribute__(i)])
            elif i in arrayMembers:
                if i == 'frames':
                    extractFrames(cstep.frames, k, instances, odb.rootAssembly)
                elif i == 'historyRegions':
                    extractHistoryRegions(cstep.historyRegions, k)
                elif i == 'loadCases':
                    extractLoadCases(cstep.loadCases, k)
                elif i == 'retainedNodalDofs':
                    extractRetainedNodalDofs(cstep.retainedNodalDofs, k)
                elif i == 'eliminatedNodalDofs':
                    extractEliminatedNodalDofs(cstep.eliminatedNodalDofs, k)
        sio.savemat(path.join(k,'steps.mat'), nonArray)


if __name__ == '__main__':
    odbName = argv[1]
    odbPath = argv[2]
    # odbName = 'FSAE_Simpleframe_Freq'
    # odbName = 'c-b-s'
    # odbName = 'baseRuns613'
    # odbName = 'Frame_Freq'
    # odbName = 'Frame_Static'
    # odbName = 'Frame_NoParts'
    # odbPath = '../odbs/frame'
    work_dir = path.abspath(argv[3])
    odbFile = path.join(odbPath, odbName + '.odb')
    itemsToExtract = ['parts', 'assembly', 'results']

    # open odb file
    odb = openOdb(odbFile, readOnly=TRUE)

    # Extract items
    for i in itemsToExtract:
        if i is 'parts':
            extractParts(odb, work_dir)
        elif i is 'assembly':
            # print "assembly"
            extractAssembly(odb, work_dir)
        elif i is 'results':
            extractResults(odb, work_dir)
    # close odb file
    odb.close()

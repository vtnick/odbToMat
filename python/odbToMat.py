from os import path, makedirs, walk
from shutil import rmtree
import numpy as np
import scipy.io as sio
from sys import argv
from odbAccess import *
from abaqusConstants import *
import timeit

def extractInstanceNodes(odb, allNodes, instanceName, i, instanceFolder):
    # Extract Nodes For Current Instance
    cnodes = allNodes.nodes[i]
    nodes = []
    nodeList = []
    # Loop Through Nodes To Get Label and Coordinates
    for node in cnodes:
        nodeList.append(node.label)
        label = np.array([node.label], dtype=float)
        coords = np.array(map(float, node.coordinates))
        nodes.append({'label': label, 'coords': coords})
    if instanceName == 'rootAssembly':
        odb.rootAssembly.NodeSetFromNodeLabels(name='ALL_NODES_CREATED', nodeLabels=(('', nodeList), ))
    # Save Nodes In Instance Folder
    f = path.join(instanceFolder, 'nodes')
    sio.savemat(f, {'nodes': nodes}, oned_as='row')

def extractInstanceElements(odb, allElements, instanceName, i, instanceFolder):
    # Same Procedure As Node Extract For Elements
    celems = allElements.elements[i]
    elements = []
    elementLabels = []
    for element in celems:
        elementLabels.append(element.label)
        label = np.array([element.label])
        connect = np.array([element.connectivity])
        eType = element.type
        x = list(element.instanceNames)
        instances = ['rootAssembly' if r == '' else r.replace('-', '_') for r in x]
        elements.append({'label': label, 'connect': connect, 'eType': eType, 'instances': np.array(instances, dtype=np.object)})
    if instanceName == 'rootAssembly':
        odb.rootAssembly.ElementSetFromElementLabels(name='ALL_ELEMENTS_CREATED', elementLabels=(('', elementLabels), ))
    f = path.join(instanceFolder, 'elements')
    sio.savemat(f, {'elements': elements}, oned_as='row')
def extractInstanceNodeSets(assembly, instanceName, i, instanceFolder):
    # Extract Node Sets -- 'if i' => checks if instance is root
    if instanceName != 'rootAssembly':
        c = 0
        cInstance = assembly.instances[instanceName]
        nodeSets = cInstance.nodeSets
        nodeSetKeys = nodeSets.keys()
        nodesDict = dict((el, []) for el in nodeSetKeys[c:])
        for nset in nodeSetKeys[c:]:
            nsetNodes = []
            for a in nodeSets[nset].nodes:
                nsetNodes.append(a.label)
            nodesDict[nset.replace('-', '_')] = np.array(nsetNodes, dtype=float)
    else:
        c = 1
        cInstance = assembly
        nodeSets = cInstance.nodeSets
        nodeSetKeys = nodeSets.keys()
        nodesDict = dict((el, []) for el in nodeSetKeys[c:])
        for nset in nodeSetKeys[c:]:
            nsetNodes = []
            for a in nodeSets[nset].nodes[0]:
                nsetNodes.append(a.label)
            nodesDict[nset.replace('-', '_')] = np.array(nsetNodes, dtype=float)
    f = path.join(instanceFolder, 'nodeSets')
    sio.savemat(f, {'nodeSets': nodesDict}, oned_as='row')

def extractInstanceElementSets(assembly, instanceName, i, instanceFolder):
    # Extract Node Sets -- 'if i' => checks if instance is root
    if instanceName != 'rootAssembly':
        c = 0
        cInstance = assembly.instances[instanceName]
        elementSets = cInstance.elementSets
        elementSetKeys = elementSets.keys()
        elementsDict = dict((el, []) for el in elementSetKeys[c:])
        for elset in elementSetKeys[c:]:
            elsetElements = []
            for a in elementSets[elset].elements:
                elsetElements.append(a.label)
            elementsDict[elset.replace('-', '_')] = np.array(elsetElements, dtype=float)
    else:
        c = 1
        cInstance = assembly
        elementSets = cInstance.elementSets
        elementSetKeys = elementSets.keys()
        elementsDict = dict((el, []) for el in elementSetKeys[c:])
        for elset in elementSetKeys[c:]:
            elsetElements = []
            for a in elementSets[elset].elements[0]:
                elsetElements.append(a.label)
            elementsDict[elset.replace('-', '_')] = np.array(elsetElements, dtype=float)
    f = path.join(instanceFolder, 'elementSets')
    sio.savemat(f, {'elementSets': elementsDict}, oned_as='row')

def extractGeometry(odb, folderPath):
    assembly = odb.rootAssembly
    y = assembly.nodeSets[' ALL NODES'].instanceNames
    instanceNames = [r.replace('-', '_') for r in y]
    allNodes = assembly.nodeSets[' ALL NODES']
    allElements = assembly.elementSets[' ALL ELEMENTS']
    for i, instance in enumerate(y):
        # MATLAB does not like field names with '-'
        instanceName = instance.replace('-', '_')
        instanceFolder = path.join(folderPath, 'geometry', instanceName)
        makedirs(instanceFolder)

        extractInstanceNodes(odb, allNodes, instanceName, i, instanceFolder)
        extractInstanceElements(odb, allElements, instanceName, i, instanceFolder)
        extractInstanceNodeSets(assembly, instance, i, instanceFolder)
        extractInstanceElementSets(assembly, instance, i, instanceFolder)

def extractFieldValues(values, location, componentLabels, description, fileName):

    allValues = []
    for cvalue in values:
        label = cvalue.__getattribute__(location)
        if cvalue.instance:
            instanceName = cvalue.instance.name
        else:
            instanceName = 'rootAssembly'
        data = cvalue.data
        allValues.append({'label': label, 'instanceName': instanceName, 'data': data, 'componentLabels': np.array([componentLabels], dtype=object), 'description': description})
    sio.savemat(fileName, {'allValues': allValues}, oned_as='row')

def extractFieldOutput(cFrame, frameCount, assembly, framePath):
    assemblySets = ['ALL_NODES_CREATED', 'ALL_ELEMENTS_CREATED']
    instances = assembly.instances
    instanceKeys = instances.keys()
    fieldOutputs = cFrame.fieldOutputs
    fields = fieldOutputs.keys()
    allValues = {}
    for field in fields:
        cfield = fieldOutputs[field]
        componentLabels = cfield.componentLabels
        description = cfield.description
        position = cfield.locations[0].position.name
        if position == 'NODAL':
            location = 'nodeLabel'
            n = True
        else:
            location = 'elementLabel'
            n = False
        for instance in instanceKeys:
            instanceValues = cfield.getSubset(region=instances[instance])
            instanceName = instance.replace('-', '_')
            f = path.join(framePath, 'frame_%d_%s_%s.mat' % (frameCount, instanceName, field.replace(' ', '_')))
            extractFieldValues(instanceValues.values, location, componentLabels, description, f)
        a = False
        if n:
            if assemblySets[0] in assembly.nodeSets.keys():
                a = True
                assemblyValues = cfield.getSubset(
                    region=assembly.nodeSets[assemblySets[0]])
        else:
            if assemblySets[0] in assembly.elementSets.keys():
                a = True
                assemblyValues = cfield.getSubset(
                    region=assembly.elementSets[assemblySets[1]])
        if a:
            f = path.join(framePath, 'frame_%d_%s_%s.mat' % (frameCount, 'Assembly', field.replace(' ', '_')))
            extractFieldValues(assemblyValues.values, location, componentLabels, description, f)

        f = path.join(framePath, 'frame_%d_%s_%s.mat' % (frameCount, 'all', field.replace(' ', '_')))
        extractFieldValues(cfield.values, location, componentLabels, description, f)

def extractFrames(frames, stepPath, assembly):
    nFrameMembers = ['cyclicModeNumber', 'description', 'incrementNumber', 'frameValue', 'domain', 'frequency', 'mode']
    aFrameMembers = ['fieldOutputs', 'loadCase', 'associatedFrame']
    for frameCount, cFrame in enumerate(frames):
        frameDir = path.join(stepPath, 'frame_%d' % (frameCount))
        makedirs(frameDir)
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
        g = path.join(frameDir, 'frame_%d_Dict.mat' % (frameCount))
        sio.savemat(g, cFrameDict, oned_as='row')
        extractFieldOutput(cFrame, frameCount, assembly, frameDir)

def extractResults(odb, folderPath):
    steps = odb.steps
    stepKeys = steps.keys()

    # Setup Members of steps into array and nonarrays
    nonArrayMembers = ['name', 'number', 'nlgeom', 'mass', 'acousticMass', 'massCenter', 'inertiaAboutCenter', 'inertiaAboutOrigin', 'acousticMassCenter', 'retainedEigenModes', 'description', 'procedure', 'domain', 'timePeriod', 'previousStepName', 'totalTime']
    arrayMembers = ['frames', 'historyRegions', 'loadCases', 'retainedNodalDofs', 'eliminatedNodalDofs']
    for ckey in stepKeys:
        # Get Current Step Information
        stepName = ckey.replace('-', '_')
        cstep = steps[ckey]
        frames = cstep.frames
        stepPath = path.join(folderPath, 'results', stepName)
        makedirs(stepPath)
        cstepDict = {}
        cstepMembers = cstep.__members__
        for cMember in cstepMembers:
            if cMember in nonArrayMembers:
                if cMember == 'domain':
                    cstepDict[cMember] = np.array([cstep.__getattribute__(cMember).name])
                else:
                    cstepDict[cMember] = np.array([cstep.__getattribute__(cMember)])
            else:
                if cMember == 'frames':
                    extractFrames(frames, stepPath, odb.rootAssembly)
        sio.savemat(path.join(stepPath, 'stepInfo'), cstepDict, oned_as='row')

if __name__ == '__main__':
    odbName = argv[1]
    odbPath = argv[2]
    work_dir = path.abspath(argv[3])
    # odbName = 'Frame_Static'
    # odbName = 'Frame_Freq'
    # odbPath = path.join('..', 'odbs', 'frame')
    # work_dir = path.abspath(path.join('..', 'work'))
    folderPath = path.abspath(path.join(work_dir, odbName))
    if path.isdir(folderPath):
        rmtree(folderPath)
    makedirs(folderPath)
    odbFile = path.join(odbPath, odbName + '.odb')

    # open odb file
    odb = openOdb(odbFile, readOnly=TRUE)
    extractGeometry(odb, folderPath)
    extractResults(odb, folderPath)
